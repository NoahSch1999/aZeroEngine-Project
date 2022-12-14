#pragma once
#include "Texture2DCache.h"
#include "ConstantBuffer.h"

struct PhongMaterialInformation
{
	int diffuseTextureID = -1;
	Vector3 ambientAbsorbation = Vector3(1.f, 1.f, 1.f);
	Vector3 specularAbsorbation = Vector3(1.f, 1.f, 1.f);
	float specularShine = 1.f;
};

class PhongMaterial
{
private:
	PhongMaterialInformation info;
	ConstantBuffer cb;
public:
	PhongMaterial(ID3D12Device* _device, CommandList* _cmdList, DescriptorHandle _descHandle, Texture2DCache* _textureCache)
	{
		info.diffuseTextureID = _textureCache->GetResource("defaultDiffuse.png")->handle.heapIndex;
		cb.InitAsDynamic(_device, _cmdList, (void*)&info, sizeof(PhongMaterialInformation));
		cb.handle = _descHandle;
		cb.InitAsCBV(_device);
	}
	~PhongMaterial() = default;

	PhongMaterialInformation* GetInfoPtr() { return &info; }
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() { return cb.GetGPUAddress(); }
	DescriptorHandle GetHandle() { return cb.handle; }

	void Update() { cb.Update((void*)&info, sizeof(PhongMaterialInformation)); }
};