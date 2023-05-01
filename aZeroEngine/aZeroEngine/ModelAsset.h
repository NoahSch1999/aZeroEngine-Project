#pragma once
#include "UploadBuffer.h"
#include "VertexDefinitions.h"

struct GeometryData
{
	UINT m_numVertices = 0;
	UINT m_numIndices = 0;
	std::string m_meshName = "";
};

class ModelAsset
{
private:
	GeometryData m_geometryData;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	UploadBuffer<BasicVertex> m_vertexBuffer;

	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
	UploadBuffer<UINT> m_indexBuffer;

public:
	UINT GetNumVertices() const { return m_geometryData.m_numVertices; }
	UINT GetNumIndices() const { return m_geometryData.m_numIndices; }
	std::string GetMeshName() const { return m_geometryData.m_meshName; }
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const { return m_vertexBufferView; }
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const { return m_indexBufferView; }

public:
	ModelAsset() = default;
	ModelAsset(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, UINT frameIndex, 
		ResourceTrashcan& trashcan, const GeometryData& geometryData, const Helper::ModelFileData& loadedModelFileData);

};