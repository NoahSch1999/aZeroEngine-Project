#pragma once
#include "BufferResource.h"

/** @brief Encapsulates a structured buffer resource.
*/
class StructuredBuffer : public BufferResource
{
private:
	int sizeOfElement = -1;
	int numElements = -1;

public:
	StructuredBuffer() { }

	virtual ~StructuredBuffer() { }

	/** Initiates the ConstantBuffer with the input data.
	@param _device The ID3D12Device to use when creating the neccessary resources.
	@param _copyList The copy list to record the initiation copy for.
	@param _data The initial data for the ConstantBuffer.
	@param _numBytes Number of bytes to allocate for the StructuredBuffer. Should be something like sizeof(Element) * NumElements.
	@param _numElements Number of elements within the StructuredBuffer.
	@param _dynamic Whether or not the StructuredBuffer is mutable.
	@param _tripple Whether or not the StructuredBuffer should be tripple buffered.
	@return void
	*/
	void Init(ID3D12Device* _device, CommandList& _copyList, void* _data, int _numBytes, int _numElements, bool _dynamic = true, bool _trippleBuffered = true);
};