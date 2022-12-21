#pragma once
#include "BaseResource.h"
#include "ShaderDescriptorHeap.h"
#include "CommandList.h"
#include "HelperFunctions.h"

class Texture2D : public BaseResource
{
private:

	DXGI_FORMAT format;
	ID3D12Resource* uploadBuffer;
	void InitSRV(ID3D12Device* _device, D3D12_RESOURCE_DESC _desc);

public:
	Texture2D();

	~Texture2D();

	void Init(ID3D12Device* _device, CommandList* _cmdList, DescriptorHandle _handle, const std::string& _path,
		D3D12_RESOURCE_STATES _state, DXGI_FORMAT _format, const std::wstring& _resourceName = L"");

	void Init(ID3D12Device* _device, CommandList* _cmdList, const std::string& _path,
		D3D12_RESOURCE_STATES _state, DXGI_FORMAT _format, const std::wstring& _resourceName = L"");
};