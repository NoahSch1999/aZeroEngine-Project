#pragma once
#include "TextureResource.h"

/** @brief Encapsulates a render target.
*/
class RenderTarget : public TextureResource
{
private:
	DescriptorHandle srvHandle;
	D3D12_CLEAR_VALUE clearValue;

public:
	RenderTarget() = default;

	virtual ~RenderTarget() {};

	/**Returns a reference to the shader resource view descriptor handle.
	@return DescriptorHandle&
	*/
	constexpr DescriptorHandle& GetSrvHandle() { return srvHandle; }

	/* TO BE EDITED
	@param
	@return
	*/
	void Init(ID3D12Device* _device, DescriptorHandle _rtvHandle, DescriptorHandle _srvHandle, UINT _width, UINT _height, UINT _channels, 
		DXGI_FORMAT _format, const Vector4& _clearColor, bool _readback = false);

	/* TO BE EDITED
	@param
	@return
	*/
	void Init(ID3D12Device* _device, DescriptorHandle _rtvHandle, UINT _width, UINT _height, UINT _channels, DXGI_FORMAT _format, bool _readback = false);

	/* Records a clear of the RenderTarget.
	@param _cmdList CommandList to record the clear command on.
	@return void
	*/
	void Clear(CommandList& _cmdList);
};

