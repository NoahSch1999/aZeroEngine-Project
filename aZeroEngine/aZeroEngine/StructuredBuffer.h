#pragma once
#include "BaseResource.h"
#include "ShaderDescriptorHeap.h"

class BaseResource;
class ShaderDescriptorHeap;

/*
I HAVE TO FIX SO THIS USES REAL TRIPPLE BUFFERING INSTEAD OF MEMCPY STUFF (WITH THIS I MEAN THE SAME WAY THAT THE CONSTANT BUFFER IS IMPLEMENTED).
*/

/** @brief Encapsulates a structured buffer resource.
*/
class StructuredBuffer : public BaseResource
{
private:
	int sizeOfElement;

public:
	StructuredBuffer()
		:BaseResource()
	{

	}

	virtual ~StructuredBuffer()
	{
		//uploadBuffer->Release();
	}

	/**Updates the specified element with the input data. Use this when the resource is SINGLE BUFFERED.
	@param _elementIndex Index of the element to update.
	@param _data Data to copy to the specified element.
	@param _size Size of the specified element.
	@return void
	*/
	void Update(int _elementIndex, void* _data, int _size);

	using BaseResource::GetGPUAddress;

	/**Returns the virtual GPU address for the specified frame index.
	@param _frameIndex Frame index to use.
	@return D3D12_GPU_VIRTUAL_ADDRESS
	*/
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(int _frameIndex);

	// Inherited via BaseResource
	// Static has to be defined
	virtual void InitStatic(ID3D12Device* _device, CommandList* _cmdList, void* _initData, int _numBytes, int _numElements, const std::wstring& _mainResourceName) override;
	// Has to be redesigned for "new" tripple buffering and tested
	virtual void InitDynamic(ID3D12Device* _device, CommandList* _cmdList, void* _initData, int _numBytes, int _numElements, bool _trippleBuffered, const std::wstring& _mainResourceName) override;
};

