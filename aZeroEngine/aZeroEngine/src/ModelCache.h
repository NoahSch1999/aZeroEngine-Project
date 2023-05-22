#pragma once
#include "ResourceCache.h"
#include "ModelAsset.h"

/** @brief Class that stores and manages loaded VertexBuffer objects. A subclass of ResourceCache.
* 
* It uses dependency injection with a ResourceEngine object, which is set during the constructor of the ModelCache object, to create and remove DirectX12 resources.
*/
class ModelCache : public ResourceCache<ModelAsset>
{
public:
	/**Initiates the ModelCache object.
	@param _resourceEngine ResourceEngine used for D3D12 resource creations.
	*/
	ModelCache(ResourceTrashcan& trashcan);

	virtual ~ModelCache();

	/**Initiates the VertexBufferCache object.
	* It also loads default meshes.
	@return void
	*/
	void Init(ID3D12Device* device, GraphicsContextHandle& _context, UINT frameIndex);

	/**Loads a vertex buffer from the disk from the specified directory.
	@param _name Name of the fbx file resource residing on the disk. The name has to contain a the fbx extension.
	@param _directory Directory where the fbx file should be located. Default value is the "../meshes/" folder.
	@return void
	*/
	virtual void LoadResource(ID3D12Device* device, GraphicsContextHandle& context, UINT frameIndex,
		const std::string& name, const std::string& directory = "../meshes/") override;

	void LoadAZModel(ID3D12Device* device, GraphicsContextHandle& context, UINT frameIndex,
		const std::string& name, const std::string& directory);

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