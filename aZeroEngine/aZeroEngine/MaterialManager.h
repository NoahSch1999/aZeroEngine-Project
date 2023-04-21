#pragma once
#include "PBRMaterial.h"
#include "ResourceEngine.h"
#include "LinearResourceAllocator.h"

/** @brief This class manages materials.
* It stores each Material subclass inside their own NameSlottedMap member variable. This member functions then manage those member variables internally.
* 
* This class uses dependency injection to minimize the function call parameters. This means that the injected reference should be accessible during the lifetime of this object.

* Functions use template specification to specify which type of material to create, remove, or get.
* 
* For each material type, material names have to be unique (there can exist multiple materials with the same name as long as they aren't of the same type).
*/
class MaterialManager
{
private:
	NamedSlottedMap<PBRMaterial>pbrMaterials;

	Texture2DCache& textureCache;
	ResourceEngine& resourceEngine;

public:

	/** The input references to ResourceEngine, DescriptorManager, and Texture2DCache are copied to the member variable references.
	@param _device The main ID3D12Device instance used for creating the neccessary resources.
	*/
	MaterialManager(ResourceEngine& _resourceEngine, Texture2DCache& _textureCache);

	/** Clears all the stored materials from CPU-memory and calls ResourceEngine::RemoveResource() on the materials constant buffer.
	*/
	~MaterialManager() = default;

	/** Initiates the MaterialManager object and creates two default materials (one for the Phong shading model and one for the PBR shading model).
	@return void
	*/
	void Init();

	/** Creates a new material and adds it to the internal NameSlottedMap for the template specified material type.
	@param _materialName Name of the material. Has to be unique for the specified material type. Otherwise the material won't be created
	@return void
	*/
	template<typename T>
	void CreateMaterial(const std::string _materialName);

	/** Loads a material (.azm) file from disk and adds it to the internal NameSlottedMap for the template specified material type.
	@param _materialName Name of the material. Has to be unique for the specified material type. Otherwise the material won't be created
	@return void
	*/
	template<typename T>
	void LoadMaterial(const std::string _materialName);

	/** Removes the material of the template specified type with the input key (std::string).
	* Immediately removed the CPU-side memory, but also prepares to delete the GPU-side resources using the dependency injected ResourceEngine.
	@param _key Key for the material to remove. Nothing is done if it doesn't exist.
	@return void
	*/
	template<typename MaterialType>
	void RemoveMaterial(const std::string& _key);

	/** Removes the material of the template specified type with the input key (int).
	* Immediately removed the CPU-side memory, but also prepares to delete the GPU-side resources using the dependency injected ResourceEngine.
	* The key should be received using MaterialManager::GetReferenceID().
	@param _key Key for the material to remove. Nothing is done if it doesn't exist.
	@return void
	*/
	template<typename MaterialType>
	void RemoveMaterial(int _key);

	/** Returns a pointer to the material specified with template arguments and unique name.
	@param _key Key of the material to retrieve.
	@return template<typename MaterialType>* or a nullptr if a material with the input _key doesn't exist.
	*/
	template<typename MaterialType>
	MaterialType* GetMaterial(const std::string& _key);

	/** Returns a pointer to the material specified with template arguments and the ID.
	@param _key Key of the material to retrieve. The key should be received using MaterialManager::GetReferenceID().
	@return template<typename MaterialType>* or a nullptr if a material with the input _key doesn't exist.
	*/
	template<typename MaterialType>
	MaterialType* GetMaterial(int _key);

	/** Returns an ID used to access the actual Material subclass specified by the template parameter.
	* This method should be used to avoid the cost of having to do a std::string > int map lookup when retrieving the Material subclass data. 
	@param _materialName Name of the material to retrieve ID for.
	@return int The ID that should be used from this point forward for access of the Material subclass object. Returns -1 if the Material doesn't exist.
	*/
	template<typename T>
	int GetReferenceID(const std::string& _materialName) const;

	/** Returns a reference to the (std::string, int) std::unordered_map for the currently stored PBRMaterial objects.
	* The std::string represents the unique Material name and the int is the ID (the same ID returned by MaterialManager::GetReferenceID()).
	@param _materialName Name of the material to retrieve ID for.
	@return std::unordered_map<std::string, int>&
	*/
	std::unordered_map<std::string, int>& GetPBRStringToIndexMap();

	/** Returns a reference to the std::vector which contains all the stored PBRMaterial objects.
	* All objects are stored consecutively for easy iteration.
	@return std::vector<PBRMaterial>&
	*/
	std::vector<PBRMaterial>& GetPBRMaterials();

	/** Checks if a Material subclass with the input key is currently stored (std::string).
	* Template specification has to be used to control what Material subclass to check for.
	@param _key The key to check for.
	@return bool TRUE: If the Material exist, FALSE: If the Material doesn't exist.
	*/
	template<typename MaterialType>
	bool Exists(const std::string& _key) const;

	/** Checks if a Material subclass with the input key is currently stored (int).
	* Template specification has to be used to control what Material subclass to check for.
	* The key should be received using MaterialManager::GetReferenceID().
	@param _key The key to check for.
	@return bool TRUE: If the Material exist, FALSE: If the Material doesn't exist.
	*/
	template<typename MaterialType>
	bool Exists(int _key) const;
};

template<typename T>
inline void MaterialManager::CreateMaterial(const std::string _materialName)
{
	if constexpr (std::is_same_v<T, PBRMaterial>)
		pbrMaterials.Add(_materialName, PBRMaterial(resourceEngine, textureCache, _materialName));
}

template<typename T>
inline void MaterialManager::LoadMaterial(const std::string _materialName)
{
	if constexpr (std::is_same_v<T, PBRMaterial>)
		pbrMaterials.Add(_materialName, PBRMaterial(resourceEngine, textureCache, "../materials/", _materialName));
}

template<typename MaterialType>
inline void MaterialManager::RemoveMaterial(const std::string& _key)
{
	if constexpr (std::is_same_v<MaterialType, PBRMaterial>)
		pbrMaterials.Remove(_key);
}

template<typename MaterialType>
inline void MaterialManager::RemoveMaterial(int _key)
{
	if constexpr (std::is_same_v<MaterialType, PBRMaterial>)
		pbrMaterials.Remove(_key);
}

template<typename MaterialType>
inline MaterialType* MaterialManager::GetMaterial(const std::string& _key)
{
	if constexpr (std::is_same_v<MaterialType, PBRMaterial>)
		return pbrMaterials.GetObjectByKey(_key);

	return nullptr;
}

template<typename MaterialType>
inline MaterialType* MaterialManager::GetMaterial(int _key)
{
	if constexpr (std::is_same_v<MaterialType, PBRMaterial>)
		return pbrMaterials.GetObjectByKey(_key);

	return nullptr;
}

template<typename T>
inline int MaterialManager::GetReferenceID(const std::string& _materialName) const
{
	if constexpr (std::is_same_v<T, PBRMaterial>)
		return pbrMaterials.GetID(_materialName);

	return -1;
}

template<typename MaterialType>
inline bool MaterialManager::Exists(const std::string& _key) const
{
	if constexpr (std::is_same_v<MaterialType, PBRMaterial>)
		return pbrMaterials.Exists(_key);

	return false;
}

template<typename MaterialType>
inline bool MaterialManager::Exists(int _key) const
{
	if constexpr (std::is_same_v<MaterialType, PBRMaterial>)
		return pbrMaterials.Exists(_key);

	return false;
}