#pragma once
#include "VertexBuffer.h"
#include "ResourceCache.h"

/** @brief Class that stores and manages loaded VertexBuffer objects. A subclass of ResourceCache.
* 
* It uses dependency injection with a ResourceEngine object, which is set during the constructor of the VertexBufferCache object, to create and remove DirectX12 resources.
*/
class VertexBufferCache : public ResourceCache<VertexBuffer>
{
private:
	void LoadVertexDataFromFile(ID3D12Device* _device, ResourceEngine& _resourceEngine, ID3D12Resource*& _intermediateResource, const std::string& _path, VertexBuffer& _vBuffer);

public:
	/**Initiates the VertexBufferCache object.
	@param _resourceEngine ResourceEngine used for D3D12 resource creations.
	*/
	VertexBufferCache(ResourceEngine& _resourceEngine);

	virtual ~VertexBufferCache();

	/**Loads a vertex buffer from the disk from the specified directory.
	@param _device ID3D12Device used for D3D12 resource creations.
	@param _name Name of the fbx file resource residing on the disk. The name has to contain a the fbx extension.
	@param _directory Directory where the fbx file should be located. Default value is the "../meshes/" folder.
	@return void
	*/
	virtual void LoadResource(ID3D12Device* _device, const std::string& _name, const std::string& _directory = "../meshes/") override;

	/**Immediately removes the vertex buffer from the CPU-side memory and queues the GPU-side resource to be removed ASAP.
	* The key is the std::string which is the same as the filename for the loaded resource.
	@param _key Name of the vertex buffer to remove. Is the same as the name used during VertexBufferCache::LoadResource()
	@return void
	*/
	virtual void RemoveResource(const std::string& _key) override;

	/**Immediately removes the VertexBuffer from the CPU-side memory and queues the GPU-side resource to be removed ASAP.
	@param _key ID of the VertexBuffer to remove. The ID is retrieves using ResourceCache::GetID() which this class inherits.
	@return void
	*/
	virtual void RemoveResource(int _key) override;
};