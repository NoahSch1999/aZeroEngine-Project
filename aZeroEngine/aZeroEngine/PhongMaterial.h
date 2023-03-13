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
	PhongMaterial() = default;

	PhongMaterial(ID3D12Device* _device, ResourceEngine& _resourceEngine, DescriptorManager& _dManager, Texture2DCache& _textureCache, const std::string& _name)
		:Material()
	{
		Load(_device, _resourceEngine, _dManager, _name, _textureCache);
	}

	PhongMaterial(ID3D12Device* _device, ResourceEngine& _resourceEngine, Texture2DCache& _textureCache, const std::string& _name)
		:Material()
	{
		name = _name;
		Texture2D* t = _textureCache.GetResource("defaultDiffuse.png");
		if (t)
			info.diffuseTextureID = t->GetHandle().GetHeapIndex(); // Bug here... When i loaded a scene and switched material...out of range on the getresource.get() method.
		else
			info.diffuseTextureID = 0;

		_resourceEngine.CreateResource(_device, buffer, (void*)&info, sizeof(PhongMaterialInformation), true, true);
		//buffer.InitDynamic(_device, &_cmdList, (void*)&info, sizeof(PhongMaterialInformation), true, L"Default Material Buffer");
	}

	~PhongMaterial() = default;

	// Inherited via Material
	virtual void Save(const std::string& _fileDirectory, const Texture2DCache& _textureCache) const override
	{
		std::string textureName = _textureCache.GetTextureName(info.diffuseTextureID);
		int numBytes = sizeof(int) + (int)textureName.length() + sizeof(Vector3) + sizeof(Vector3) + sizeof(float);

		std::ofstream file(_fileDirectory + name + ".azm", std::ios::trunc | std::ios::out | std::ios::binary);

		Helper::WriteToFile(file, textureName);
		file.write((char*)&info.ambientAbsorbation, sizeof(Vector3));
		file.write((char*)&info.specularAbsorbation, sizeof(Vector3));
		file.write((char*)&info.specularShine, sizeof(float));

		file.close();
	}

	virtual void Load(ID3D12Device* _device, ResourceEngine& _resourceEngine, DescriptorManager& _dManager, const std::string& _name, Texture2DCache& _textureCache) override
	{
		std::ifstream file("..\\materials\\" + _name + ".azm", std::ios::in | std::ios::binary);

		name = _name;

		std::string textureName;
		Helper::ReadFromFile(file, textureName);
		file.read((char*)&info.ambientAbsorbation, sizeof(Vector3));
		file.read((char*)&info.specularAbsorbation, sizeof(Vector3));
		file.read((char*)&info.specularShine, sizeof(float));
		file.close();

		if (_textureCache.Exists(_name))
		{
			info.diffuseTextureID = _textureCache.GetResource(textureName)->GetHandle().GetHeapIndex(); // _name -> textureName
		}
		else
		{
			_textureCache.LoadResource(_device, "..\\textures\\", textureName); // _name -> textureName
			info.diffuseTextureID = _textureCache.GetResource(textureName)->GetHandle().GetHeapIndex(); // _name -> textureName
		}

		_resourceEngine.CreateResource(_device, buffer, (void*)&info, sizeof(PhongMaterialInformation), true, true);
	}
};