#include "ModelCache.h"
#include "aZeroModelParsing/ModelParser.h"

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

	// Loading
	Helper::ModelFileData fileData;
	std::string path = directory + name;
	Helper::LoadFBXFile(fileData, path);
	std::wstring wstr(path.begin(), path.end());

	GeometryData geoData;
	geoData.m_numVertices = fileData.verticeData.size();
	geoData.m_meshName = name;
	geoData.m_numIndices = fileData.indexData.size();

	*temp = std::move(ModelAsset(device, context.getList(), frameIndex, m_trashcan, geoData, fileData));
}

void ModelCache::LoadAZModel(ID3D12Device* device, GraphicsContextHandle& context, UINT frameIndex, const std::string& name, const std::string& directory)
{
	if (resources.Exists(name))
		return;

	resources.Add(name, ModelAsset());
	ModelAsset* temp = resources.GetObjectByKey(name);

	std::optional<std::unique_ptr<aZeroFiles::LoadedModelContainer>> model = aZeroFiles::LoadAZModel(directory, name);

	if (model.has_value())
	{
		aZeroFiles::LoadedModelContainer* container = model->get();
		GeometryData geoData;
		geoData.m_numVertices = container->m_NumVertices;
		geoData.m_numIndices = container->m_NumIndices;
		geoData.m_meshName = name;

		*temp = std::move(ModelAsset(device, context.getList(), frameIndex, m_trashcan, geoData, container->m_rawVertexData, container->m_rawIndexData));
	}
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