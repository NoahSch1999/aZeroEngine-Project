#pragma once
#include <unordered_map>
#include "HelperFunctions.h"

/** @brief Virtual base class for storing and handeling loaded resources within an unordered map.
*/
template<typename T>
class ResourceCache
{
protected:
	/// \public
	std::unordered_map<std::string, T*>resourceMap;
public:
	ResourceCache() = default;
	/**Clears all the resources within the unordered map
	*/
	virtual ~ResourceCache() 
	{ 
		for (auto& [str, obj] : resourceMap)
		{
			delete obj;
		}
		resourceMap.clear(); 
	}
	/**Loads resource by filename.
	@param _device Device used to create the resource
	@param _cmdList CommandList used to create the resource
	@param _name Name of the resource
	@return void
	*/
	virtual void LoadResource(ID3D12Device*_device, CommandList* _cmdList, const std::string& _name) = 0;
	/**Removes resources by filename.
	@param _name Filename of the resource to remove
	@return void
	*/
	virtual void RemoveResource(const std::string& _name) = 0;

	/**Checks if the resource with the specified filename is loaded.
	@param _name Filename of the resource to check for
	@return true if the resource is loaded
	@return false if the resource isn't loaded
	*/
	bool Exists(const std::string& _name)
	{
		if (resourceMap.count(_name) > 0)
			return true;
		return false;
	}

	/**Return a pointer to a resource within the unordered map
	@param _name Name of the resource to get
	@return T
	*/
	T* GetResource(const std::string& _name)
	{
		return resourceMap.at(_name);
	}
};