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

class VertexBuffer : public BaseResource
{
private:
	
	D3D12_VERTEX_BUFFER_VIEW view;
	ID3D12Resource* uploadBuffer;
	IndexBuffer indexBuffer;
	int numVertices;
public:
	VertexBuffer();
	VertexBuffer(ID3D12Device* _device, CommandList* _cmdList, void* _data, int _size, int _stride);

	void InitStatic(ID3D12Device* _device, CommandList* _cmdList, void* _data, int _size, int _stride, const std::wstring& _name = L"");

	~VertexBuffer();

	D3D12_VERTEX_BUFFER_VIEW& GetView() { return view; }
	IndexBuffer* GetIndexBuffer() { return &indexBuffer; }
	int GetNumVertices() { return numVertices; }
	void SetNumVertices(int _numVertices) { numVertices = _numVertices; }
	int referenceCount = 0;
};

