#pragma once
#include "Texture2DCache.h"
#include "ConstantBuffer.h"


template<typename T>
class Material
{
protected:
	T info;
	ConstantBuffer buffer;
public:
	Material() = default;
	virtual ~Material() = default;

	T* GetInfoPtr() { return &info; }
	ConstantBuffer* GetBufferPtr() { return &buffer; }
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() { return buffer.GetGPUAddress(); }
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(int _frameIndex) { return buffer.GetGPUAddress(_frameIndex); }

	DescriptorHandle GetHandle() { return buffer.handle; }
	void Update() { buffer.Update((void*)&info, sizeof(T)); }
	void Update(CommandList* _cmdList, int _frameIndex) { buffer.Update(_cmdList, (void*)&info, sizeof(T), _frameIndex); }
	
	int referenceCount = 0;
};