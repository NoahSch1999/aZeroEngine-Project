#include "Mesh.h"
#include "HelperFunctions.h"

Mesh::Mesh()
	:numVertices(0)
{
}

Mesh::Mesh(ID3D12Device* _device, CommandList* _cmdList, const std::string& _path)
{
	LoadBufferFromFile(_device, _cmdList, _path);
}

Mesh::~Mesh()
{
	// cleanup of vertex buffer handled via vertexBufferCache
	delete buffer;
}

void Mesh::LoadBufferFromFile(ID3D12Device* _device, CommandList* _cmdList, const std::string& _path)
{
	Helper::BasicVertexListInfo vertexInfo;
	Helper::LoadVertexListFromFile(&vertexInfo, _path);
	buffer = new VertexBuffer(_device, _cmdList, vertexInfo.verticeData.data(), (int)vertexInfo.verticeData.size(), sizeof(BasicVertex));
	numVertices = (int)vertexInfo.verticeData.size();
}

void Mesh::SetBuffer(VertexBuffer* _buffer)
{
	buffer = _buffer;
}
