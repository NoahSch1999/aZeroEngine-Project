#pragma once
#include "D3D12Include.h"

template<typename T>
class Allocation
{
private:
	UINT m_current = 0;
	T* m_startPointer = nullptr;

public:
	Allocation(T* startPointer)
		:m_startPointer(startPointer) { }

	Allocation() = default;
	~Allocation() = default;

	void operator++() { m_current++; m_startPointer++; }
	T& operator*() { return *m_startPointer; }

	UINT CurrentOffset() const { return m_current; }
};

template<typename T>
class LinearResourceAllocator
{
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadBuffer = nullptr;
	D3D12_GPU_VIRTUAL_ADDRESS m_gpuAddress;
	void* m_mappedPointer = nullptr;
	UINT m_maxElements = 0;

	T* m_framePointer = nullptr;
	UINT m_nextElement = 0;

public:
	LinearResourceAllocator(ID3D12Device* device, UINT maxElements, UINT numSubresources)
		:m_maxElements(maxElements)
	{
		int width = (maxElements * sizeof(T)) * numSubresources;
		m_uploadBuffer = Helper::CreateBufferResource(device, width, D3D12_HEAP_TYPE_UPLOAD);

		m_uploadBuffer->Map(0, NULL, reinterpret_cast<void**>(&m_mappedPointer));
		m_framePointer = (T*)m_mappedPointer;

		m_gpuAddress = m_uploadBuffer->GetGPUVirtualAddress();

#ifdef _DEBUG
		const std::string nameStr = "Linear Alloc: " + std::to_string(sizeof(T));
		const std::wstring nameWStr(nameStr.begin(), nameStr.end());
		m_uploadBuffer->SetName(nameWStr.c_str());
#endif // _DEBUG

	}

	~LinearResourceAllocator()
	{
		m_uploadBuffer->Unmap(0, nullptr);
	}

	void BeginFrame(UINT frameIndex)
	{
		m_framePointer = (T*)m_mappedPointer + (frameIndex * m_maxElements);
		m_nextElement = (frameIndex * m_maxElements);
	}

	// Maybe make it return a pointer of input size instead
	UINT Allocate(const T& data)
	{
		memcpy((void*)m_framePointer, (void*)&data, sizeof(T));

		UINT currentElement = m_nextElement;

		m_framePointer++;
		m_nextElement++;

		return currentElement;
	}

	Allocation<T> Allocate(UINT numElements)
	{
		T* currentOffset = m_framePointer;
		m_framePointer += numElements;
		return Allocation<T>(currentOffset);
	}

	D3D12_GPU_VIRTUAL_ADDRESS GetVirtualAddress() const { return m_gpuAddress; }

};