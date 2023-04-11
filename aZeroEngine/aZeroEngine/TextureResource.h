#pragma once
#include "DescriptorHandle.h"
#include "CommandList.h"
#include "HelperFunctions.h"

class TextureResource
{
protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> gpuOnlyResource = nullptr;
	D3D12_RESOURCE_STATES gpuOnlyResourceState = D3D12_RESOURCE_STATE_COMMON;

	Microsoft::WRL::ComPtr<ID3D12Resource> readbackResource = nullptr;
	D3D12_RESOURCE_STATES readbackResourceState = D3D12_RESOURCE_STATE_COMMON;
	void* readbackData = nullptr;

	DXGI_FORMAT format;
	int width = -1;
	int height = -1;
	int bytePerTexel = -1;
	int rowPitch = -1;
	DescriptorHandle handle;

	D3D12_RESOURCE_DESC desc = {};
public:

	void Init(ID3D12Device* _device, Microsoft::WRL::ComPtr<ID3D12Resource>& _uploadResource,
		CommandList& _transitionList, CommandList& _copyList, void* _data,
		int _width, int _height, int _bytePerTexel, DXGI_FORMAT _format, D3D12_RESOURCE_STATES _initState);

	void Init(ID3D12Device* _device, int _width, int _height, int _bytePerTexel, DXGI_FORMAT _format, D3D12_RESOURCE_STATES _initState,
		D3D12_CLEAR_VALUE _clearValue, D3D12_RESOURCE_FLAGS _flags = D3D12_RESOURCE_FLAG_NONE, bool _readback = false);
	

	Vector2 GetDimensions() const { return Vector2(width, height); }
	int GetTexelSize() const { return bytePerTexel; }

	Microsoft::WRL::ComPtr<ID3D12Resource>& GetGPUOnlyResource() { return gpuOnlyResource; }
	D3D12_RESOURCE_STATES GetGPUOnlyResourceState() const { return gpuOnlyResourceState; }
	void SetGPUOnlyResourceState(D3D12_RESOURCE_STATES _newState) { gpuOnlyResourceState = _newState; }

	Microsoft::WRL::ComPtr<ID3D12Resource>& GetReadbackResource() { return readbackResource; }
	D3D12_RESOURCE_STATES GetReadbackResourceState() const { return readbackResourceState; }
	void SetGetReadbackResourceState(D3D12_RESOURCE_STATES _newState) { readbackResourceState = _newState; }

	DescriptorHandle& GetHandle() { return handle; }
	DXGI_FORMAT GetFormat() { return format; }

	void Transition(CommandList& _cmdList, D3D12_RESOURCE_STATES _newState);

	void*& GetReadbackData() { return readbackData; }
	int GetReadbackSize() const { return rowPitch * height; }
	int GetRowPitch()const { return rowPitch; }

	D3D12_RESOURCE_DESC GetDesc() const { return desc; }
};