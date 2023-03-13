#include "MaterialManager.h"

MaterialManager::MaterialManager(ResourceEngine& _resourceEngine, DescriptorManager& _dManager, Texture2DCache& _textureCache)
	:resourceEngine(_resourceEngine), dManager(_dManager), textureCache(_textureCache), phongMaterials(100), pbrMaterials(100)
{
}

MaterialManager::~MaterialManager()
{
	std::vector<PhongMaterial>& pMats = phongMaterials.GetObjects();
	for (auto& mat : pMats)
	{
		resourceEngine.RemoveResource(mat.GetBufferPtr());
	}

	std::vector<PBRMaterial>& pbrMats = pbrMaterials.GetObjects();
	for (auto& mat : pbrMats)
	{
		resourceEngine.RemoveResource(mat.GetBufferPtr());
	}
}

void MaterialManager::Init(ID3D12Device* _device)
{
	CreateMaterial<PhongMaterial>(_device, "DefaultPhongMaterial");
	CreateMaterial<PBRMaterial>(_device, "DefaultPBRMaterial");
}

void MaterialManager::ShutDown()
{
	std::vector<PhongMaterial>& pMats = phongMaterials.GetObjects();
	for (auto& mat : pMats)
	{
		resourceEngine.RemoveResource(mat.GetBufferPtr());
	}

	std::vector<PBRMaterial>& pbrMats = pbrMaterials.GetObjects();
	for (auto& mat : pbrMats)
	{
		resourceEngine.RemoveResource(mat.GetBufferPtr());
	}
}

std::unordered_map<std::string, int>& MaterialManager::GetPhongStringToIndexMap()
{
	return phongMaterials.GetStrToIndexMap();
}

std::unordered_map<std::string, int>& MaterialManager::GetPBRStringToIndexMap()
{
	return pbrMaterials.GetStrToIndexMap();
}

std::vector<PhongMaterial>& MaterialManager::GetPhongMaterials()
{
	return phongMaterials.GetObjects();
}

std::vector<PBRMaterial>& MaterialManager::GetPBRMaterials()
{
	return pbrMaterials.GetObjects();
}
