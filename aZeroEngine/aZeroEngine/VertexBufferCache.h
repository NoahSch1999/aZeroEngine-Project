#pragma once
#include "VertexBuffer.h"
#include "MappedVector.h"
#include "HelperFunctions.h"

/** @brief Functions as a vertex buffer cache. Is used to load, store, and retrieve vertex buffers from.
* By using VertexBufferCache::GetBufferIndex() you can retrieve an index. This index can then be used with VertexBufferCache::GetBuffer(int) to get the actual resource in constant time.
*/
class VertexBufferCache
{
private:
	MappedVector<VertexBuffer>vertexBuffers;
public:
	VertexBufferCache() = default;

	~VertexBufferCache()
	{
		std::vector<VertexBuffer>& vbs = vertexBuffers.GetObjects();
		for (int i = 0; i < vbs.size(); i++)
		{
			vbs[i].uploadBuffer->Release();
			vbs[i].GetResource()->Release();
		}
	}

	/** Loads an fbx file with the specified name into a vertex buffer which in turn is stored inside this class.
	@param _device Device to use when creating the D3D12 resources
	@param _cmdList CommandList to record the necessary D3D12 command on
	@param _name Filename of the fbx file to load
	@return int That can be used to retrieve the buffer with VertexBufferCache::GetBuffer(int)
	*/
	int LoadBuffer(ID3D12Device* _device, CommandList* _cmdList, const std::string& _name)
	{
	  VertexBuffer temp;
	  Helper::LoadVertexDataFromFile(_device, _cmdList, "../meshes/" + _name, temp);
	  temp.SetFileName(_name);
	  vertexBuffers.Add(_name, temp);
		  return vertexBuffers.GetID(_name);
	}

	/** Removes the buffer with the specified name.
	@param _name Name of the resource to remove from this class
	@return void
	*/
	void RemoveBuffer(const std::string& _name)
	{
		vertexBuffers.Remove(_name);
	}

	/** Returns a pointer to the vertex buffer that corresponds to the input index/ID.
	@param _ID Index of the buffer to return
	@return VertexBuffer*
	*/
	VertexBuffer* GetBuffer(int _ID)
	{
		return &vertexBuffers.Get(_ID);
	}

	/** Returns a pointer to the vertex buffer that corresponds to the input name.
	@param _name Name of the buffer to return
	@return VertexBuffer*
	*/
	VertexBuffer* GetBuffer(const std::string& _name)
	{
		return &vertexBuffers.Get(_name);
	}

	/** Returns the index of the buffer with the input name.
	@param _name Name of the buffer to get the index from
	@return int
	*/
	int GetBufferIndex(const std::string& _name)
	{
		return vertexBuffers.GetID(_name);
	}

	/** Returns whether or not the buffer with the specified name is loaded.
	@param _name Name of the buffer to look for
	@return bool (TRUE: Buffer is loaded, FALSE: Buffer isn't loaded)
	*/
	bool Exists(const std::string& _name)
	{
		if (vertexBuffers.Exists(_name) > 0)
			return true;
		return false;
	}
};