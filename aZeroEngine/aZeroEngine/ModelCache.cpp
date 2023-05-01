#include "ModelCache.h"

ModelCache::ModelCache(ResourceTrashcan& trashcan)
	:ResourceCache(trashcan) { }

ModelCache::~ModelCache() { }

void ModelCache::Init(ID3D12Device* device, GraphicsContextHandle& context, UINT frameIndex)
{
	if(!Exists("defaultCube"))
		LoadResource(device, context, frameIndex, "defaultCube", "..\\meshes\\");

	if (!Exists("defaultSphere"))
		LoadResource(device, context, frameIndex, "defaultSphere", "..\\meshes\\");
}

void ModelCache::LoadResource(ID3D12Device* device, GraphicsContextHandle& context, UINT frameIndex, const std::string& name, const std::string& directory)
{
	if (resources.Exists(name))
		return;

	resources.Add(name, ModelAsset());
	ModelAsset* temp = resources.GetObjectByKey(name);
	LoadFromFBX(device, context, frameIndex, directory + name, name, *temp);
}

void ModelCache::RemoveResource(const std::string& _key)
{
	if (resources.Exists(_key))
	{
		int id = resources.GetID(_key);
		ModelAsset* vb = resources.GetObjectByKey(id);
		resources.Remove(id);
	}
}

void ModelCache::RemoveResource(int _key)
{
	if (resources.Exists(_key))
	{
		ModelAsset* vb = resources.GetObjectByKey(_key);
		resources.Remove(_key);
	}
}

void ModelCache::LoadFromFBX(ID3D12Device* device, GraphicsContextHandle& context, UINT frameIndex,
	const std::string& _path, const std::string& _name, ModelAsset& _vBuffer)
{
	Helper::ModelFileData fileData;
	Helper::LoadFBXFile(fileData, _path);
	std::wstring wstr(_path.begin(), _path.end());

	GeometryData geoData;
	geoData.m_numVertices = fileData.verticeData.size();
	geoData.m_meshName = _name;
	geoData.m_numIndices = fileData.indexData.size();

	_vBuffer = std::move(ModelAsset(device, context.GetList(), frameIndex, m_trashcan, geoData, fileData));
}
