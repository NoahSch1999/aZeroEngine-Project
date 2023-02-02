#pragma once
#include "BaseResource.h"
#include "CommandList.h"

class IndexBuffer : public BaseResource
{
private:
	D3D12_INDEX_BUFFER_VIEW view;
	ID3D12Resource* uploadBuffer;
public:
	IndexBuffer()
		:BaseResource()
	{

	}

	~IndexBuffer()
	{
		uploadBuffer->Release();
	}

	void InitStatic(ID3D12Device* _device, CommandList* _cmdList, void* _data, int _numIndices, const std::wstring& _name = L"");

	D3D12_INDEX_BUFFER_VIEW& GetView() { return view; }
	int numIndices = 0;
};

/** @brief Encapsulates a vertex buffer.
*/
class VertexBuffer : public BaseResource
{
private:
	D3D12_VERTEX_BUFFER_VIEW view;
	int numVertices;
	std::string fileName;
public:
	ID3D12Resource* uploadBuffer;
	VertexBuffer();

	/**Initiates as a static vertex buffer.
	@param _device Device to use when creating the D3D12 resources.
	@param _cmdList CommandList to execute the resource initiation commands on.
	@param _data Data to initiate the buffer with.
	@param _size Size of the data to initiate with.
	@param _stride Size of each vertex within the resource.
	*/
	VertexBuffer(ID3D12Device* _device, CommandList* _cmdList, void* _data, int _size, int _stride);

	/**Initiates as a static vertex buffer.
	@param _device Device to use when creating the D3D12 resources.
	@param _cmdList CommandList to execute the resource initiation commands on.
	@param _data Data to initiate the buffer with.
	@param _size Size of the data to initiate with.
	@param _stride Size of each vertex within the resource.
	@param _name Optional name of the internal ID3D12Resource* object.
	@return void
	*/
	void InitStatic(ID3D12Device* _device, CommandList* _cmdList, void* _data, int _size, int _stride, const std::wstring& _name = L"");

	~VertexBuffer();

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
	int referenceCount = 0;

	/**Returns a constant pointer to the name of the mesh data file.
	@return const char*
	*/
	const std::string GetFileName()const { return fileName; }

	/**Sets the internal name of the mesh data file.
	@return void
	*/
	void SetFileName(const std::string& _fileName) { fileName = _fileName; }
};

