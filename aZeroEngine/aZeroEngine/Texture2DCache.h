#pragma once
#include "ResourceCache.h"
#include "Texture2D.h"
#include "DescriptorManager.h"

/** @brief Class that stores and manages loaded Texture2D objects. A subclass of ResourceCache.
* 
* It uses dependency injection with a ResourceEngine object, which is set during the constructor of the Texture2DCache object, to create and remove DirectX12 resources.
* 
* It uses dependency injection with a DescriptorManager object, which is set during the constructor of the Texture2DCache object, to get available DescriptorHandle objects for the resource.
*/
class Texture2DCache : public ResourceCache<Texture2D>
{
private:
	std::unordered_map<int, std::string> heapIndexToStr;

public:

	/**Initiates the Texture2DCache object.
	* It also creates a default texture using a texture within the texture-folder called DefaultTexture.png.
	@param _resourceEngine ResourceEngine used for D3D12 resource creations.
	@param _dManager DescriptorManager used to get available DescriptorHandle objects from.
	*/
	Texture2DCache(ResourceEngine& _resourceEngine);

	virtual ~Texture2DCache();

	/**Initiates the Texture2DCache object.
	* It also loads a default texture using a texture within the texture-folder called DefaultTexture.png.
	@return void
	*/
	void Init();

	/**Loads a 2D texture from the disk from the specified directory.
	@param _device ID3D12Device used for D3D12 resource creations.
	@param _name Name of the 2D texture resource residing on the disk. The name has to contain the png extension.
	@param _directory Directory where the texture file should be located. Default value is the "../textures/" folder.
	@return void
	*/
	virtual void LoadResource(const std::string& _name, const std::string& _directory = "../textures/") override;

	/**Immediately removes the Texture2D from the CPU-side memory and queues the GPU-side resource to be removed ASAP.
	@param _key Name of the Texture2D to remove. Is the same as the name used during Texture2DCache::LoadResource()
	@return void
	*/
	virtual void RemoveResource(const std::string& _key) override;

	/**Immediately removes the Texture2D from the CPU-side memory and queues the GPU-side resource to be removed ASAP.
	@param _key ID of the Texture2D to remove. The ID is retrieves using ResourceCache::GetID() which this class inherits.
	@return void
	*/
	virtual void RemoveResource(int _key) override;

	/**Returns the filename corresponding to the input DescriptorManager ID3D12DescriptorHeap index or "" if no resource with the index is loaded.
	@param _index ID3D12DescriptorHeap index to get the file name for.
	@return std::string The file name if the resource is loaded, otherwise it returns "" (an empty std::string)
	*/
	std::string GetTextureName(int _index) const;

	/**Returns the DescriptorManager ID3D12DescriptorHeap index for the resource.
	@param _key Key to get the heap index for.
	@return int The heap index or -1 if no resource with the given key exists.
	*/
	int GetTextureHeapID(const std::string& _key);

	/**Returns the DescriptorManager ID3D12DescriptorHeap index for the resource.
	@param _key Key to get the heap index for.
	@return int The heap index or -1 if no resource with the given key exists.
	*/
	int GetTextureHeapID(int _key);
};

