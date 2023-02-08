#pragma once
#include "VertexBuffer.h"
#include "HelperFunctions.h"
#include "ResourceCache.h"

/** @brief Functions as a vertex buffer cache. Is used to load, store, and retrieve vertex buffers from.
* By using VertexBufferCache::GetBufferIndex() you can retrieve an index. This index can then be used with VertexBufferCache::GetBuffer(int) to get the actual resource in constant time.
*/
class VertexBufferCache : public ResourceCache<VertexBuffer>
{
private:

	void LoadVertexDataFromFile(ID3D12Device* _device, CommandList* _cmdList, ID3D12Resource*& _intermediateResource, const std::string& _path, VertexBuffer& _vBuffer);

public:
	VertexBufferCache() = default;

	virtual ~VertexBufferCache()
	{
		std::vector<VertexBuffer>& vbs = resourceMVec.GetObjects();
		for (int i = 0; i < vbs.size(); i++)
		{
			vbs[i].GetMainResource()->Release();
		}
	}

	virtual void LoadResource(ID3D12Device* _device, CommandList* _cmdList, const std::string& _name) override
	{
		VertexBuffer temp;
		ID3D12Resource* intermediateResource;
		LoadVertexDataFromFile(_device, _cmdList, intermediateResource, "../meshes/" + _name, temp);
		temp.SetFileName(_name);
		resourceMVec.Add(_name, temp);
		intermediateResources.emplace_back(intermediateResource);
		//return resourceMVec.GetID(_name);
	}

	virtual void RemoveResource(const std::string& _name) override
	{
		if (resourceMVec.Exists(_name) > 0)
		{
			resourceMVec.Remove(_name);
		}
	}

	/** Removes the buffer with the specified name.
	@param _name Name of the resource to remove from this class
	@return void
	*/
	void RemoveBuffer(const std::string& _name)
	{
		resourceMVec.Remove(_name);
	}

	/** Returns a pointer to the vertex buffer that corresponds to the input index/ID.
	@param _ID Index of the buffer to return
	@return VertexBuffer*
	*/
	VertexBuffer* GetBuffer(int _ID)
	{
		return &resourceMVec.Get(_ID);
	}

	/** Returns a pointer to the vertex buffer that corresponds to the input name.
	@param _name Name of the buffer to return
	@return VertexBuffer*
	*/
	VertexBuffer* GetBuffer(const std::string& _name)
	{
		return &resourceMVec.Get(_name);
	}

	/** Returns the index of the buffer with the input name.
	@param _name Name of the buffer to get the index from
	@return int
	*/
	int GetBufferIndex(const std::string& _name)
	{
		return resourceMVec.GetID(_name);
	}

	/** Returns whether or not the buffer with the specified name is loaded.
	@param _name Name of the buffer to look for
	@return bool (TRUE: Buffer is loaded, FALSE: Buffer isn't loaded)
	*/
	bool Exists(const std::string& _name)
	{
		if (resourceMVec.Exists(_name) > 0)
			return true;
		return false;
	}
};