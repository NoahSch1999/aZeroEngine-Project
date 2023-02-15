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
	int sizeOfElement = -1;
	int numElements = -1;

	// Inherited via BaseResource
	// Disabled
	virtual void InitStatic(ID3D12Device* _device, CommandList* _cmdList, void* _initData, int _numBytes, const std::wstring& _mainResourceName) override;
	virtual void InitDynamic(ID3D12Device* _device, CommandList* _cmdList, void* _initData, int _numBytes, bool _trippleBuffered, const std::wstring& _mainResourceName) override;
public:
	StructuredBuffer()
		:BaseResource()
	{

	}

	virtual ~StructuredBuffer()
	{
		//uploadBuffer->Release();
	}

	using BaseResource::GetGPUAddress;

	/**Updates the specified element with the input data. Use this when the resource is SINGLE BUFFERED.
	@param _elementIndex Index of the element to update.
	@param _data Data to copy to the specified element.
	@param _size Size of the specified element.
	@return void
	*/
	void Update(int _elementIndex, void* _data, int _size);

	/**Returns the virtual GPU address for the specified frame index.
	@param _frameIndex Frame index to use.
	@return D3D12_GPU_VIRTUAL_ADDRESS
	*/
	D3D12_GPU_VIRTUAL_ADDRESS const GetGPUAddress(int _frameIndex);

	// Redesign for "new" tripple buffer version
	void InitDynamic(ID3D12Device* _device, CommandList* _cmdList, void* _initData, int _numBytes, int _numElements, bool _isTrippleBuffered, const std::wstring& _mainResourceName);
};

