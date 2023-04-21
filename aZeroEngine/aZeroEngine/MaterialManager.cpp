#include "MaterialManager.h"

MaterialManager::MaterialManager(ResourceEngine& _resourceEngine, Texture2DCache& _textureCache)
	:resourceEngine(_resourceEngine), textureCache(_textureCache), pbrMaterials(500)
{
}

void MaterialManager::Init()
{
	CreateMaterial<PBRMaterial>("DefaultPBRMaterial");
}

std::unordered_map<std::string, int>& MaterialManager::GetPBRStringToIndexMap()
{
	return pbrMaterials.GetStrToIndexMap();
}

std::vector<PBRMaterial>& MaterialManager::GetPBRMaterials()
{
	return pbrMaterials.GetObjects();
}
