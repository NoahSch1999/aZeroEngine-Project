#pragma once
#include "Material.h"

struct PBRMaterialInformation
{
	int albedoMapIndex = 0;
	int roughnessMapIndex = -1;
	int metallicMapIndex = -1;
	float roughnessFactor = 0.f;
	float metallicFactor = 0.f;
};

class PBRMaterial : public Material<PBRMaterialInformation>
{
private:

public:
	PBRMaterial() = default;

	PBRMaterial(ID3D12Device* _device, ResourceEngine& _resourceEngine, DescriptorManager& _dManager, Texture2DCache& _textureCache, const std::string& _name)
		:Material()
	{
		Load(_device, _resourceEngine, _dManager, _name, _textureCache);
	}

	PBRMaterial(ID3D12Device* _device, ResourceEngine& _resourceEngine, Texture2DCache& _textureCache, const std::string& _name)
		:Material()
	{
		name = _name;
		Texture2D* t = _textureCache.GetResource("defaultDiffuse.png");
		if (t)
			info.albedoMapIndex = t->GetHandle().GetHeapIndex(); // Bug here... When i loaded a scene and switched material...out of range on the getresource.get() method.

		_resourceEngine.CreateResource(_device, buffer, (void*)&info, sizeof(PBRMaterialInformation), true, true);
	}

	~PBRMaterial() = default;

	// Inherited via Material
	virtual void Save(const std::string& _fileDirectory, const Texture2DCache& _textureCache) const override
	{
		std::ofstream file(_fileDirectory + name + ".azmpbr", std::ios::trunc | std::ios::out | std::ios::binary);

		std::string mapName("");
		
		// Albedo Map
		mapName = _textureCache.GetTextureName(info.albedoMapIndex);
		Helper::WriteToFile(file, mapName);
		
		// Roughness Map
		mapName = _textureCache.GetTextureName(info.roughnessMapIndex);
		if (mapName.length() > 0) // has texture
		{
			file.write((char*)&info.roughnessMapIndex, sizeof(float));
			Helper::WriteToFile(file, mapName);
		}
		else
		{
			int temp = -1;
			file.write((char*)&temp, sizeof(float));
		}
		file.write((char*)&info.roughnessFactor, sizeof(float));

		// Metallic Map
		mapName = _textureCache.GetTextureName(info.metallicMapIndex);
		if (mapName.length() > 0) // has texture
		{
			file.write((char*)&info.metallicMapIndex, sizeof(float));
			Helper::WriteToFile(file, mapName);
		}
		else
		{
			int temp = -1;
			file.write((char*)&temp, sizeof(float));
		}
		file.write((char*)&info.metallicFactor, sizeof(float));

	}

	virtual void Load(ID3D12Device* _device, ResourceEngine& _resourceEngine, DescriptorManager& _dManager, const std::string& _name, Texture2DCache& _textureCache) override
	{
		std::ifstream file("..\\materials\\" + _name + ".azmpbr", std::ios::in | std::ios::binary);

		name = _name;

		std::string mapName("");

		// Albedo Map
		Helper::ReadFromFile(file, mapName);
		if (_textureCache.Exists(mapName))
		{
			info.albedoMapIndex = _textureCache.GetResource(mapName)->GetHandle().GetHeapIndex();
		}
		else
		{
			_textureCache.LoadResource(_device, mapName, "..\\textures\\");
			info.albedoMapIndex = _textureCache.GetResource(mapName)->GetHandle().GetHeapIndex();
		}
		
		// Roughness Map
		file.read((char*)&info.roughnessMapIndex, sizeof(int));
		if (info.roughnessMapIndex != -1)
		{
			Helper::ReadFromFile(file, mapName);

			if (_textureCache.Exists(mapName))
			{
				info.roughnessMapIndex = _textureCache.GetResource(mapName)->GetHandle().GetHeapIndex();
			}
			else
			{
				_textureCache.LoadResource(_device, mapName, "..\\textures\\");
				info.roughnessMapIndex = _textureCache.GetResource(mapName)->GetHandle().GetHeapIndex();
			}
		}
		file.read((char*)&info.roughnessFactor, sizeof(float));

		// Metallic Map
		file.read((char*)&info.metallicMapIndex, sizeof(int));
		if (info.metallicMapIndex != -1)
		{
			Helper::ReadFromFile(file, mapName);

			if (_textureCache.Exists(mapName))
			{
				info.metallicMapIndex = _textureCache.GetResource(mapName)->GetHandle().GetHeapIndex();
			}
			else
			{
				_textureCache.LoadResource(_device, mapName, "..\\textures\\");
				info.metallicMapIndex = _textureCache.GetResource(mapName)->GetHandle().GetHeapIndex();
			}
		}
		file.read((char*)&info.metallicFactor, sizeof(float));

		_resourceEngine.CreateResource(_device, buffer, (void*)&info, sizeof(PBRMaterialInformation), true, true);
	}

};