#include "VertexBufferCache.h"

VertexBufferCache::VertexBufferCache(ResourceEngine& _resourceEngine)
	:ResourceCache(_resourceEngine) { }

VertexBufferCache::~VertexBufferCache() { }

void VertexBufferCache::LoadResource(ID3D12Device* _device, const std::string& _name, const std::string& _directory)
{
	if (resources.Exists(_name))
		return;

	VertexBuffer temp;
	ID3D12Resource* intermediateResource = nullptr;
	LoadVertexDataFromFile(_device, resourceEngine, intermediateResource, _directory + _name, temp);
	temp.SetFileName(_name);
	resources.Add(_name, temp);
	resourceEngine.RemoveResource(intermediateResource);

}

void VertexBufferCache::RemoveResource(const std::string& _key)
{
	if (resources.Exists(_key))
	{
		int id = resources.GetID(_key);
		VertexBuffer* vb = resources.GetObjectByKey(id);
		resourceEngine.RemoveResource(*vb);
		resources.Remove(id);
	}
}

void VertexBufferCache::RemoveResource(int _key)
{
	if (resources.Exists(_key))
	{
		VertexBuffer* vb = resources.GetObjectByKey(_key);
		resourceEngine.RemoveResource(vb->GetMainResource());
		resources.Remove(_key);
	}
}

void VertexBufferCache::LoadVertexDataFromFile(ID3D12Device* _device, ResourceEngine& _resourceEngine, ID3D12Resource*& _intermediateResource, const std::string& _path, VertexBuffer& _vBuffer)
{
	Helper::BasicVertexListInfo vertexInfo;
	Helper::LoadVertexListFromFile(&vertexInfo, _path);
	std::wstring wstr(_path.begin(), _path.end());
	_resourceEngine.CreateResource(_device, _vBuffer, vertexInfo.verticeData.data(), vertexInfo.verticeData.size() * sizeof(BasicVertex), (int)vertexInfo.verticeData.size(), _path);
	_vBuffer.SetNumVertices((int)vertexInfo.verticeData.size());
	_intermediateResource = _vBuffer.GetIntermediateResource();
}
