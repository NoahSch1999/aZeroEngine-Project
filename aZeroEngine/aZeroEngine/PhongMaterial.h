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

	PhongMaterial(ID3D12Device* _device, ResourceManager* _rManager, CommandList* _cmdList, const std::string& _fileDirectory, const std::string& _name, Texture2DCache* _textureCache)
		:Material()
	{
		Load(_device, _cmdList, _rManager, _fileDirectory, _name, _textureCache);
	}

	PhongMaterial(ID3D12Device* _device, CommandList* _cmdList, Texture2DCache* _textureCache, const std::string& _name)
		:Material()
	{
		name = _name;
		info.diffuseTextureID = _textureCache->GetResource("defaultDiffuse.png").GetHandle().GetHeapIndex();
		buffer.InitDynamic(_device, _cmdList, (void*)&info, sizeof(PhongMaterialInformation), 1, true, L"Default Material Buffer");
	}
	~PhongMaterial() = default;

	// Inherited via Material
	virtual void Save(const std::string& _fileDirectory, const std::string& _name, const Texture2DCache* _textureCache, bool _debugASCII = false) override
	{
		std::string textureName = _textureCache->GetTextureName(info.diffuseTextureID);
		int numBytes = sizeof(int) + textureName.length() + sizeof(Vector3) + sizeof(Vector3) + sizeof(float);

		std::ofstream file(_fileDirectory + _name + ".azm", std::ios::trunc | std::ios::out | std::ios::binary);

		Helper::WriteToFile(file, textureName);
		file.write((char*)&info.ambientAbsorbation, sizeof(Vector3));
		file.write((char*)&info.specularAbsorbation, sizeof(Vector3));
		file.write((char*)&info.specularShine, sizeof(float));

		file.close();

		if (_debugASCII)
		{
			std::ofstream file(_fileDirectory + _name + "_ASCII.txt", std::ios::trunc | std::ios::out);
			file << "Type: Phong\n";
			file << "Diffuse Texture Name: " << textureName << "\n";
			file << "Ambient Absorbation: [" << info.ambientAbsorbation.x << ":" << info.ambientAbsorbation.y << ":" << info.ambientAbsorbation.z << "]\n";
			file << "Specular Absorbation: [" << info.specularAbsorbation.x << ":" << info.specularAbsorbation.y << ":" << info.specularAbsorbation.z << "]\n";
			file << "Specular Shininess Exponent: " << info.specularShine << "\n";
			file.close();
		}
	}

	virtual void Load(ID3D12Device* _device, CommandList* _cmdList, ResourceManager* _rManager, const std::string& _fileDirectory, const std::string& _name, Texture2DCache* _textureCache) override
	{
		std::ifstream file(_fileDirectory + "/" + _name + ".azm", std::ios::in | std::ios::binary);

		name = _name;

		std::string textureName;
		Helper::ReadFromFile(file, textureName);
		file.read((char*)&info.ambientAbsorbation, sizeof(Vector3));
		file.read((char*)&info.specularAbsorbation, sizeof(Vector3));
		file.read((char*)&info.specularShine, sizeof(float));
		file.close();

		if (_textureCache->Exists(_name))
		{
			info.diffuseTextureID = _textureCache->GetResource(textureName).GetHandle().GetHeapIndex(); // _name -> textureName
		}
		else
		{
			_textureCache->LoadResource(_device, _rManager->GetTexture2DDescriptor(), _cmdList, textureName); // _name -> textureName
			info.diffuseTextureID = _textureCache->GetResource(textureName).GetHandle().GetHeapIndex(); // _name -> textureName
		}

		std::wstring tempName(_name.begin(), _name.end());
		buffer.InitDynamic(_device, _cmdList, (void*)&info, sizeof(PhongMaterialInformation), 1, true, tempName + L" Material Buffer");
	}
};