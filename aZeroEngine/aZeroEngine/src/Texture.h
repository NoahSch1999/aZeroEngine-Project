#pragma once
#include "ResourceRecycler.h"
#include "DescriptorManager.h"
#include "HelperFunctions.h"
#include <optional>
#include <map>

enum TEXTUREUSAGE
{
	NONE = 1 << 0,
	SRV = 1 << 1,
	UAV = 1 << 2,
	RTV = 1 << 3,
	DSV = 1 << 4,
	SIMACCESS = 1 << 5
};
DEFINE_ENUM_FLAG_OPERATORS(TEXTUREUSAGE)

class Texture
{
	friend class SwapChain; // TO-DO: Remove
public:
	struct Description
	{
		UINT m_bytesPerTexel = 4;
		
		UINT m_mipLevels = 1;
		UINT m_width = 1;
		UINT m_height = 1;

		DXGI_FORMAT m_mainFormat = DXGI_FORMAT::DXGI_FORMAT_FORCE_UINT;
		DXGI_FORMAT m_srvFormat = DXGI_FORMAT::DXGI_FORMAT_FORCE_UINT;
		DXGI_FORMAT m_uavFormat = DXGI_FORMAT::DXGI_FORMAT_FORCE_UINT;
		TEXTUREUSAGE m_usage = TEXTUREUSAGE::NONE;

		D3D12_CLEAR_VALUE m_clearValue;

		UINT m_sampleCount = 1;
		UINT m_sampleQuality = 0;

		D3D12_RESOURCE_STATES m_initialState = D3D12_RESOURCE_STATE_COMMON;
	};

private:

	Description m_description;

	DescriptorHandle m_srvHandle;
	DescriptorHandle m_uavHandle;

	// TO-DO: Make union
	DescriptorHandle m_dsvHandle;
	DescriptorHandle m_rtvHandle;

	DescriptorManager* m_descriptorManager = nullptr;
	ResourceRecycler* m_recycler = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_gpuResource = nullptr;
	D3D12_RESOURCE_STATES m_gpuResourceState = D3D12_RESOURCE_STATE_COMMON;

private:
	void init(ID3D12Device* device, const Description& description);
	void createDescriptors(ID3D12Device* device);
	void createResource(ID3D12Device* device, D3D12_CLEAR_VALUE* clearValue, D3D12_RESOURCE_FLAGS resourceFlags);

	bool checkBits(TEXTUREUSAGE bits, TEXTUREUSAGE targetBits);
	void moveData(Texture& target, Texture&& other);

public:

	void generateMipSRV(ID3D12Device* device, DescriptorHandle& handle, UINT mipLevelStart, UINT mipLevelEnd, DXGI_FORMAT format)
	{
		Helper::createSRVHandleMIP(device, m_gpuResource, handle.getCPUHandle(), format, mipLevelStart, mipLevelEnd, m_description.m_sampleCount > 1);
	}

	void generateMipRTV(ID3D12Device* device, DescriptorHandle& handle, DXGI_FORMAT format, bool multiSampled, UINT mipSlice)
	{
		Helper::CreateRTVHandle(device, m_gpuResource, handle.getCPUHandle(), format, multiSampled, mipSlice);
	}

	void generateMipDSV(ID3D12Device* device, DescriptorHandle& handle, DXGI_FORMAT format, bool multiSampled, UINT mipSlice)
	{
		Helper::CreateDSVHandle(device, m_gpuResource, handle.getCPUHandle(), format, multiSampled, mipSlice);
	}

	void generateMipUAV(ID3D12Device* device, DescriptorHandle& handle, DXGI_FORMAT format, bool multiSampled, UINT mipSlice)
	{
		Helper::createUAVHandle(device, m_gpuResource, handle.getCPUHandle(), format, multiSampled, mipSlice);
	}

	void upload(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const std::vector<D3D12_SUBRESOURCE_DATA>& subresourceData)
	{
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_gpuResource.Get(), 0,
			static_cast<UINT>(subresourceData.size()));

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);

		auto desc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

		ComPtr<ID3D12Resource> uploadRes;
		if (FAILED(device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploadRes.GetAddressOf()))))
			throw;

		this->transition(commandList, D3D12_RESOURCE_STATE_COPY_DEST);

		UpdateSubresources(commandList, m_gpuResource.Get(), uploadRes.Get(), 0, 0, 
			static_cast<UINT>(subresourceData.size()), subresourceData.data());

		this->transition(commandList, m_description.m_initialState);

		m_recycler->destroyResource(uploadRes);
	}

	Texture() = default;
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	Texture(ID3D12Device* device, DescriptorManager& descriptorManager,
		ResourceRecycler& recycler,
		const Description& description);

	~Texture();

	Texture(Texture&& other) noexcept;
	Texture& operator=(Texture&& other) noexcept;

	void transition(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES newState);

	void recreate(ID3D12Device* device, const Description& description);

	ID3D12Resource* const getGPUResource() const { return m_gpuResource.Get(); }
	UINT getWidth() const { return m_description.m_width; }
	UINT getHeight() const { return m_description.m_height; }
	UINT getBytesPerTexel() const { return m_description.m_bytesPerTexel; }
	DXGI_FORMAT getMainFormat() const { return m_description.m_mainFormat; }
	DXGI_FORMAT getUAVFormat() const { return m_description.m_uavFormat; }
	DXGI_FORMAT getSRVFormat() const { return m_description.m_srvFormat; }
	TEXTUREUSAGE getUsageFlags() const { return m_description.m_usage; }

	UINT getPaddedRowPitch() const { return ((m_description.m_width * m_description.m_bytesPerTexel + 128) / 256) * 256; }
	DescriptorHandle getSRVHandle() const { return m_srvHandle; }
	DescriptorHandle getUAVHandle() const { return m_uavHandle; }
	DescriptorHandle getRTVHandle() const { return m_rtvHandle; }
	DescriptorHandle getDSVHandle() const { return m_dsvHandle; }
	D3D12_CLEAR_VALUE getClearValue() const { return m_description.m_clearValue; }
	D3D12_RESOURCE_STATES getResourceState() const { return m_gpuResourceState; }
	Description getDescription() const { return m_description; }
	UINT getMipLevels() const { return m_gpuResource->GetDesc().MipLevels; }

	void forceResourceState(D3D12_RESOURCE_STATES newState) { m_gpuResourceState = newState; }
};