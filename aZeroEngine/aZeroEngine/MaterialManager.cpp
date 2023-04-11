#include "MaterialManager.h"

MaterialManager::MaterialManager(ResourceEngine& _resourceEngine, Texture2DCache& _textureCache)
	:resourceEngine(_resourceEngine), textureCache(_textureCache), phongMaterials(100), pbrMaterials(100)
{
}

MaterialManager::~MaterialManager()
{
	std::vector<PhongMaterial>& pMats = phongMaterials.GetObjects();
	for (auto& mat : pMats)
	{
		resourceEngine.RemoveResource(mat.GetBuffer());
	}

	std::vector<PBRMaterial>& pbrMats = pbrMaterials.GetObjects();
	for (auto& mat : pbrMats)
	{
		resourceEngine.RemoveResource(mat.GetBuffer());
	}
}

void MaterialManager::Init()
{
	CreateMaterial<PhongMaterial>("DefaultPhongMaterial");
	CreateMaterial<PBRMaterial>("DefaultPBRMaterial");
}

void MaterialManager::ShutDown()
{
	std::vector<PhongMaterial>& pMats = phongMaterials.GetObjects();
	for (auto& mat : pMats)
	{
		resourceEngine.RemoveResource(mat.GetBuffer());
	}

	std::vector<PBRMaterial>& pbrMats = pbrMaterials.GetObjects();
	for (auto& mat : pbrMats)
	{
		resourceEngine.RemoveResource(mat.GetBuffer());
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
