#pragma once
#include "DescriptorHandle.h"
#include <algorithm>

class HiddenDescriptorHeap
{
private:
	ID3D12DescriptorHeap* heap;
	DescriptorHandle handle;			// handle to start
	int descriptorSize;

	std::vector<int>freeSlots;			// free slots
	int slotSize;						// descriptor range size
	int numSlots;						// number of ranges of descriptors
	int totalDescriptors;				// total descriptors that the heap can hold
	D3D12_DESCRIPTOR_HEAP_TYPE type;

	int offsetInShaderHeap = 0;			// Offset within the shader heap
public:

	HiddenDescriptorHeap(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, int _numSlots, int _slotSize, const std::wstring& _name)
	{
		numSlots = _numSlots;
		slotSize = _slotSize;
		type = _type;

		totalDescriptors = numSlots * slotSize;

		freeSlots.resize(numSlots);
		for (int i = 0; i < numSlots; ++i)
		{
			freeSlots[i] = i;
		}

		D3D12_DESCRIPTOR_HEAP_DESC desc;
		desc.NumDescriptors = totalDescriptors;
		desc.Type = type;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 0; // Note - Remove?

		HRESULT hr = _device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap));
		if (FAILED(hr))
			throw;
		int x = 1;
		handle.SetHandle(heap->GetCPUDescriptorHandleForHeapStart());
		descriptorSize = _device->GetDescriptorHandleIncrementSize(_type);

		heap->SetName(_name.c_str());
	}

	~HiddenDescriptorHeap()
	{
		heap->Release();
	}

	DescriptorHandle GetNewSlot()
	{
		DescriptorHandle retHandle;
		if (freeSlots.size() == 0)
		{
			retHandle.SetHeapIndex(-1); // Invalidates the handle
			return retHandle;
		}

		int freeSlotStart = freeSlots[0];
		freeSlots[0] = freeSlots[freeSlots.size() - 1];
		freeSlots.resize(freeSlots.size() - 1);
		std::sort(freeSlots.begin(), freeSlots.end());

		D3D12_CPU_DESCRIPTOR_HANDLE handleCPUAddress = handle.GetCPUHandle();
		handleCPUAddress.ptr += freeSlotStart * (slotSize * descriptorSize);

		retHandle.SetHeapIndex(freeSlotStart * slotSize);
		retHandle.SetHandle(handleCPUAddress);
		return retHandle;
	}

	void RemoveSlot(const DescriptorHandle& _handle)
	{
		freeSlots.emplace_back(_handle.GetHeapIndex());
		std::sort(freeSlots.begin(), freeSlots.end());
	}

	ID3D12DescriptorHeap* GetHeap() { return heap; }
	D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() { return type; }
	DescriptorHandle& GetDescriptorHandle() { return handle; }
	int GetDescriptorSize() { return descriptorSize; }
	int GetSlotSize() { return slotSize; }
	int GetTotalDescriptors() { return totalDescriptors; }
	int GetOffSetInShaderHeap() { return offsetInShaderHeap; }

};