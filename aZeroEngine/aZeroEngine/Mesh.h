#pragma once
#include "VertexBuffer.h"

class Mesh
{
public:
	Mesh();
	Mesh(ID3D12Device* _device, CommandList* _cmdList, const std::string& _path);
	~Mesh();

	// Change so it loads into vertex cache or sets from it if already existing...
	void LoadBufferFromFile(ID3D12Device* _device, CommandList* _cmdList, const std::string& _path);
	
	void SetBuffer(VertexBuffer* _buffer);

	VertexBuffer* buffer;
	unsigned int numVertices;
};

