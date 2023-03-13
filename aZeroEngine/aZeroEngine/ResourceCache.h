#pragma once
#include <unordered_map>
#include "HelperFunctions.h"
#include "ResourceEngine.h"
#include "ECSBase.h"

/** @brief Virtual base class for storing and handeling loaded resources within a NamedSlottedMap.
*/
template<typename T>
class ResourceCache
{
protected:
	ResourceEngine& resourceEngine;
	NamedSlottedMap<T> resources;
public:
	ResourceCache(ResourceEngine& _resourceEngine)
		:resourceEngine(_resourceEngine), resources(100, 100) { }

	/**Clears all the resources within the unordered map
	*/
	virtual ~ResourceCache();

	void ShutDown();

	/**Returns a reference to the resources.
	@return std::vector<T>&
	*/
	std::vector<T>& GetAllResources() { return resources.GetObjects(); }

	/**Returns a reference to the std::unordered_map<std::string, int> of the NamedSlottedMap.
	@return std::unordered_map<std::string, int>&
	*/
	std::unordered_map<std::string, int>& GetStringToIndexMap() { return resources.GetStrToIndexMap(); }

	/** Pure virtual method that should be overwritten by the inheriting subclass.
	* Loads resources from file.
	@param _device ID3D12Device used to create the neccessary resources.
	@param _name Name of the resource to load.
	@param _directory The directory of the resource to load.
	@return void
	*/
	virtual void LoadResource(ID3D12Device* _device, const std::string& _name, const std::string& _directory) = 0;

	/** Pure virtual method that should be overwritten by the inheriting subclass.
	* Removes resources by filename.
	@param _key Filename of the resource to remove.
	@return void
	*/
	virtual void RemoveResource(const std::string& _key) = 0;

	/** Pure virtual method that should be overwritten by the inheriting subclass.
	* Removes resources by ID.
	@param _key ID of the resource to remove.
	@return void
	*/
	virtual void RemoveResource(int _key) = 0;

	/** Returns the ID of the loaded resource which can be used in conjunction with the other member functions to avoid using the file name.
	* If no resource with the specified file name is loaded, the method will return -1 instead of the actual ID.
	@param _fileName File name of the loaded resource which was used when loading the resource using ResourceCache::LoadResource().
	@return int The ID for the resource
	*/
	int GetID(const std::string& _fileName) const { return resources.GetID(_fileName); }

	/** Returns the file name corresponding to the input ID retrieved by ResourceCache::GetID().
	* If no resource with the ID is loaded, the method returns an empty std::string ("").
	@param _ID The ID to get the file name for.
	@return std::string
	*/
	std::string GetFileName(int _ID) const { return resources.GetString(_ID); }

	/**Checks if the resource with the specified filename is loaded.
	@param _name Filename of the resource to check for.
	@return bool TRUE: If the resource is loaded, FALSE: If the resource isn't loaded
	*/
	bool Exists(const std::string& _name)
	{
		return resources.Exists(_name);
	}

	/**Checks if the resource with the specified ID is loaded.
	@param _ID ID of the resource to check for
	@return bool TRUE: If the resource is loaded, FALSE: If the resource isn't loaded
	*/
	bool Exists(int _ID)
	{
		return resources.Exists(_ID);
	}

	/**Return a pointer to a resource within the NameSlottedMap.
	@param _name Name of the resource to get.
	@return T* or nullptr if the resources isn't loaded.
	*/
	T* GetResource(const std::string& _name)
	{
		return resources.GetObjectByKey(_name);
	}

	/**Return a pointer to a resource within the NameSlottedMap.
	@param _ID ID of the resource to get.
	@return T* or nullptr if the resources isn't loaded.
	*/
	T* GetResource(int _ID)
	{
		return resources.GetObjectByKey(_ID);
	}
};

template<typename T>
inline ResourceCache<T>::~ResourceCache()
{
	/*for (auto& res : resources.GetObjects())
	{
		resourceEngine.RemoveResource(res);
	}
	resources.Reset();*/
}

template<typename T>
inline void ResourceCache<T>::ShutDown()
{
	for (auto& res : resources.GetObjects())
	{
		resourceEngine.RemoveResource(res);
	}
	resources.Reset();
}
