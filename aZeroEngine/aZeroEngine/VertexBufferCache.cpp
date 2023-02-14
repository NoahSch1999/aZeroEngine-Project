#include "VertexBufferCache.h"

VertexBufferCache::~VertexBufferCache()
{
	
}

void VertexBufferCache::LoadResource(ID3D12Device* _device, CommandList& _cmdList, const std::string& _name)
{
	if (resourceMVec.Exists(_name))
		return;
	VertexBuffer temp;
	ID3D12Resource* intermediateResource;
	LoadVertexDataFromFile(_device, _cmdList, intermediateResource, "..\\meshes\\" + _name, temp);
	temp.SetFileName(_name);
	resourceMVec.Add(_name, temp);
	intermediateResources.emplace_back(intermediateResource);
}

void VertexBufferCache::RemoveResource(const std::string& _name)
{
	resourceMVec.Remove(_name);
}

bool VertexBufferCache::Exists(const std::string& _name)
{
	if (resourceMVec.Exists(_name) > 0)
		return true;
	return false;
}

bool VertexBufferCache::Exists(int _ID)
{
	if (resourceMVec.Exists(_ID) > 0)
		return true;
	return false;
}

void VertexBufferCache::LoadVertexDataFromFile(ID3D12Device* _device, CommandList& _cmdList, ID3D12Resource*& _intermediateResource, const std::string& _path, VertexBuffer& _vBuffer)
{
	Helper::BasicVertexListInfo vertexInfo;
	Helper::LoadVertexListFromFile(&vertexInfo, _path);
	std::wstring wstr(_path.begin(), _path.end());
	_vBuffer.InitStatic(_device, &_cmdList, vertexInfo.verticeData.data(), vertexInfo.verticeData.size() * sizeof(BasicVertex), (int)vertexInfo.verticeData.size(), wstr);
	_vBuffer.SetNumVertices((int)vertexInfo.verticeData.size());
	_intermediateResource = _vBuffer.GetIntermediateResource();
}
