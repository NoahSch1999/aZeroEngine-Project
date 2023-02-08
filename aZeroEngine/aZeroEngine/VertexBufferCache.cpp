#include "VertexBufferCache.h"

void VertexBufferCache::LoadVertexDataFromFile(ID3D12Device* _device, CommandList* _cmdList, ID3D12Resource*& _intermediateResource, const std::string& _path, VertexBuffer& _vBuffer)
{
	Helper::BasicVertexListInfo vertexInfo;
	Helper::LoadVertexListFromFile(&vertexInfo, _path);
	std::wstring wstr(_path.begin(), _path.end());
	_vBuffer.InitStatic(_device, _cmdList, vertexInfo.verticeData.data(), vertexInfo.verticeData.size() * sizeof(BasicVertex), (int)vertexInfo.verticeData.size(), wstr);
	_vBuffer.SetNumVertices((int)vertexInfo.verticeData.size());
	_intermediateResource = _vBuffer.GetIntermediateResource();
}
