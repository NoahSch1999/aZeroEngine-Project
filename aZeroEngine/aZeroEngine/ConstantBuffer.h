#pragma once
#include "BufferResource.h"

/** @brief Encapsulates a constant buffer.
Can be either static or dynamic (single or tripple-buffered).
*/
class ConstantBuffer : public BufferResource
{
public:

	ConstantBuffer() = default;
	virtual ~ConstantBuffer() {};

	/** Initiates the ConstantBuffer with the input data.
	@param _device The ID3D12Device to use when creating the neccessary resources.
	@param _copyList The copy list to record the initiation copy for.
	@param _data The initial data for the ConstantBuffer.
	@param _numBytes Number of bytes to allocate for the ConstantBuffer.
	@param _dynamic Whether or not the ConstantBuffer is mutable.
	@param _tripple Whether or not the ConstantBuffer should be tripple buffered.
	@return void
	*/
	void Init(ID3D12Device* _device, CommandList& _copyList, void* _data, int _numBytes, bool _dynamic = false, int _trippleBuffered = false);
};

