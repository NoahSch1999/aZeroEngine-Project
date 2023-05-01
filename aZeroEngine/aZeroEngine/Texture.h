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
	bool discardUpload = true;
	void* initialData = nullptr;
};

struct TextureSettings
{
	DXGI_FORMAT srvFormat = DXGI_FORMAT_FORCE_UINT;
	DXGI_FORMAT rtvFormat = DXGI_FORMAT_FORCE_UINT;
	DXGI_FORMAT dsvFormat = DXGI_FORMAT_FORCE_UINT;
	UINT height = 0;
	UINT width = 0;
	UINT bytesPerTexel = 0;
	bool createReadback = false;
	D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;
	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
	D3D12_CLEAR_VALUE clearValue;
	TextureUploadSettings uploadSettings;
};

class Texture
{
	friend class FileTexture;
	friend class SwapChain;
	friend class GraphicsContextHandle;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> gpuOnlyResource = nullptr;
	D3D12_RESOURCE_STATES gpuOnlyResourceState = D3D12_RESOURCE_STATE_COMMON;

	Microsoft::WRL::ComPtr<ID3D12Resource> uploadResource = nullptr;
	D3D12_RESOURCE_STATES uploadResourceState = D3D12_RESOURCE_STATE_COMMON;
	void* mappedBuffer = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> readbackResource = nullptr;
	void* readbackMappedPtr = nullptr;
	UINT rowPitch = 0;

	DescriptorHandle handleSRV;
	DescriptorHandle handleRTVDSV;

	TextureSettings settings;

	DescriptorManager* descriptorManager = nullptr;
	ResourceTrashcan* trashcan = nullptr;

public:
	ID3D12Resource* GetGPUOnlyResource() { return gpuOnlyResource.Get(); }

	ID3D12Resource* GetUploadResource() { return uploadResource.Get(); }

	ID3D12Resource* GetReadbackResource() { return readbackResource.Get(); }
	void* GetReadbackPtr() const { return readbackMappedPtr; }

	DescriptorHandle& GetSRVHandle() { return handleSRV; }
	DescriptorHandle& GetRTVDSVHandle() { return handleRTVDSV; }
	DXGI_FORMAT GetSRVFormat() const { return settings.srvFormat; }
	DXGI_FORMAT GetRTVFormat() const { return settings.rtvFormat; }
	DXGI_FORMAT GetDSVFormat() const { return settings.dsvFormat; }

	Vector2 GetDimensions() const { return Vector2(settings.width, settings.height); }
	UINT GetBytesPerTexel() const { return settings.bytesPerTexel; }
	UINT GetRowPitch() const { return rowPitch; }

	void SetResourceState(D3D12_RESOURCE_STATES _newState) { gpuOnlyResourceState = _newState; }

public:
	Texture() = default;
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	Texture(ID3D12Device* _device, ID3D12GraphicsCommandList* _commandList, const TextureSettings& _settings,
		DescriptorManager& _descriptorManager, ResourceTrashcan& _trashcan);
		
	~Texture();

	Texture(Texture&& _other) noexcept;

	Texture& operator=(Texture&& _other) noexcept;

	void Transition(ID3D12GraphicsCommandList* _commandList, D3D12_RESOURCE_STATES _newState);

};