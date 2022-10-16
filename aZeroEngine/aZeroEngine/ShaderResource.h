#pragma once
#include "BaseResource.h"
#include "ShaderDescriptorHeap.h"
#include "HiddenDescriptorHeap.h"
#include "CommandList.h"

class ShaderResource : public BaseResource
{
public:
	ShaderResource();
	~ShaderResource();

	// Remove duplicate (maybe take in base-class and then cast idk...)
	void InitAsTextureImplicit(ID3D12Device* _device, ShaderDescriptorHeap* _heap, CommandList* _cmdList, const std::string& _path, 
		D3D12_RESOURCE_STATES _state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, const std::wstring& _resourceName = L"");

	// not up to date
	void InitAsTextureImplicit(ID3D12Device* _device, HiddenDescriptorHeap* _heap, CommandList* _cmdList, const std::string& _path,
		D3D12_RESOURCE_STATES _state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, const std::wstring& _resourceName = L"");

	// Add functions for initiating as other type of resource...

	// Tiled...

	// Non-texture...

	//

	ID3D12Resource* uploadBuffer;
};

