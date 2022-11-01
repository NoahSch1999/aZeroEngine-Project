#pragma once
#include "BaseResource.h"
#include "ShaderDescriptorHeap.h"
#include "HiddenDescHeap.h"
#include "CommandList.h"

class ConstantBuffer : public BaseResource
{
public:
	ConstantBuffer();

	void InitStatic(ID3D12Device* _device, CommandList* _cmdList, void* _data, int _size, const std::wstring& _name = L"");
	void InitAsDynamic(ID3D12Device* _device, CommandList* _cmdList, void* _data, int _size, const std::wstring& _name = L"");

	// Requires the handle to be initialized
	void InitAsCBV(ID3D12Device* _device);

	void Update(const void* data, int size);
	~ConstantBuffer();

	ID3D12Resource* uploadBuffer;
	void* mappedBuffer;
	int size;
	bool isStatic;
};

