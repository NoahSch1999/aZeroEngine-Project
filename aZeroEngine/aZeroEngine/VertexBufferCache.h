#pragma once
#include "VertexBuffer.h"
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

	virtual ~VertexBufferCache();

	/**Loads a vertex buffer from the disk.
	* Inherited via ResourceCache
	@param _device The main ID3D12Device instance used
	@param _cmdList The main CommandList instance to register the resource creation command on
	@param _name Name of the fbx file residing on the disk without the .fbx extension. The application will look within the "../meshes/" folder.
	@return void
	*/
	virtual void LoadResource(ID3D12Device* _device, CommandList* _cmdList, const std::string& _name) override;

	/**Removes the vertex buffer from the VertexBufferCache and memory.
	* Inherited via ResourceCache
	@param _name Name of the vertex buffer to remove. Is the same as the name used during VertexBufferCache::LoadResource()
	@return void
	*/
	virtual void RemoveResource(const std::string& _name) override;

	/** Removes the buffer with the specified name.
	@param _name Name of the resource to remove from this class
	@return void
	*/
	void RemoveBuffer(const std::string& _name) { resourceMVec.Remove(_name); }

	/** Returns a pointer to the vertex buffer that corresponds to the input index/ID.
	@param _ID Index of the buffer to return
	@return VertexBuffer*
	*/
	VertexBuffer* GetBuffer(int _ID){ return &resourceMVec.Get(_ID); }

	/** Returns a pointer to the vertex buffer that corresponds to the input name.
	@param _name Name of the buffer to return
	@return VertexBuffer*
	*/
	VertexBuffer* GetBuffer(const std::string& _name){ return &resourceMVec.Get(_name); }

	/** Returns the index of the buffer with the input name.
	@param _name Name of the buffer to get the index from
	@return int
	*/
	int GetBufferIndex(const std::string& _name){ return resourceMVec.GetID(_name); }

	/** Returns whether or not the buffer with the specified name is loaded.
	@param _name Name of the buffer to look for
	@return bool (TRUE: Buffer is loaded, FALSE: Buffer isn't loaded)
	*/
	bool Exists(const std::string& _name);
};