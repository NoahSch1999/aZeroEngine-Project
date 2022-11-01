#pragma once
#include "BaseResource.h"
#include "ShaderDescriptorHeap.h"
#include "HiddenDescHeap.h"
#include "CommandList.h"

class ShaderResource : public BaseResource
{
public:
	ShaderResource();
	~ShaderResource();

	// Remove duplicate (maybe take in base-class and then cast idk...)
	void InitAsTextureAndDescriptor(ID3D12Device* _device, ShaderDescriptorHeap* _heap, CommandList* _cmdList, const std::string& _path, 
		D3D12_RESOURCE_STATES _state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, const std::wstring& _resourceName = L"");

	void InitAsTexture(ID3D12Device* _device, CommandList* _cmdList, const std::string& _path,
		D3D12_RESOURCE_STATES _state, const std::wstring& _resourceName);

	// Requires the handle to be initialized
	void InitAsSRV(ID3D12Device* _device);

	ID3D12Resource* uploadBuffer;
	D3D12_RESOURCE_DESC rDesc = {};
};

