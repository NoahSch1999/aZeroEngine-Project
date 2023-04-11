#include "VertexBufferCache.h"

VertexBufferCache::VertexBufferCache(ResourceEngine& _resourceEngine)
	:ResourceCache(_resourceEngine) { }

VertexBufferCache::~VertexBufferCache() { }

void VertexBufferCache::Init()
{
	if(!Exists("defaultCube"))
		LoadResource("defaultCube", "..\\meshes\\");

	if (!Exists("defaultSphere"))
		LoadResource("defaultSphere", "..\\meshes\\");
}

void VertexBufferCache::LoadResource(const std::string& _name, const std::string& _directory)
{
	if (resources.Exists(_name))
		return;

	VertexBuffer temp;
	LoadVertexDataFromFile(resourceEngine, _directory + _name, temp);
	temp.SetName(_name);
	resources.Add(_name, temp);
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
		resourceEngine.RemoveResource(vb->GetGPUOnlyResource());
		resources.Remove(_key);
	}
}

void VertexBufferCache::LoadVertexDataFromFile(ResourceEngine& _resourceEngine, const std::string& _path, VertexBuffer& _vBuffer)
{
	Helper::BasicVertexListInfo vertexInfo;
	Helper::LoadVertexListFromFile(&vertexInfo, _path);
	std::wstring wstr(_path.begin(), _path.end());
	_resourceEngine.CreateResource(_vBuffer, vertexInfo.verticeData.data(), vertexInfo.verticeData.size() * sizeof(BasicVertex), (int)vertexInfo.verticeData.size(), _path);
	_resourceEngine.RemoveResource(_vBuffer.GetUploadResource());
	_vBuffer.SetNumVertices((int)vertexInfo.verticeData.size());
}
