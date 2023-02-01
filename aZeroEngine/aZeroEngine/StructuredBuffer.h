#pragma once
#include "BaseResource.h"
#include "ShaderDescriptorHeap.h"
#include "CommandList.h"
#include "HelperFunctions.h"

/*
I HAVE TO FIX SO THIS USES REAL TRIPPLE BUFFERING INSTEAD OF MEMCPY STUFF (WITH THIS I MEAN THE SAME WAY THAT THE CONSTANT BUFFER IS IMPLEMENTED).
*/

/** @brief Encapsulates a structured buffer resource.
*/
class StructuredBuffer : public BaseResource
{
private:
	int numElements;
	int sizeOfElement;
	int sizePerSubresource;
	bool isTrippleBuffered = false;

	int totalSize;

	ID3D12Resource* uploadBuffer;
	void* mappedBuffer;

public:
	StructuredBuffer()
		:BaseResource()
	{

	}

	~StructuredBuffer()
	{
		uploadBuffer->Release();
	}

	/**Initiates as a dynamic structured buffer.
	@param _device Device to use when creating the D3D12 resources.
	@param _cmdList CommandList to execute the resource initiation commands on.
	@param _numElements Num elements that the resource should contain.
	@param _sizeOfElements Size of each element within the resource.
	@param _trippleBuffering Whether or not the resource should be tripple buffered. Defaulted to single buffering.
	@return void
	*/
	void Init(ID3D12Device* _device, CommandList* _cmdList, int _numElements, int _sizeOfElement, bool _trippleBuffering = false)
	{
		numElements = _numElements;
		sizeOfElement = _sizeOfElement;
		sizePerSubresource = _numElements * _sizeOfElement;
		isTrippleBuffered = _trippleBuffering;
		totalSize = sizePerSubresource;

		if (isTrippleBuffered)
		{
			totalSize = sizePerSubresource * 3;
		}

		D3D12_RESOURCE_DESC rDesc;
		ZeroMemory(&rDesc, sizeof(D3D12_RESOURCE_DESC));
		rDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		rDesc.Width = totalSize;
		rDesc.Height = 1;
		rDesc.DepthOrArraySize = 1;
		rDesc.MipLevels = 1;
		rDesc.SampleDesc.Count = 1;
		rDesc.SampleDesc.Quality = 0;
		rDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		rDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		rDesc.Format = DXGI_FORMAT_UNKNOWN;

		Helper::CreateCommitedResourceDynamic(_device, resource, rDesc);
		gpuAddress = resource->GetGPUVirtualAddress();

		resource->Map(0, NULL, reinterpret_cast<void**>(&mappedBuffer));
	}

	/**Updates the specified element with the input data. Use this when the resource is SINGLE BUFFERED.
	@param _elementIndex Index of the element to update.
	@param _data Data to copy to the specified element.
	@param _size Size of the specified element.
	@return void
	*/
	void Update(int _elementIndex, void* _data, int _size)
	{
		if (_size > totalSize)
			throw;

		memcpy((char*)mappedBuffer + _elementIndex * sizeOfElement, _data, _size);
	}

	/**Updates the specified element with the input data. Use this when the resource is TRIPPLE BUFFERED.
	@param _frameIndex The current frame index.
	@param _elementIndex Index of the element to update.
	@param _data Data to copy to the specified element.
	@param _size Size of the specified element.
	@return void
	*/
	void Update(int _frameIndex, int _elementIndex, void* _data, int _size)
	{
		if (_size > totalSize)
			throw;

		memcpy((char*)mappedBuffer + (sizePerSubresource * _frameIndex) + (_elementIndex * sizeOfElement), _data, _size);
	}

	using BaseResource::GetGPUAddress;

	/**Returns the virtual GPU address for the specified frame index.
	@param _frameIndex Frame index to use.
	@return D3D12_GPU_VIRTUAL_ADDRESS
	*/
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(int _frameIndex);
};

