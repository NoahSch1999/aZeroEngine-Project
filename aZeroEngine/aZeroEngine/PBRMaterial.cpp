#include "PBRMaterial.h"

PBRMaterial::PBRMaterial(ResourceEngine& _resourceEngine, Texture2DCache& _textureCache, const std::string& _fileDirectory, const std::string& _name)
	:Material()
{
	Load(_resourceEngine, _fileDirectory, _name, _textureCache);
}

PBRMaterial::PBRMaterial(ResourceEngine& _resourceEngine, Texture2DCache& _textureCache, const std::string& _name)
	:Material()
{
	name = _name;
	Texture2D* defaultText = _textureCache.GetResource("defaultDiffuse.png");
	if (defaultText)
		info.albedoMapIndex = defaultText->GetHandle().GetHeapIndex();

	_resourceEngine.CreateResource(buffer, (void*)&info, sizeof(PBRMaterialInformation), true, true);
}

void PBRMaterial::Save(const std::string& _fileDirectory, const Texture2DCache& _textureCache) const
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

	// Normal Map
	mapName = _textureCache.GetTextureName(info.normalMapIndex);
	if (mapName.length() > 0) // has texture
	{
		file.write((char*)&info.normalMapIndex, sizeof(float));
		Helper::WriteToFile(file, mapName);
	}
	else
	{
		int temp = -1;
		file.write((char*)&temp, sizeof(float));
	}
}

void PBRMaterial::Load(ResourceEngine& _resourceEngine, const std::string& _fileDirectory, const std::string& _name, Texture2DCache& _textureCache)
{
	std::ifstream file(_fileDirectory + _name + ".azmpbr", std::ios::in | std::ios::binary);

	std::string mapName("");

	name = _name;

	// Albedo Map
	Helper::ReadFromFile(file, mapName);
	if (_textureCache.Exists(mapName))
	{
		info.albedoMapIndex = _textureCache.GetResource(mapName)->GetHandle().GetHeapIndex();
	}
	else
	{
		_textureCache.LoadResource(mapName, "..\\textures\\");
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
			_textureCache.LoadResource(mapName, "..\\textures\\");
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
			_textureCache.LoadResource(mapName, "..\\textures\\");
			info.metallicMapIndex = _textureCache.GetResource(mapName)->GetHandle().GetHeapIndex();
		}
	}
	file.read((char*)&info.metallicFactor, sizeof(float));

	// Normal Map
	file.read((char*)&info.normalMapIndex, sizeof(int));
	if (info.normalMapIndex != -1)
	{
		Helper::ReadFromFile(file, mapName);

		if (_textureCache.Exists(mapName))
		{
			info.normalMapIndex = _textureCache.GetResource(mapName)->GetHandle().GetHeapIndex();
		}
		else
		{
			_textureCache.LoadResource(mapName, "..\\textures\\");
			info.normalMapIndex = _textureCache.GetResource(mapName)->GetHandle().GetHeapIndex();
		}
	}

	_resourceEngine.CreateResource(buffer, (void*)&info, sizeof(PBRMaterialInformation), true, true);
}