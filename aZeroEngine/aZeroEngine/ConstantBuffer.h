#pragma once
#include "BaseResource.h"
#include "ShaderDescriptorHeap.h"
#include "HiddenDescriptorHeap.h"
#include "CommandList.h"

class ConstantBuffer : public BaseResource
{
private:
	ID3D12Resource* uploadBuffer;
	D3D12_RESOURCE_STATES uploadState = D3D12_RESOURCE_STATE_COMMON;
	int totalSize;
	bool isStatic;
	bool isTrippleBuffered;
public:
	int sizePerSubresource;
	void* mappedBuffer;
	ConstantBuffer();
	~ConstantBuffer();

	void InitStatic(ID3D12Device* _device, CommandList* _cmdList, void* _data, int _size, const std::wstring& _name = L"");
	void InitAsDynamic(ID3D12Device* _device, CommandList* _cmdList, void* _data, int _size, bool _trippleBuffering = false, const std::wstring& _name = L"");

	// Requires the handle to be initialized before being called
	void InitAsCBV(ID3D12Device* _device);

	void Update(const void* _data, int _size);

	void Update(CommandList* _cmdList, const void* _data, int _size, int _frameIndex);

	bool IsStatic() { return isStatic; }
	bool IsTrippleBuffered() { return isTrippleBuffered; }

	using BaseResource::GetGPUAddress;
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(int _frameIndex);
};

