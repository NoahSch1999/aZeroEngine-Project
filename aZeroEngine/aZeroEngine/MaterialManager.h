#pragma once
#include "PhongMaterial.h"
#include "MappedVector.h"
#include "ResourceManager.h"


/** @brief Manages materials.
* All the functions can be used to create, remove, or get a material.
* It currently only supports materials of the type PhongMaterial.
* All functions use template specification to specify which type of material to create, remove, or get.
* For each material type, material names have to be unique.
*/
class MaterialManager
{
private:
	MappedVector<PhongMaterial>phongMaterials; // Change from PhongMaterial* to PhongMaterial!!!!
public:
	MaterialManager() = default;

	/** Creates a new material and adds it to the internal MappedVector for the template specified material type.
	@param _device The main ID3D12Device instance used
	@param _cmdList The main CommandList instance to register the resource creation commands on
	@param _handle The DescriptorHandle to be used for the bindless rendering constant buffer
	@param _textureCache The Texture2DCache instance to get default material textures from
	@param _materialName Name of the material. Has to be unique for the specified material type. Otherwise the material won't be created
	@return void
	*/
	template<typename T>
	void CreateMaterial(ID3D12Device* _device, CommandList* _cmdList, Texture2DCache* _textureCache, const std::string _materialName);

	template<typename T>
	void CreateMaterial(ID3D12Device* _device, ResourceManager* _rManager, CommandList* _cmdList, Texture2DCache* _textureCache, const std::string _fileDirectory, const std::string _materialName);

	template<typename T>
	void CreateMaterial(const T& _material);

	/** Removes the material of the template specified type with the input name.
	@param _materialName Name of the material to remove. Has to exist, otherwise there could be a potential crash
	@param _resourceManager The ResourceManager instance to free up the materials' bindless rendering constant buffer descriptor handle from
	@return void
	*/
	template<typename T>
	void RemoveMaterial(const std::string& _materialName, ResourceManager* _resourceManager);

	/** Returns a pointer to the material specified with template arguments and unique name.
	@param _materialName Name of the material to retrieve. Has to exist, otherwise there could be a potential crash
	@return pointer to the specified material instance
	*/
	template<typename T>
	T* GetMaterial(const std::string& _materialName);


	/** Returns a pointer to the material specified with template arguments and internal MappedVector vector index retrieved with MaterialManager<MaterialType>GetMaterialIDByName(unique name)
	@param _ID Vector index of the material within the internal MappedVector vector
	@return pointer to the specified material instance
	*/
	template<typename T>
	T* GetMaterial(int _ID);


	/** Returns the index of the specified material within the internal vector which the MappedVector within the MaterialManager class contains.
	* Used in conjunction with MaterialManager::GetMaterial<MaterialType>(int _ID) to avoid the inefficiency of the MaterialManager::GetMaterial<MaterialType>(const std::string& _materialName) method.
	@param _materialName Name of the material to retrieve the vector index for.
	@return pointer to the specified material instance
	*/
	template<typename T>
	int GetReferenceID(const std::string& _materialName);

	/** Decrements the material reference ID.
	@param _ID ID of the material to decrement reference count for
	@return void
	*/
	template<typename T>
	void FreeReferenceID(int _ID);

	std::vector<PhongMaterial>& GetPhongMaterials() { return phongMaterials.GetObjects(); }

	bool Exists(const std::string& _name)
	{
		return phongMaterials.Exists(_name);
	}
};

template<typename T>
inline void MaterialManager::CreateMaterial(ID3D12Device* _device, CommandList* _cmdList, Texture2DCache* _textureCache, const std::string _materialName)
{
	if constexpr (std::is_same_v<T, PhongMaterial>)
	{
		phongMaterials.Add(_materialName, PhongMaterial(_device, _cmdList, _textureCache, _materialName));
	}
}

template<typename T>
inline void MaterialManager::CreateMaterial(ID3D12Device* _device, ResourceManager* _rManager, CommandList* _cmdList, Texture2DCache* _textureCache, const std::string _fileDirectory, const std::string _materialName)
{
	if constexpr (std::is_same_v<T, PhongMaterial>)
	{
		if (phongMaterials.Exists(_materialName))
			return;
		phongMaterials.Add(_materialName, PhongMaterial(_device, _rManager, _cmdList, _fileDirectory, _materialName, _textureCache));
	}
}

template<typename T>
inline void MaterialManager::CreateMaterial(const T& _material)
{
	if constexpr (std::is_same_v<T, PhongMaterial>)
	{
		phongMaterials.Add(_material.name, _material);
	}
}

template<typename T>
inline void MaterialManager::RemoveMaterial(const std::string& _materialName, ResourceManager* _resourceManager)
{
	if constexpr (std::is_same_v<T, PhongMaterial>)
	{
		_resourceManager->FreePassDescriptor(phongMaterials.Get(_materialName).GetHandle().heapIndex);
		phongMaterials.Remove(_materialName);
	}
}

template<typename T>
inline T* MaterialManager::GetMaterial(const std::string& _materialName)
{
	if constexpr (std::is_same_v<T, PhongMaterial>)
	{
		return &phongMaterials.Get(_materialName);
	}

	return nullptr;
}

template<typename T>
inline T* MaterialManager::GetMaterial(int _ID)
{
	if constexpr (std::is_same_v<T, PhongMaterial>)
	{
		return &phongMaterials.Get(_ID);
	}

	return nullptr;
}

template<typename T>
inline int MaterialManager::GetReferenceID(const std::string& _materialName)
{
	if constexpr (std::is_same_v<T, PhongMaterial>)
	{
		phongMaterials.Get(_materialName).referenceCount++;
		return phongMaterials.GetID(_materialName);
	}

	return -1;
}

template<typename T>
inline void MaterialManager::FreeReferenceID(int _ID)
{
	if constexpr (std::is_same_v<T, PhongMaterial>)
	{
		PhongMaterial* temp = &phongMaterials.Get(_ID);
		if(temp->referenceCount > 0)
			temp->referenceCount--;
	}
}
