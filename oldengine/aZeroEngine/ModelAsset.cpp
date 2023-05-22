#include "ModelAsset.h"

ModelAsset::ModelAsset(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, 
	UINT frameIndex, ResourceTrashcan& trashcan, const GeometryData& geometryData, const Helper::ModelFileData& loadedModelFileData)
	:m_geometryData(geometryData)
{
	UploadBufferInitSettings vbInitSettings;
	vbInitSettings.m_discardUpload = true;
	vbInitSettings.m_initialData = (void*)loadedModelFileData.verticeData.data();

	UploadBufferSettings vbSettings;
	vbSettings.m_numSubresources = 1;
	vbSettings.m_numElements = loadedModelFileData.verticeData.size();

	m_vertexBuffer = std::move(UploadBuffer<BasicVertex>(device, commandList, frameIndex, vbInitSettings, vbSettings, trashcan));

	int stride = sizeof(BasicVertex);
	m_vertexBufferView.BufferLocation = m_vertexBuffer.getVirtualAddress();
	m_vertexBufferView.SizeInBytes = stride * loadedModelFileData.verticeData.size();
	m_vertexBufferView.StrideInBytes = stride;

	UploadBufferInitSettings ibInitSettings;
	ibInitSettings.m_discardUpload = true;
	ibInitSettings.m_initialData = (void*)loadedModelFileData.indexData.data();
	UploadBufferSettings ibSettings;
	ibSettings.m_numSubresources = 1;
	ibSettings.m_numElements = loadedModelFileData.indexData.size();
	m_indexBuffer = std::move(UploadBuffer<UINT>(device, commandList, frameIndex, ibInitSettings, ibSettings, trashcan));

	m_indexBufferView.BufferLocation = m_indexBuffer.getVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_indexBufferView.SizeInBytes = sizeof(UINT) * loadedModelFileData.indexData.size();
}

ModelAsset::ModelAsset(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, UINT frameIndex, ResourceTrashcan& trashcan, const GeometryData& geometryData, const void* const vertexPtr, const void* const indexPtr)
	:m_geometryData(geometryData)
{
	UploadBufferInitSettings vbInitSettings;
	vbInitSettings.m_discardUpload = true;
	vbInitSettings.m_initialData = (void*)vertexPtr;

	UploadBufferSettings vbSettings;
	vbSettings.m_numSubresources = 1;
	vbSettings.m_numElements = geometryData.m_numVertices;

	m_vertexBuffer = std::move(UploadBuffer<BasicVertex>(device, commandList, frameIndex, vbInitSettings, vbSettings, trashcan));

	int stride = sizeof(BasicVertex);
	m_vertexBufferView.BufferLocation = m_vertexBuffer.getVirtualAddress();
	m_vertexBufferView.SizeInBytes = stride * geometryData.m_numVertices;
	m_vertexBufferView.StrideInBytes = stride;

	UploadBufferInitSettings ibInitSettings;
	ibInitSettings.m_discardUpload = true;
	ibInitSettings.m_initialData = (void*)indexPtr;
	UploadBufferSettings ibSettings;
	ibSettings.m_numSubresources = 1;
	ibSettings.m_numElements = geometryData.m_numIndices;
	m_indexBuffer = std::move(UploadBuffer<UINT>(device, commandList, frameIndex, ibInitSettings, ibSettings, trashcan));

	m_indexBufferView.BufferLocation = m_indexBuffer.getVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_indexBufferView.SizeInBytes = sizeof(UINT) * geometryData.m_numIndices;
}
