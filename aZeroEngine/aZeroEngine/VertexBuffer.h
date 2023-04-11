#pragma once
#include "BufferResource.h"

/** @brief Encapsulates a vertex buffer.
*/
class VertexBuffer : public BufferResource
{
private:
	D3D12_VERTEX_BUFFER_VIEW view;
	int numVertices = -1;
	std::string name = "";

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
	
	/** Set the number of vertices within the VertexBuffer.
	@param _numVertices Number of vertices.
	@return void
	*/
	void SetNumVertices(int _numVertices) { numVertices = _numVertices; }

	/** Returns a std::string with the name of the VertexBuffer.
	@return std::string
	*/
	std::string GetName()const { return name; }

	/** Sets the internal name of the VertexBuffer.
	@param _name The new name of the VertexBuffer.
	@return void
	*/
	void SetName(const std::string& _name) { name = _name; }

	/** Initiates the VertexBuffer with the input data.
	@param _device The ID3D12Device to use when creating the neccessary resources.
	@param _copyList The copy list to record the initiation copy for.
	@param _data The initial data for the VertexBuffer.
	@param _numBytes Number of bytes to allocate for the VertexBuffer. Should be something like sizeof(Vertex) * NumVertices.
	@param _numElements Number of vertices within the VertexBuffer
	@param _name Name of the VertexBuffer.
	@return void
	*/
	void Init(ID3D12Device* _device, CommandList& _copyList, void* _data, int _numBytes, int _numElements, const std::string& _name);
};

