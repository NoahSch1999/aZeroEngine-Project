#pragma once
#include <unordered_map>
#include "HelperFunctions.h"
#include "MappedVector.h"

/** @brief Virtual base class for storing and handeling loaded resources within an unordered map.
*/
template<typename T>
class ResourceCache
{
protected:
	std::vector<ID3D12Resource*> intermediateResources;
	MappedVector<T> resourceMVec;
public:
	ResourceCache() = default;

	/**Clears all the resources within the unordered map
	*/
	virtual ~ResourceCache() 
	{
		std::vector<T>& res = resourceMVec.GetObjects();
		for (int i = 0; i < res.size(); i++)
		{
			res[i].GetMainResource()->Release();
		}

		for (auto& resource : intermediateResources)
		{
			resource->Release();
		}

		intermediateResources.clear();
	}

	/**Returns a reference to the internal resource vector
	@param _device Device used to create the resource
	@param _cmdList CommandList used to create the resource
	@param _name Name of the resource
	@return void
	*/
	std::vector<T>& GetAllResources() { return resourceMVec.GetObjects(); }

	/**Loads resource by filename.
	@param _device Device used to create the resource
	@param _cmdList CommandList used to create the resource
	@param _name Name of the resource
	@return void
	*/
	virtual void LoadResource(ID3D12Device*_device, CommandList& _cmdList, const std::string& _name) = 0;
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
		if (resourceMVec.Exists(_name) > 0)
			return true;
		return false;
	}

	/**Return a pointer to a resource within the MappedVector
	@param _name Name of the resource to get
	@return T
	*/
	T& GetResource(const std::string& _name)
	{
		return resourceMVec.Get(_name);
	}


	/**Return a pointer to a resource within the MappedVector
	@param _id ID of the resource to get
	@return T
	*/
	T& GetResource(int _id)
	{
		return resourceMVec.Get(_id);
	}

	/** Releases the intermediate resource for all static resources.
	@return void
	*/
	void ReleaseIntermediateResources()
	{
		for (int i = 0; i < intermediateResources.size(); i++)
		{
			intermediateResources[i]->Release();
		}
		intermediateResources.resize(0);
	}
};