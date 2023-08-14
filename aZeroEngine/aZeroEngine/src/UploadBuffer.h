#pragma once
#undef GetObject
#include "ResourceRecycler.h"
#include "HelperFunctions.h"

struct UploadBufferInitSettings
{
	bool m_discardUpload = false;
	void* m_initialData = nullptr;
};

struct UploadBufferSettings
{
	UINT m_numElements = 1;
	UINT m_numSubresources = 1;
};

template <typename T>
class UploadBuffer
{
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_gpuOnlyResource = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadResource = nullptr;
	void* m_mappedBuffer = nullptr;

	D3D12_GPU_VIRTUAL_ADDRESS m_virtualAddress = (UINT64)0;

	ResourceRecycler* m_trashcan = nullptr;

	UploadBufferSettings m_settings;

private:
	void InitResource(ID3D12Device* device)
	{
		UINT sizePerSubresource = (sizeof(T) * m_settings.m_numElements);
		m_gpuOnlyResource = Helper::CreateBufferResource(device, sizePerSubresource, D3D12_HEAP_TYPE_DEFAULT);

		UINT uploadWidth = sizePerSubresource * m_settings.m_numSubresources;
		m_uploadResource = Helper::CreateBufferResource(device, uploadWidth, D3D12_HEAP_TYPE_UPLOAD);
		m_uploadResource->Map(0, NULL, reinterpret_cast<void**>(&m_mappedBuffer));

		m_virtualAddress = m_gpuOnlyResource->GetGPUVirtualAddress();

#ifdef _DEBUG
		m_gpuOnlyResource->SetName(L"UploadBufferGPU");
		m_uploadResource->SetName(L"UploadBufferUPLOAD");
#endif // _DEBUG
	}

public:
	UploadBuffer() = default;
	UploadBuffer(const UploadBuffer&) = delete;
	UploadBuffer& operator=(const UploadBuffer&) = delete;

	UploadBuffer(ID3D12Device* device, const UploadBufferInitSettings& initSettings, const UploadBufferSettings& settings, ResourceRecycler& trashcan)
		:m_settings(settings), m_trashcan(&trashcan)
	{
		this->InitResource(device);

		if (initSettings.m_discardUpload)
		{
			trashcan.destroyResource(m_uploadResource);
			m_uploadResource->Unmap(0, nullptr);
			m_uploadResource = nullptr;
		}
	}

	UploadBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, UINT frameIndex,
		const UploadBufferInitSettings& initSettings, const UploadBufferSettings& settings, ResourceRecycler& trashcan)
		:m_settings(settings), m_trashcan(&trashcan)
	{
		this->InitResource(device);

		if (m_settings.m_numSubresources == 1)
		{
			this->update(commandList, initSettings.m_initialData);
		}
		else
		{
			this->update(commandList, frameIndex, initSettings.m_initialData);
		}

		if (initSettings.m_discardUpload)
		{
			trashcan.destroyResource(m_uploadResource);
			m_uploadResource->Unmap(0, nullptr);
			m_uploadResource = nullptr;
		}
	}

	~UploadBuffer()
	{
		if (m_gpuOnlyResource)
		{
			m_trashcan->destroyResource(m_gpuOnlyResource);
		}

		if (m_uploadResource)
		{
			m_uploadResource->Unmap(0, NULL);
			m_trashcan->destroyResource(m_uploadResource);
		}
	}

	UploadBuffer(UploadBuffer&& other) noexcept
	{
		m_gpuOnlyResource = other.m_gpuOnlyResource;
		m_uploadResource = other.m_uploadResource;
		m_mappedBuffer = other.m_mappedBuffer;
		m_virtualAddress = other.m_virtualAddress;
		m_trashcan = other.m_trashcan;
		m_settings = other.m_settings;

		other.m_gpuOnlyResource = nullptr;
		other.m_uploadResource = nullptr;
		other.m_mappedBuffer = nullptr;
	}

	UploadBuffer& operator=(UploadBuffer&& other) noexcept
	{
		if (this != &other)
		{
			if (m_gpuOnlyResource)
			{
				m_trashcan->destroyResource(m_gpuOnlyResource);
			}

			if (m_uploadResource)
			{
				m_uploadResource->Unmap(0, NULL);
				m_trashcan->destroyResource(m_uploadResource);
			}

			m_gpuOnlyResource = other.m_gpuOnlyResource;
			m_uploadResource = other.m_uploadResource;
			m_mappedBuffer = other.m_mappedBuffer;
			m_virtualAddress = other.m_virtualAddress;
			m_trashcan = other.m_trashcan;
			m_settings = other.m_settings;

			other.m_gpuOnlyResource = nullptr;
			other.m_uploadResource = nullptr;
			other.m_mappedBuffer = nullptr;
		}

		return *this;
	}

	// For singular subresource buffer
	void update(ID3D12GraphicsCommandList* commandList, T& data, UINT elementIndex)
	{
		UINT sizePerSubresource = (sizeof(T) * m_settings.m_numElements);
		UINT64 offset = (sizeof(T) * elementIndex);
		memcpy((char*)m_mappedBuffer + offset, (char*)&data, sizePerSubresource);
		commandList->CopyBufferRegion(m_gpuOnlyResource.Get(), 0, m_uploadResource.Get(), offset, sizeof(T));
	}

	// For singular subresource buffer
	void update(ID3D12GraphicsCommandList* commandList, void* data)
	{
		UINT sizePerSubresource = (sizeof(T) * m_settings.m_numElements);
		memcpy(reinterpret_cast<char*>(m_mappedBuffer), data, sizePerSubresource);
		commandList->CopyBufferRegion(m_gpuOnlyResource.Get(), 0, m_uploadResource.Get(), 0, sizePerSubresource);
	}

	// For buffer with multiple subresources based on frames in flight
	void update(ID3D12GraphicsCommandList* commandList, UINT frameIndex, T& data, UINT elementIndex)
	{
		UINT sizePerSubresource = (sizeof(T) * m_settings.m_numElements);
		UINT64 offset = (sizePerSubresource * frameIndex) + (sizeof(T) * elementIndex);
		memcpy((char*)m_mappedBuffer + offset, (char*)&data, sizePerSubresource);
		commandList->CopyBufferRegion(m_gpuOnlyResource.Get(), 0, m_uploadResource.Get(), offset, sizeof(T));
	}

	// TO-FIX sizePerSubresource might exceed the data that _data is pointer to which might cause UB
	// For buffer with multiple subresources based on frames in flight
	void update(ID3D12GraphicsCommandList* commandList, UINT frameIndex, void* data)
	{
		UINT sizePerSubresource = (sizeof(T) * m_settings.m_numElements);
		UINT64 offset = sizePerSubresource * frameIndex;
		memcpy(reinterpret_cast<char*>(m_mappedBuffer) + offset, data, sizePerSubresource);
		commandList->CopyBufferRegion(m_gpuOnlyResource.Get(), 0, m_uploadResource.Get(), offset, sizePerSubresource);
	}

	D3D12_GPU_VIRTUAL_ADDRESS getVirtualAddress() const { return m_virtualAddress; }
};