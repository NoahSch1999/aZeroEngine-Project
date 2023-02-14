#pragma once
#include "ResourceCache.h"
#include "Texture2D.h"
#include "ResourceManager.h"

/** @brief Class that stores and manages 2D textures. A subclass of ResourceCache.
*/
class Texture2DCache : public ResourceCache<Texture2D>
{
private:
	std::unordered_map<int, std::string> indexToStr;
public:
	Texture2DCache();

	virtual ~Texture2DCache();

	/**Initiates the Texture2DCache object.
	* It also creates a default texture.
	@param _device ID3D12Device used for D3D12 resource creations.
	@param _cmdList The main CommandList instance to register the resource creation command on.
	@param _rManager ResourceManager to use for the default texture creation.
	@return void
	*/
	void Init(ID3D12Device* _device, CommandList& _cmdList, ResourceManager& _rManager);

	/**Loads a 2D texture from the disk.
	* Inherited via ResourceCache
	@param _device ID3D12Device used for D3D12 resource creations.
	@param _cmdList The main CommandList instance to register the resource creation command on.
	@param _name Name of the 2D texture resource residing on the disk. The application will look within the "../textures/" folder.
	The parameter name has to contain a jpg or png extension.
	@return void
	*/
	virtual void LoadResource(ID3D12Device* _device, CommandList& _cmdList, const std::string& _name) override;

	/**Loads a 2D texture from the disk.
	@param _device ID3D12Device used for D3D12 resource creations.
	@param _cmdList The main CommandList instance to register the resource creation command on.
	@param _rManager ResourceManager to acquire an empty Texture2D DescriptorHandle from.
	@param _name Name of the 2D texture resource residing on the disk. The application will look within the "../textures/" folder. The parameter name has to contain a jpg or png extension.
	@return void
	*/
	void LoadResource(ID3D12Device* _device, CommandList& _cmdList, ResourceManager& _rManager, const std::string& _name);

	/**Removes a texture from the cache and memory.
	* Inherited via ResourceCache
	@param _name Name of the texture to remove. Is the same as the name used during Texture2DCache::LoadResource()
	@return void
	*/
	virtual void RemoveResource(const std::string& _name) override;

	/**Returns the filename of the input index.
	@param _index Index to get the matching filename for.
	@return const std::string
	*/
	const std::string GetTextureName(int _index) const { if (indexToStr.count(_index) > 0) { return indexToStr.at(_index); } }
};

