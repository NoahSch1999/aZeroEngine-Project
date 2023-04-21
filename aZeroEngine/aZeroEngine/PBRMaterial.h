#pragma once
#include "Material.h"

struct PBRMaterialInfo
{
	int albedoMapIndex = 0;
	int roughnessMapIndex = -1;
	int metallicMapIndex = -1;
	int normalMapIndex = -1;
	float roughnessFactor = 0.f;
	float metallicFactor = 0.f;
};

class PBRMaterial : public Material<PBRMaterialInfo>
{
public:
	PBRMaterial() = default;

	PBRMaterial(ResourceEngine& _resourceEngine, Texture2DCache& _textureCache, const std::string& _fileDirectory, const std::string& _name);

	PBRMaterial(ResourceEngine& _resourceEngine, Texture2DCache& _textureCache, const std::string& _name);

	~PBRMaterial() = default;

	virtual void Save(const std::string& _fileDirectory, const Texture2DCache& _textureCache) const override;

	virtual void Load(ResourceEngine& _resourceEngine, const std::string& _fileDirectory, const std::string& _name, Texture2DCache& _textureCache) override;
};