#pragma once
#include "DescriptorHandle.h"
#include "CommandList.h"
#include "HelperFunctions.h"

/** @brief Base class for all texture resources within aZero Engine.
*/
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

	/* TO BE EDITED
	@param
	@return
	*/
	void Init(ID3D12Device* _device, Microsoft::WRL::ComPtr<ID3D12Resource>& _uploadResource,
		CommandList& _transitionList, CommandList& _copyList, void* _data,
		int _width, int _height, int _bytePerTexel, DXGI_FORMAT _format, D3D12_RESOURCE_STATES _initState);

	/* TO BE EDITED
	@param
	@return
	*/
	void Init(ID3D12Device* _device, int _width, int _height, int _bytePerTexel, DXGI_FORMAT _format, D3D12_RESOURCE_STATES _initState,
		D3D12_CLEAR_VALUE _clearValue, D3D12_RESOURCE_FLAGS _flags = D3D12_RESOURCE_FLAG_NONE, bool _readback = false);
	
	/* TO BE EDITED
	@param
	@return
	*/
	Vector2 GetDimensions() const { return Vector2(width, height); }

	/* TO BE EDITED
	@param
	@return
	*/
	int GetTexelSize() const { return bytePerTexel; }

	/* TO BE EDITED
	@param
	@return
	*/
	Microsoft::WRL::ComPtr<ID3D12Resource>& GetGPUOnlyResource() { return gpuOnlyResource; }

	/* TO BE EDITED
	@param
	@return
	*/
	D3D12_RESOURCE_STATES GetGPUOnlyResourceState() const { return gpuOnlyResourceState; }

	/* TO BE EDITED
	@param
	@return
	*/
	void SetGPUOnlyResourceState(D3D12_RESOURCE_STATES _newState) { gpuOnlyResourceState = _newState; }

	/* TO BE EDITED
	@param
	@return
	*/
	Microsoft::WRL::ComPtr<ID3D12Resource>& GetReadbackResource() { return readbackResource; }

	/* TO BE EDITED
	@param
	@return
	*/
	D3D12_RESOURCE_STATES GetReadbackResourceState() const { return readbackResourceState; }

	/* TO BE EDITED
	@param
	@return
	*/
	void SetGetReadbackResourceState(D3D12_RESOURCE_STATES _newState) { readbackResourceState = _newState; }

	/* TO BE EDITED
	@param
	@return
	*/
	DescriptorHandle& GetHandle() { return handle; }

	/* TO BE EDITED
	@param
	@return
	*/
	DXGI_FORMAT GetFormat() const { return format; }

	/* TO BE EDITED
	@param
	@return
	*/
	void Transition(CommandList& _cmdList, D3D12_RESOURCE_STATES _newState);

	/* TO BE EDITED
	@param
	@return
	*/
	void*& GetReadbackData() { return readbackData; }

	/* TO BE EDITED
	@param
	@return
	*/
	int GetReadbackSize() const { return rowPitch * height; }

	/* TO BE EDITED
	@param
	@return
	*/
	int GetRowPitch() const { return rowPitch; }

	/* TO BE EDITED
	@param
	@return
	*/
	D3D12_RESOURCE_DESC GetDesc() const { return desc; }
};