#pragma once
#include "TextureResource.h"

/** @brief Encapsulates a render target.
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

	/* TO BE EDITED
	@param
	@return
	*/
	void Init(ID3D12Device* _device, DescriptorHandle _dsvHandle, DescriptorHandle _srvHandle, UINT _width, UINT _height);

	/* TO BE EDITED
	@param
	@return
	*/
	void Init(ID3D12Device* _device, DescriptorHandle _dsvHandle, UINT _width, UINT _height);

	/* Records a clear of the DepthStencil.
	@param _cmdList CommandList to record the clear command on.
	@return void
	*/
	void Clear(CommandList& _cmdList);
};

