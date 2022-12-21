#pragma once
#include "Material.h"

struct PhongMaterialInformation
{
	int diffuseTextureID = -1;
	Vector3 ambientAbsorbation = Vector3(1.f, 1.f, 1.f);
	Vector3 specularAbsorbation = Vector3(1.f, 1.f, 1.f);
	float specularShine = 1.f;
	PhongMaterialInformation() = default;
};

class PhongMaterial : public Material<PhongMaterialInformation>
{
public:
	PhongMaterial(ID3D12Device* _device, CommandList* _cmdList, Texture2DCache* _textureCache)
		:Material()
	{
		info.diffuseTextureID = _textureCache->GetResource("defaultDiffuse.png")->handle.heapIndex;
		buffer.InitAsDynamic(_device, _cmdList, (void*)&info, sizeof(PhongMaterialInformation), true);
	}
	~PhongMaterial() = default;
};