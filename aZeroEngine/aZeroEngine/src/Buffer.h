#pragma once
#include "HelperFunctions.h"
#include "ResourceRecycler.h"

class Buffer
{
public:
	struct Description
	{
		D3D12_HEAP_TYPE m_heapType;
		UINT m_sizeBytesPerElement = 0;
		UINT m_numElements = 1;
		D3D12_RESOURCE_FLAGS m_flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;
	};

private:
	Description m_description;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_resource = nullptr;
	void* m_mappedPtr = nullptr;
	D3D12_GPU_VIRTUAL_ADDRESS m_virtualAddress = (UINT64)0;
	ResourceRecycler* m_resourceRecycler = nullptr;

public:
	ID3D12Resource* const getResource() const { return m_resource.Get(); }
	D3D12_HEAP_TYPE getType() const { return m_description.m_heapType; }
	D3D12_GPU_VIRTUAL_ADDRESS getVirtualAddress() const { return m_virtualAddress; }
	UINT getTotalSizeBytes() const { return m_description.m_numElements * m_description.m_sizeBytesPerElement; }
	UINT getElementSizeBytes() const { return m_description.m_sizeBytesPerElement; }
	UINT getNumElements() const { return m_description.m_numElements; }
	Description getDescription() const { return m_description; }

	Buffer() = default;
	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;

	Buffer(ID3D12Device* device, const Description& description, ResourceRecycler& resourceRecycler)
		:m_resourceRecycler(&resourceRecycler)
	{
		this->create(device, description);
	}

	~Buffer()
	{
		if (m_resource)
		{
			m_resourceRecycler->destroyResource(m_resource);
			m_resource = nullptr;
		}
	}

	Buffer(Buffer&& other) noexcept
	{
		m_resource = other.m_resource;
		m_description = other.m_description;
		m_mappedPtr = other.m_mappedPtr;
		m_resourceRecycler = other.m_resourceRecycler;
		m_virtualAddress = other.m_virtualAddress;

		other.m_resource = nullptr;
	}

	Buffer& operator=(Buffer&& other) noexcept
	{
		if (this != &other)
		{
			m_resource = other.m_resource;
			m_description = other.m_description;
			m_mappedPtr = other.m_mappedPtr;
			m_resourceRecycler = other.m_resourceRecycler;
			m_virtualAddress = other.m_virtualAddress;

			other.m_resource = nullptr;
		}

		return *this;
	}

	void create(ID3D12Device* device, const Description& description)
	{
		if (m_resource)
		{
			m_resourceRecycler->destroyResource(m_resource);
			m_resource = nullptr;
		}

		m_description = description;
		m_resource = Helper::CreateBufferResource(device,
			m_description.m_sizeBytesPerElement * m_description.m_numElements,
			m_description.m_heapType, m_description.m_flags);

		m_virtualAddress = m_resource->GetGPUVirtualAddress();

		if (m_description.m_heapType == D3D12_HEAP_TYPE_UPLOAD || m_description.m_heapType == D3D12_HEAP_TYPE_READBACK)
		{
			m_resource->Map(0, NULL, reinterpret_cast<void**>(&m_mappedPtr));
		}
	}

	DescriptorHandle generateUAV(ID3D12Device* device, DescriptorManager& descriptorManager, DXGI_FORMAT format)
	{
		DescriptorHandle handle = descriptorManager.getResourceDescriptor();
		Helper::createUAVHandleBuffer(device, m_resource.Get(), handle.getCPUHandle(), format);
		return handle;
	}

	DescriptorHandle generateSRV(ID3D12Device* device, DescriptorManager& descriptorManager, DXGI_FORMAT format)
	{
		DescriptorHandle handle = descriptorManager.getResourceDescriptor();
		Helper::createSRVHandleBuffer(device, m_resource.Get(), handle.getCPUHandle(), format);
		return handle;
	}

	DescriptorHandle generateCBV(ID3D12Device* device, DescriptorManager& descriptorManager, UINT offset, UINT sizeInBytes)
	{
		DescriptorHandle handle = descriptorManager.getResourceDescriptor();
		Helper::createCBVHandleBuffer(device, handle.getCPUHandle(), m_virtualAddress + offset, sizeInBytes);
		return handle;
	}

	// Only works if srcBuffer is D3D12_HEAP_TYPE_UPLOAD and "this" is D3D12_HEAP_TYPE_UPLOAD or D3D12_HEAP_TYPE_DEFAULT
	void uploadFromSource(ID3D12GraphicsCommandList* commandList, Buffer& srcBuffer, UINT srcOffset, void* data, UINT dataSize, UINT dstOffset = 0)
	{
#ifdef _DEBUG
		if (srcBuffer.getType() != D3D12_HEAP_TYPE_UPLOAD)
			throw;

		if (m_description.m_heapType == D3D12_HEAP_TYPE_READBACK)
			throw;
#endif // _DEBUG

		memcpy((char*)srcBuffer.m_mappedPtr + srcOffset, data, dataSize);
		commandList->CopyBufferRegion(m_resource.Get(), dstOffset, srcBuffer.m_resource.Get(), srcOffset, dataSize);
	}

	// Only works if "this" is D3D12_HEAP_TYPE_UPLOAD
	void uploadFromRaw(void* data, UINT dataSize, UINT dstOffset = 0)
	{
#ifdef _DEBUG
		if (m_description.m_heapType != D3D12_HEAP_TYPE_UPLOAD)
			throw;
#endif // _DEBUG

		memcpy((char*)m_mappedPtr + dstOffset, data, dataSize);
	}

	// Only works if "this" is D3D12_HEAP_TYPE_READBACK
	template<typename T>
	void readbackData(T& dst, UINT srcOffset)
	{
#ifdef _DEBUG
		if (m_description.m_heapType != D3D12_HEAP_TYPE_READBACK)
			throw;
#endif // _DEBUG

		memcpy((void*)&dst, (char*)m_mappedPtr + srcOffset, sizeof(T));
	}
};