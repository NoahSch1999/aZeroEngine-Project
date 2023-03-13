#pragma once
#include "CommandList.h"
#include "FreeList.h"
#include "HiddenDescriptorHeap.h"

/** @brief Manages D3D12 descriptorsd within the engine.
Contains all descriptor heaps and necessary methods to utilize them.
*/
class DescriptorManager
{
private:
	FreeList texture2DFreeList;
	FreeList passFreeList;

	int totalResourceDescriptors = -1;
	ShaderDescriptorHeap resourceHeap;

	FreeList samplerFreeList;
	ShaderDescriptorHeap samplerHeap;

	FreeList dsvFreeList;
	HiddenDescriptorHeap dsvHeap;

	FreeList rtvFreeList;
	HiddenDescriptorHeap rtvHeap;
public:
	DescriptorManager() = default;

	~DescriptorManager()
	{
	}

	/**Returns a pointer to the internal ID3D12DescriptorHeap that contains CBV, UAV, and SRV resources.
	@return ID3D12DescriptorHeap*
	*/
	ID3D12DescriptorHeap* GetResourceHeap()
	{
		return resourceHeap.GetHeap();
	}

	/**Returns a pointer to the internal ID3D12DescriptorHeap that contains Samplers.
	@return ID3D12DescriptorHeap*
	*/
	ID3D12DescriptorHeap* GetSamplerHeap()
	{
		return samplerHeap.GetHeap();
	}

	/**Returns the GPU descriptor handle to the start of the Texture2D section of the resource heap.
	* This can be used for bindless rendering of the 2D textures.
	@return D3D12_GPU_DESCRIPTOR_HANDLE
	*/
	D3D12_GPU_DESCRIPTOR_HANDLE GetTexture2DStartAddress()
	{
		return resourceHeap.GetDescriptorHandle().GetGPUHandle();
	}

	/**Initializes the object.
	@param _device Device to use when creating the necessary D3D12 resources.
	@param _numTexture2DDescriptors Max number of 2D textures supported.
	@param _numPassDescriptors Max number of other resources supported.
	@return void
	*/
	void Init(ID3D12Device* _device, int _numTexture2DDescriptors, int _numPassDescriptors)
	{
		texture2DFreeList.Init(0, _numTexture2DDescriptors);
		passFreeList.Init(_numTexture2DDescriptors, _numTexture2DDescriptors + _numPassDescriptors);

		totalResourceDescriptors = _numTexture2DDescriptors + _numPassDescriptors;

		resourceHeap.Init(_device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, totalResourceDescriptors, L"Shader Visible Resource Heap");

		samplerFreeList.Init(0, 10);
		samplerHeap.Init(_device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 10, L"Sampler Heap");

		dsvFreeList.Init(0, 40);
		dsvHeap.Init(_device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 40, 1, L"DSV Heap");

		rtvFreeList.Init(0, 40);
		rtvHeap.Init(_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 40, 1, L"RTV Heap");
	}

	/**Returns a copy of a new unused descriptor for a Texture2D resource.
	@return DescriptorHandle
	*/
	DescriptorHandle GetTexture2DDescriptor()
	{
		return resourceHeap.GetNewDescriptorHandle(texture2DFreeList.GetValue());
	}
	
	/**Clear the Texture2D descriptor at the specified index so that it can be reused. Always call this before getting rid of a Texture2D which has gotten a DescriptorHandle throught the ResourceManager::GetTexture2DDescriptor() method.
	@param _index Index of the descriptor to be cleared.
	@return void
	*/
	void FreeTexture2DDescriptor(int _index)
	{
		texture2DFreeList.FreeValue(_index);
	}

	/**Returns a copy of a new unused descriptor for a non-Texture2D resource.
	@return DescriptorHandle
	*/
	DescriptorHandle GetPassDescriptor()
	{
		return resourceHeap.GetNewDescriptorHandle(passFreeList.GetValue());
	}

	/**Clear the resource descriptor at the specified index so that it can be reused. Always call this before getting rid of a resource which has gotten a DescriptorHandle throught the ResourceManager::GetPassDescriptor() method.
	@param _index Index of the descriptor to be cleared.
	@return void
	*/
	void FreePassDescriptor(int _index)
	{
		passFreeList.FreeValue(_index);
	}

	/**Returns a copy of a new unused descriptor for a sampler resource.
	@return DescriptorHandle
	*/
	DescriptorHandle GetSamplerDescriptor()
	{
		return samplerHeap.GetNewDescriptorHandle(samplerFreeList.GetValue());
	}

	/**Clear the sampler descriptor at the specified index so that it can be reused. Always call this before getting rid of a sampler which has gotten a DescriptorHandle throught the ResourceManager::GetSamplerDescriptor() method.
	@param _index Index of the descriptor to be cleared.
	@return void
	*/
	void FreeSamplerDescriptor(int _index)
	{
		samplerFreeList.FreeValue(_index);
	}

	DescriptorHandle GetRTVDescriptor()
	{
		return rtvHeap.GetNewSlot();
	}

	void FreeRTVDescriptor(const DescriptorHandle& _handle)
	{
		rtvHeap.RemoveSlot(_handle);
	}

	DescriptorHandle GetDSVDescriptor()
	{
		return dsvHeap.GetNewSlot();
	}

	void FreeDSVDescriptor(const DescriptorHandle& _handle)
	{
		dsvHeap.RemoveSlot(_handle);
	}

};