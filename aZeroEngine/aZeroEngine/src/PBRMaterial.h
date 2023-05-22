#pragma once
#include "Material.h"

struct PBRMaterialInfo
{
	// PBR
	int albedoMapIndex = 0;
	int roughnessMapIndex = -1;
	int metallicMapIndex = -1;
	int normalMapIndex = -1;
	float roughnessFactor = 0.f;
	float metallicFactor = 0.f;

	// Transparency
	int enableTransparency = 0;
	int transparencyMapIndex = -1;
	float transparencyFactor = 0.f;

	// Glow / Bloom
	int enableGlow = 0;
	int glowMapIndex = -1;
	float glowIntensity = 0.f;
	DXM::Vector3 glowFullColor = DXM::Vector3(1.f, 1.f, 1.f);
};

class PBRMaterial : public Material<PBRMaterialInfo>
{
public:
	PBRMaterial() = default;

	PBRMaterial(ID3D12Device* device, GraphicsContextHandle& context, UINT frameIndex, Texture2DCache& _textureCache, const std::string& _fileDirectory, const std::string& _name);

	PBRMaterial(Texture2DCache& _textureCache, const std::string& _name);

	PBRMaterial(PBRMaterial&& _other) noexcept
	{
		info = _other.info;
		name = _other.name;
	}

	PBRMaterial& operator=(PBRMaterial&& _other) noexcept
	{
		info = _other.info;
		name = _other.name;
		return *this;
	}

	~PBRMaterial() = default;

	virtual void Save(const std::string& _fileDirectory, const Texture2DCache& _textureCache) const override;

	virtual void Load(ID3D12Device* device, GraphicsContextHandle& context, UINT frameIndex, const std::string& _fileDirectory, const std::string& _name, Texture2DCache& _textureCache) override;
};