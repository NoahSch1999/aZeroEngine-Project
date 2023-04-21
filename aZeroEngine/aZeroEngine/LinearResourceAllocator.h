#pragma once
#include "ResourceEngine.h"

template<typename T>
class Allocation
{
private:
	UINT current = 0;
	T* startPointer = nullptr;

public:
	Allocation(T* _startPointer)
		:startPointer(_startPointer) { }

	Allocation() = default;
	~Allocation() = default;

	void operator++() { current++; startPointer++; }
	T& operator*() { return *startPointer; }

	UINT CurrentOffset() const { return current; }
};

template<typename T>
class LinearResourceAllocator
{
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer = nullptr;
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
	void* mappedPointer = nullptr;
	UINT maxElements = 0;

	T* framePointer = nullptr;
	UINT nextElement = 0;

public:
	LinearResourceAllocator(ID3D12Device* _device, UINT _maxElements, UINT _numSubresources = 3)
		:maxElements(_maxElements)
	{
		int width = (maxElements * sizeof(T)) * _numSubresources;
		uploadBuffer = Helper::CreateBufferResource(_device, width, D3D12_HEAP_TYPE_UPLOAD);

		uploadBuffer->Map(0, NULL, reinterpret_cast<void**>(&mappedPointer));
		framePointer = (T*)mappedPointer;

		gpuAddress = uploadBuffer->GetGPUVirtualAddress();

#ifdef _DEBUG
		const std::string nameStr = "Linear Alloc: " + std::to_string(sizeof(T));
		const std::wstring nameWStr(nameStr.begin(), nameStr.end());
		uploadBuffer->SetName(nameWStr.c_str());
#endif // _DEBUG

	}

	~LinearResourceAllocator()
	{
		uploadBuffer->Unmap(0, nullptr);
	}

	void BeginFrame(UINT _frameIndex)
	{
		framePointer = (T*)mappedPointer + (_frameIndex * maxElements);
		nextElement = (_frameIndex * maxElements);
	}

	// Maybe make it return a pointer of input size instead
	UINT Allocate(const T& _data)
	{
		memcpy((void*)framePointer, (void*)&_data, sizeof(T));

		int currentElement = nextElement;

		framePointer++;
		nextElement++;

		return currentElement;
	}

	Allocation<T> Allocate(UINT _numElements)
	{
		T* currentOffset = framePointer;
		framePointer += _numElements;
		return Allocation<T>(currentOffset);
	}

	D3D12_GPU_VIRTUAL_ADDRESS GetVirtualAddress() const { return gpuAddress; }

};