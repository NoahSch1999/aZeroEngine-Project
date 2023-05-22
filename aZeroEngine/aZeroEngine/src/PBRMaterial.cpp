#include "PBRMaterial.h"

PBRMaterial::PBRMaterial(ID3D12Device* device, GraphicsContextHandle& context, UINT frameIndex, Texture2DCache& _textureCache, const std::string& _fileDirectory, const std::string& _name)
	:Material()
{
	Load(device, context, frameIndex, _fileDirectory, _name, _textureCache);
}

PBRMaterial::PBRMaterial(Texture2DCache& _textureCache, const std::string& _name)
	:Material()
{
	name = _name;
	Texture* defaultText = _textureCache.GetResource("defaultDiffuse.png");
	if (defaultText)
		info.albedoMapIndex = defaultText->getSRVHandle().getHeapIndex();
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
		file.write((char*)&info.metallicMapIndex, sizeof(int));
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
		file.write((char*)&info.normalMapIndex, sizeof(int));
		Helper::WriteToFile(file, mapName);
	}
	else
	{
		int temp = -1;
		file.write((char*)&temp, sizeof(float));
	}

	// Transparency
	mapName = _textureCache.GetTextureName(info.transparencyMapIndex);
	if (mapName.length() > 0) // has texture
	{
		file.write((char*)&info.transparencyMapIndex, sizeof(int));
		Helper::WriteToFile(file, mapName);
	}
	else
	{
		int temp = -1;
		file.write((char*)&temp, sizeof(float));
	}
	file.write((char*)&info.transparencyFactor, sizeof(float));
	file.write((char*)&info.enableTransparency, sizeof(int));

	// Glow / Bloom
	mapName = _textureCache.GetTextureName(info.glowMapIndex);
	if (mapName.length() > 0) // has texture
	{
		file.write((char*)&info.glowMapIndex, sizeof(int));
		Helper::WriteToFile(file, mapName);
	}
	else
	{
		int temp = -1;
		file.write((char*)&temp, sizeof(float));
	}
	file.write((char*)&info.glowIntensity, sizeof(float));
	file.write((char*)&info.enableGlow, sizeof(int));
	file.write((char*)&info.glowFullColor, sizeof(DXM::Vector3));
}

void PBRMaterial::Load(ID3D12Device* device, GraphicsContextHandle& context, UINT frameIndex, const std::string& _fileDirectory, const std::string& _name, Texture2DCache& _textureCache)
{
	std::ifstream file(_fileDirectory + _name + ".azmpbr", std::ios::in | std::ios::binary);

	std::string mapName("");

	name = _name;

	// Albedo Map
	Helper::ReadFromFile(file, mapName);
	if (_textureCache.Exists(mapName))
	{
		info.albedoMapIndex = _textureCache.GetResource(mapName)->getSRVHandle().getHeapIndex();
	}
	else
	{
		_textureCache.LoadResource(device, context, frameIndex, mapName, "..\\textures\\");
		info.albedoMapIndex = _textureCache.GetResource(mapName)->getSRVHandle().getHeapIndex();
	}

	// Roughness Map
	file.read((char*)&info.roughnessMapIndex, sizeof(int));
	if (info.roughnessMapIndex != -1)
	{
		Helper::ReadFromFile(file, mapName);

		if (_textureCache.Exists(mapName))
		{
			info.roughnessMapIndex = _textureCache.GetResource(mapName)->getSRVHandle().getHeapIndex();
		}
		else
		{
			_textureCache.LoadResource(device, context, frameIndex, mapName, "..\\textures\\");
			info.roughnessMapIndex = _textureCache.GetResource(mapName)->getSRVHandle().getHeapIndex();
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
			info.metallicMapIndex = _textureCache.GetResource(mapName)->getSRVHandle().getHeapIndex();
		}
		else
		{
			_textureCache.LoadResource(device, context, frameIndex, mapName, "..\\textures\\");
			info.metallicMapIndex = _textureCache.GetResource(mapName)->getSRVHandle().getHeapIndex();
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
			info.normalMapIndex = _textureCache.GetResource(mapName)->getSRVHandle().getHeapIndex();
		}
		else
		{
			_textureCache.LoadResource(device, context, frameIndex, mapName, "..\\textures\\");
			info.normalMapIndex = _textureCache.GetResource(mapName)->getSRVHandle().getHeapIndex();
		}
	}

	// Transparency
	file.read((char*)&info.transparencyMapIndex, sizeof(int));
	if (info.transparencyMapIndex != -1)
	{
		Helper::ReadFromFile(file, mapName);

		if (_textureCache.Exists(mapName))
		{
			info.transparencyMapIndex = _textureCache.GetResource(mapName)->getSRVHandle().getHeapIndex();
		}
		else
		{
			_textureCache.LoadResource(device, context, frameIndex, mapName, "..\\textures\\");
			info.transparencyMapIndex = _textureCache.GetResource(mapName)->getSRVHandle().getHeapIndex();
		}
	}
	file.read((char*)&info.transparencyFactor, sizeof(float));
	file.read((char*)&info.enableTransparency, sizeof(int));

	// Glow / Bloom
	file.read((char*)&info.glowMapIndex, sizeof(int));
	if (info.glowMapIndex != -1)
	{
		Helper::ReadFromFile(file, mapName);

		if (_textureCache.Exists(mapName))
		{
			info.glowMapIndex = _textureCache.GetResource(mapName)->getSRVHandle().getHeapIndex();
		}
		else
		{
			_textureCache.LoadResource(device, context, frameIndex, mapName, "..\\textures\\");
			info.glowMapIndex = _textureCache.GetResource(mapName)->getSRVHandle().getHeapIndex();
		}
	}
	file.read((char*)&info.glowIntensity, sizeof(float));
	file.read((char*)&info.enableGlow, sizeof(int));
	file.read((char*)&info.glowFullColor, sizeof(DXM::Vector3));

}