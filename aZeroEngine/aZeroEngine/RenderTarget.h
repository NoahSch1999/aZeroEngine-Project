#pragma once
#include "TextureResource.h"

class RenderTarget : public TextureResource
{
private:
	DescriptorHandle srvHandle;
	//DXGI_FORMAT format;
	D3D12_CLEAR_VALUE clearValue;
public:
	RenderTarget() = default;

	virtual ~RenderTarget() {};

	/**Returns a reference to the shader resource view descriptor handle.
	@return DescriptorHandle&
	*/
	constexpr DescriptorHandle& GetSrvHandle() { return srvHandle; }

	void Init(ID3D12Device* _device, DescriptorHandle _rtvHandle, DescriptorHandle _srvHandle, UINT _width, UINT _height, UINT _channels, 
		DXGI_FORMAT _format, const Vector4& _clearColor, bool _readback = false);

	void Init(ID3D12Device* _device, DescriptorHandle _rtvHandle, UINT _width, UINT _height, UINT _channels, DXGI_FORMAT _format, bool _readback = false);

	void Clear(CommandList& _cmdList);
};

