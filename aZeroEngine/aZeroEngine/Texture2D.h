#pragma once
#include "BaseResource.h"
#include "ShaderDescriptorHeap.h"

class BaseResource;
class ShaderDescriptorHeap;

/** @brief Encapsulates a 2D texture resource.
*/
class Texture2D
{
private:
	std::string fileName;
	DXGI_FORMAT format;
	void InitSRV(ID3D12Device* _device, D3D12_RESOURCE_DESC _desc);
	ID3D12Resource* mainResource = nullptr;
	D3D12_RESOURCE_STATES mainResourceState = D3D12_RESOURCE_STATE_COMMON;
	DescriptorHandle handle;
	int width = 0;
	int height = 0;
	ID3D12Resource* intermResource = nullptr;
public:
	Texture2D();

	~Texture2D();

	/**Loads and initiates the texture WITH a descriptor handle.
	@param _device Device to use when creating the D3D12 resources.
	@param _cmdList CommandList to execute the resource initiation commands on.
	@param _handle Handle to copy to the internal descriptor handle variable.
	@param _path Directory to the texture to load.
	@param _name File name of the texture to load.
	@param _state State of the texture once the resource is initialized.
	@param _format Format of the texture to load.
	@return void
	*/
	void Init(ID3D12Device* _device, CommandList* _cmdList, ID3D12Resource*& _intermediateResource, DescriptorHandle _handle, const std::string& _path, const std::string& _name,
		D3D12_RESOURCE_STATES _state, DXGI_FORMAT _format);

	// Have to set name
	void InitBase(ID3D12Device* _device, CommandList& _endPassList, CommandList& _copyList, DescriptorHandle _srvHandle, void* _data, UINT _width, UINT _height, UINT _channels, DXGI_FORMAT _format, const std::string& _name)
	{
		format = _format;

		D3D12_RESOURCE_DESC rDesc;
		ZeroMemory(&rDesc, sizeof(D3D12_RESOURCE_DESC));
		rDesc.MipLevels = 1;
		rDesc.Format = format;
		rDesc.Width = _width;
		rDesc.Height = _height;
		rDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		rDesc.DepthOrArraySize = 1;
		rDesc.SampleDesc.Count = 1;				
		rDesc.SampleDesc.Quality = 0;
		rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;	
		rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;	
		rDesc.Alignment = 0;

		D3D12_RESOURCE_ALLOCATION_INFO allocInfo = _device->GetResourceAllocationInfo(0, 1, &rDesc);

		CD3DX12_RESOURCE_DESC uDesc = CD3DX12_RESOURCE_DESC::Buffer(allocInfo.SizeInBytes);

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

		HRESULT hr = _device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE, &rDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&mainResource));
		if (FAILED(hr))
			throw;

		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		hr = _device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE, &uDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&intermResource));
		if (FAILED(hr))
			throw;

		D3D12_SUBRESOURCE_DATA sData = {};
		sData.pData = _data;
		sData.RowPitch = _width * _channels;
		sData.SlicePitch = _width * _channels * _height;

		UpdateSubresources(_copyList.GetGraphicList(), mainResource, intermResource, 0, 0, 1, &sData);

		D3D12_RESOURCE_BARRIER barrier(CD3DX12_RESOURCE_BARRIER::Transition(mainResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		_endPassList.GetGraphicList()->ResourceBarrier(1, &barrier);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = rDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = rDesc.MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.f;

		handle = _srvHandle;

		_device->CreateShaderResourceView(mainResource, &srvDesc, handle.GetCPUHandle());

		fileName = _name;
		width = _width;
		height = _height;
	}

	/**Returns a constant pointer to the name of the texture file.
	@return const char*
	*/
	const char* GetFileName() const { return fileName.c_str(); }

	/**Sets the internal name of the texture file.
	@return void
	*/
	void SetFileName(const char* _fileName) { fileName.assign(_fileName); }

	DescriptorHandle GetHandle() const { return handle; }

	/**Returns a pointer reference to the internal main ID3D12Resource object.
	@return ID3D12Resource*&
	*/
	ID3D12Resource*& GetMainResource() { return mainResource; }

	Vector2 GetDimensions()const { return Vector2(width, height); }
};