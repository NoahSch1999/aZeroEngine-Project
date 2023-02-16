#pragma once
#include "PhongMaterial.h"
class PhongMaterial;

/** @brief Manages materials.
* All the functions can be used to create, remove, or get a material.
* It currently only supports materials of the type PhongMaterial.
* All functions use template specification to specify which type of material to create, remove, or get.
* For each material type, material names have to be unique.
*/
class MaterialManager
{
private:
	MappedVector<PhongMaterial>phongMaterials;
	ResourceManager* rManager;
	Texture2DCache* textureCache;
public:
	MaterialManager() = default;

	void Init(ID3D12Device* _device, CommandList& _cmdList, ResourceManager* _rManager, Texture2DCache* _textureCache) 
	{ 
		rManager = _rManager;
		textureCache = _textureCache;
		CreateMaterial<PhongMaterial>(_device, _cmdList, "DefaultPhongMaterial");
	}

	~MaterialManager() 
	{
		std::vector<int>matsToRemove;
		for (auto [name, index] : phongMaterials.GetStringToIndexMap())
		{
			matsToRemove.emplace_back(index);
		}
		for (int i = 0; i < matsToRemove.size(); i++)
		{
			RemoveMaterial<PhongMaterial>(matsToRemove[i]);
		}
	}

	/** Creates a new material and adds it to the internal MappedVector for the template specified material type.
	@param _device The main ID3D12Device instance used
	@param _cmdList The main CommandList instance to register the resource creation commands on
	@param _materialName Name of the material. Has to be unique for the specified material type. Otherwise the material won't be created
	@return void
	*/
	template<typename T>
	void CreateMaterial(ID3D12Device* _device, CommandList& _cmdList, const std::string _materialName);

	/** Loads a material (.azm) file from disk and adds it to the internal MappedVector for the template specified material type.
	@param _device The main ID3D12Device instance used
	@param _cmdList The main CommandList instance to register the resource creation commands on
	@param _materialName Name of the material. Has to be unique for the specified material type. Otherwise the material won't be created
	@return void
	*/
	template<typename T>
	void LoadMaterial(ID3D12Device* _device, CommandList& _cmdList, const std::string _materialName);

	/** Removes the material of the template specified type with the input name.
	@param _materialName Name of the material to remove. Nothing is done if it doesn't exist.
	@return void
	*/
	template<typename T>
	void RemoveMaterial(const std::string& _materialName);

	/** Removes the material of the template specified type with the input ID retrieved by MaterialManager::GetReferenceID().
	@param _ID ID of the material to remove. Nothing is done if it doesn't exist.
	@return void
	*/
	template<typename T>
	void RemoveMaterial(int _ID);

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

	std::unordered_map<std::string, int>& GetPhongStringToIndexMap() { return phongMaterials.GetStringToIndexMap(); }

	bool Exists(const std::string& _name) { return phongMaterials.Exists(_name); }
};

template<typename T>
inline void MaterialManager::CreateMaterial(ID3D12Device* _device, CommandList& _cmdList, const std::string _materialName)
{
	if constexpr (std::is_same_v<T, PhongMaterial>)
	{
		if (phongMaterials.Exists(_materialName))
			return;
		phongMaterials.Add(_materialName, PhongMaterial(_device, _cmdList, *textureCache, _materialName));
	}
}

template<typename T>
inline void MaterialManager::LoadMaterial(ID3D12Device* _device, CommandList& _cmdList, const std::string _materialName)
{
	if constexpr (std::is_same_v<T, PhongMaterial>)
	{
		if (phongMaterials.Exists(_materialName))
			return;
		phongMaterials.Add(_materialName, PhongMaterial(_device, _cmdList, *rManager, *textureCache, _materialName));
	}
}

template<typename T>
inline void MaterialManager::RemoveMaterial(const std::string& _materialName)
{
	if constexpr (std::is_same_v<T, PhongMaterial>)
	{
		if (!phongMaterials.Exists(_materialName))
			return;
		phongMaterials.Remove(_materialName);
	}
}

template<typename T>
inline void MaterialManager::RemoveMaterial(int _ID)
{
	if constexpr (std::is_same_v<T, PhongMaterial>)
	{
		if (!phongMaterials.Exists(_ID))
			return;
		phongMaterials.Remove(_ID);
	}
}

template<typename T>
inline T* MaterialManager::GetMaterial(const std::string& _materialName)
{
	if constexpr (std::is_same_v<T, PhongMaterial>)
	{
		if (!phongMaterials.Exists(_materialName))
			return nullptr;
		return &phongMaterials.Get(_materialName);
	}

	return nullptr;
}

template<typename T>
inline T* MaterialManager::GetMaterial(int _ID)
{
	if constexpr (std::is_same_v<T, PhongMaterial>)
	{
		if (!phongMaterials.Exists(_ID))
			return nullptr;
		return &phongMaterials.Get(_ID);
	}

	return nullptr;
}

template<typename T>
inline int MaterialManager::GetReferenceID(const std::string& _materialName)
{
	if constexpr (std::is_same_v<T, PhongMaterial>)
	{
		if (!phongMaterials.Exists(_materialName))
			return -1;
		return phongMaterials.GetID(_materialName);
	}

	return -1;
}