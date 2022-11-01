#include "HiddenDescriptorHeap.h"
//
//HiddenDescriptorHeap::HiddenDescriptorHeap(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, int _maxDescriptors, const std::wstring& _name)
//	:DescriptorHeap(_device, _type, _maxDescriptors, false, _name)
//{
//}
//
//HiddenDescriptorHeap::~HiddenDescriptorHeap()
//{
//}
//
//DescriptorHandle HiddenDescriptorHeap::GetNewDescriptorHandle(int _numHandles)
//{
//	int newHandleIndex = 0;
//	int handleBlockEnd = currentIndex + _numHandles;
//	if (handleBlockEnd <= maxDescriptors)
//	{
//		newHandleIndex = currentIndex;
//		currentIndex = handleBlockEnd;
//		numDescriptors = currentIndex;
//	}
//	else
//	{
//		throw;
//	}
//
//	D3D12_CPU_DESCRIPTOR_HANDLE newCPUHandle = cpuAddress;
//	newCPUHandle.ptr += newHandleIndex * descriptorSize;
//
//	DescriptorHandle handle(newCPUHandle, newHandleIndex);
//	return handle;
//}
