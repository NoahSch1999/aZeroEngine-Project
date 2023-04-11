#include "Texture2D.h"
#include "stb_image.h"

Texture2D::Texture2D()
{
}

Texture2D::~Texture2D()
{
}

void Texture2D::Init(ID3D12Device* _device, CommandList& _transitionList, CommandList& _copyList, 
	Microsoft::WRL::ComPtr<ID3D12Resource>& _uploadResource, DescriptorHandle _srvHandle, void* _data, 
	UINT _width, UINT _height, UINT _channels, DXGI_FORMAT _format, const std::string& _name)
{
	SetName(_name);

	TextureResource::Init(_device, _uploadResource, _transitionList, _copyList, _data, _width, _height, _channels, _format, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	handle = _srvHandle;
	Helper::CreateSRVHandle(_device, gpuOnlyResource, handle.GetCPUHandle(), format);
}