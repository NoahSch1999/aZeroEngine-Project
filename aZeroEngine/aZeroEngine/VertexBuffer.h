#pragma once
#include "BaseResource.h"

class BaseResource;

/** @brief Encapsulates a vertex buffer.
*/
class VertexBuffer : public BaseResource
{
private:
	D3D12_VERTEX_BUFFER_VIEW view;
	int numVertices;
	std::string fileName;

public:
	VertexBuffer();

	~VertexBuffer(){}

	/** Returns a reference to the vertex buffer view.
	@return D3D12_VERTEX_BUFFER_VIEW&
	*/
	D3D12_VERTEX_BUFFER_VIEW& GetView() { return view; }

	/** Get the number of vertices within the buffer.
	@return int
	*/
	int GetNumVertices() { return numVertices; }
	
	/** Set the number of vertices within the buffer.
	@param _numVertices Number of vertices.
	@return void
	*/
	void SetNumVertices(int _numVertices) { numVertices = _numVertices; }

	/**Returns a constant pointer to the name of the mesh data file.
	@return const char*
	*/
	const std::string GetFileName()const { return fileName; }

	/**Sets the internal name of the mesh data file.
	@return void
	*/
	void SetFileName(const std::string& _fileName) { fileName = _fileName; }

	void InitBase(ID3D12Device* _device, CommandList& _copyList, void* _initData, int _numBytes, int _numElements, const std::string& _name)
	{
		numVertices = _numElements;
		int stride = _numBytes / _numElements;

		sizePerSubresource = _numBytes;

		D3D12_RESOURCE_DESC rDesc;
		ZeroMemory(&rDesc, sizeof(rDesc));
		rDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		rDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		rDesc.Width = sizePerSubresource;
		rDesc.Height = 1;
		rDesc.DepthOrArraySize = 1;
		rDesc.MipLevels = 1;
		rDesc.Format = DXGI_FORMAT_UNKNOWN;
		rDesc.SampleDesc.Count = 1;
		rDesc.SampleDesc.Quality = 0;
		rDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		rDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

		HRESULT hr = _device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE, &rDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&mainResource));
		if (FAILED(hr))
			throw;

		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		hr = _device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE, &rDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&intermediateResource));
		if (FAILED(hr))
			throw;

		D3D12_SUBRESOURCE_DATA sData = {};
		sData.pData = _initData;
		sData.RowPitch = sizePerSubresource;
		sData.SlicePitch = sizePerSubresource;

		UpdateSubresources(_copyList.GetGraphicList(), mainResource, intermediateResource, 0, 0, 1, &sData);

		view.BufferLocation = mainResource->GetGPUVirtualAddress();
		view.SizeInBytes = sizePerSubresource;
		view.StrideInBytes = stride;

		fileName = _name;
	}
};

