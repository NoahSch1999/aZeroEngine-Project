#pragma once
#include "CommandList.h"
#include "FreeList.h"

class ResourceManager
{
private:
	FreeList texture2DFreeList;
	FreeList passFreeList;

	int totalResourceDescriptors = -1;
	ShaderDescriptorHeap resourceHeap;

	FreeList samplerFreeList;
	ShaderDescriptorHeap samplerHeap;
public:
	ResourceManager() = default;

	~ResourceManager()
	{
		resourceHeap.heap->Release();
		samplerHeap.heap->Release();
	}

	ID3D12DescriptorHeap* GetResourceHeap()
	{
		return resourceHeap.heap;
	}

	ID3D12DescriptorHeap* GetSamplerHeap()
	{
		return samplerHeap.heap;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE GetTexture2DStartAddress()
	{
		return resourceHeap.handle.gpuHandle;
	}

	void Init(ID3D12Device* _device, int _numTexture2DDescriptors, int _numPassDescriptors)
	{
		texture2DFreeList.Init(0, _numTexture2DDescriptors);
		passFreeList.Init(_numTexture2DDescriptors, _numTexture2DDescriptors + _numPassDescriptors);

		totalResourceDescriptors = _numTexture2DDescriptors + _numPassDescriptors;

		resourceHeap.Init(_device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, totalResourceDescriptors, L"Shader Visible Resource Heap");

		samplerFreeList.Init(0, 10);
		samplerHeap.Init(_device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 10, L"Sampler Heap");
	}

	DescriptorHandle GetTexture2DDescriptor()
	{
		return resourceHeap.GetNewDescriptorHandle(texture2DFreeList.GetValue());
	}

	void FreeTexture2DDescriptor(int _index)
	{
		texture2DFreeList.FreeValue(_index);
	}

	DescriptorHandle GetPassDescriptor()
	{
		return resourceHeap.GetNewDescriptorHandle(passFreeList.GetValue());
	}

	void FreePassDescriptor(int _index)
	{
		passFreeList.FreeValue(_index);
	}

	DescriptorHandle GetSamplerDescriptor()
	{
		return samplerHeap.GetNewDescriptorHandle(samplerFreeList.GetValue());
	}

	void FreeSamplerDescriptor(int _index)
	{
		samplerFreeList.FreeValue(_index);
	}
};