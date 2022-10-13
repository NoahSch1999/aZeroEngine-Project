#pragma once
#include "BaseResource.h"
#include "ShaderDescriptorHeap.h"
#include "HiddenDescriptorHeap.h"
#include "CommandList.h"

class ConstantBuffer : public BaseResource
{
public:
	// Try avoid duplicate of constructors...
	ConstantBuffer(ID3D12Device* _device, ShaderDescriptorHeap* _heap, CommandList* _cmdList, void* _data, int _size, bool _static = false, const std::wstring& _name = L"");
	
	// Add functionality for this...
	ConstantBuffer(ID3D12Device* _device, HiddenDescriptorHeap* _heap, CommandList* _cmdList, void* _data, int _size, bool _static = false, const std::wstring& _name = L"");
	
	void Update(const void* data, int size);
	~ConstantBuffer();

	ID3D12Resource* uploadBuffer;
	void* mappedBuffer;
	int dataSize;
	bool isStatic;
};

