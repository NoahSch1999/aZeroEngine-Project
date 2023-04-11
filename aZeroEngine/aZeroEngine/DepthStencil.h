#pragma once
#include "TextureResource.h"

/** @brief A class containing everything necessary for creating a depth stencil view.
* It can be created either as a pure depth stencil or as a depth stencil with a linked shader resource view.
*/
class DepthStencil : public TextureResource
{
private:
	DescriptorHandle srvHandle;
public:

	DepthStencil() = default;
	virtual ~DepthStencil() { }

	/**Returns a reference to the shader resource view descriptor handle.
	@return DescriptorHandle&
	*/
	DescriptorHandle& GetSrvHandle() { return srvHandle; }

	void Init(ID3D12Device* _device, DescriptorHandle _dsvHandle, DescriptorHandle _srvHandle, UINT _width, UINT _height);

	void Init(ID3D12Device* _device, DescriptorHandle _dsvHandle, UINT _width, UINT _height);

	void Clear(CommandList& _cmdList);
};

