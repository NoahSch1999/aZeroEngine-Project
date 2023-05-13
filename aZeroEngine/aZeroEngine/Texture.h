#pragma once
#include "ResourceTrashcan.h"
#include "DescriptorManager.h"
#include "VertexDefinitions.h"

/*
* TO DO
*	REMOVE UPLOAD AND READBACK RESOURCES ETC AND REPLACE IT WITH A LINEAR ALLOCATOR OR SOMETHING SIMILAR
*/

struct TextureUploadSettings
{
	bool m_discardUpload = true;
	void* m_initialData = nullptr;
};

struct TextureSettings
{
	DXGI_FORMAT m_srvFormat = DXGI_FORMAT_FORCE_UINT;
	DXGI_FORMAT m_rtvFormat = DXGI_FORMAT_FORCE_UINT;
	DXGI_FORMAT m_dsvFormat = DXGI_FORMAT_FORCE_UINT;
	UINT m_height = 0;
	UINT m_width = 0;
	UINT m_bytesPerTexel = 0;
	D3D12_RESOURCE_STATES m_initialState = D3D12_RESOURCE_STATE_COMMON;
	D3D12_RESOURCE_FLAGS m_flags = D3D12_RESOURCE_FLAG_NONE;
	D3D12_CLEAR_VALUE m_clearValue;
	TextureUploadSettings m_uploadSettings;
};

class Texture
{
	friend class FileTexture;
	friend class SwapChain;
	friend class GraphicsContextHandle;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_gpuOnlyResource = nullptr;
	D3D12_RESOURCE_STATES m_gpuOnlyResourceState = D3D12_RESOURCE_STATE_COMMON;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadResource = nullptr;
	D3D12_RESOURCE_STATES m_uploadResourceState = D3D12_RESOURCE_STATE_COMMON;
	void* m_mappedBuffer = nullptr;

	UINT m_rowPitch = 0;

	DescriptorHandle m_handleSRV;
	DescriptorHandle m_handleRTVDSV;
	DescriptorHandle m_handleUAV;

	TextureSettings m_settings;

	DescriptorManager* m_descriptorManager = nullptr;
	ResourceTrashcan* m_trashcan = nullptr;

public:
	ID3D12Resource* getGPUOnlyResource() { return m_gpuOnlyResource.Get(); }

	ID3D12Resource* getUploadResource() { return m_uploadResource.Get(); }

	DescriptorHandle& getSRVHandle() { return m_handleSRV; }
	DescriptorHandle& getRTVDSVHandle() { return m_handleRTVDSV; }
	DescriptorHandle& getUAVHandle() { return m_handleUAV; }
	DXGI_FORMAT getSRVFormat() const { return m_settings.m_srvFormat; }
	DXGI_FORMAT getRTVFormat() const { return m_settings.m_rtvFormat; }
	DXGI_FORMAT getDSVFormat() const { return m_settings.m_dsvFormat; }

	DXM::Vector2 getDimensions() const { return DXM::Vector2(m_settings.m_width, m_settings.m_height); }
	UINT getBytesPerTexel() const { return m_settings.m_bytesPerTexel; }
	UINT getPaddedRowPitch() const { return m_rowPitch; }

	void setResourceState(D3D12_RESOURCE_STATES newState) { m_gpuOnlyResourceState = newState; }

	void initUAV(ID3D12Device* device, DXGI_FORMAT format);

public:
	Texture() = default;
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	Texture(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const TextureSettings& settings,
		DescriptorManager& descriptorManager, ResourceTrashcan& trashcan);
		
	~Texture();

	Texture(Texture&& other) noexcept;

	Texture& operator=(Texture&& other) noexcept;

	void transition(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES newState);

};