#include "MaterialManager.h"

MaterialManager::MaterialManager(Texture2DCache& _textureCache)
	:textureCache(_textureCache), pbrMaterials(500)
{
}

void MaterialManager::Init(ID3D12Device* device, GraphicsContextHandle& context, UINT frameIndex)
{
	LoadMaterial<PBRMaterial>(device, context, frameIndex, "DefaultPBRMaterial");
}

std::unordered_map<std::string, int>& MaterialManager::GetPBRStringToIndexMap()
{
	return pbrMaterials.GetStrToIndexMap();
}

std::vector<PBRMaterial>& MaterialManager::GetPBRMaterials()
{
	return pbrMaterials.GetObjects();
}
