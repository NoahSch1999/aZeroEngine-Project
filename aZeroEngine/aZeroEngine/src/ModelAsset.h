#pragma once
#include "UploadBuffer.h"
#include "VertexDefinitions.h"

struct GeometryData
{
	UINT m_numVertices = 0;
	UINT m_numIndices = 0;
	float m_boundingRadius = 0;
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
	UINT getNumVertices() const { return m_geometryData.m_numVertices; }
	UINT getNumIndices() const { return m_geometryData.m_numIndices; }
	float getBoundingRadius() const { return m_geometryData.m_boundingRadius; }
	std::string getMeshName() const { return m_geometryData.m_meshName; }
	D3D12_VERTEX_BUFFER_VIEW getVertexBufferView() const { return m_vertexBufferView; }
	D3D12_INDEX_BUFFER_VIEW getIndexBufferView() const { return m_indexBufferView; }

public:
	ModelAsset() = default;
	ModelAsset(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, UINT frameIndex, 
		ResourceRecycler& trashcan, const GeometryData& geometryData, const Helper::ModelFileData& loadedModelFileData);

	ModelAsset(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, UINT frameIndex,
		ResourceRecycler& trashcan, const GeometryData& geometryData, const void* const vertexPtr, const void* const indexPtr);
};