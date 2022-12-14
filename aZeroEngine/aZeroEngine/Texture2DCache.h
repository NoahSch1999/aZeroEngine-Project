#pragma once
#include "ResourceCache.h"
#include "ShaderResource.h"

/** @brief Class that stores and manages 2D textures. A subclass of ResourceCache.
*/
class Texture2DCache : public ResourceCache<ShaderResource>
{
private:

public:
	Texture2DCache();
	~Texture2DCache();

	/**Loads a 2D texture from the disk.
	* Inherited via ResourceCache
	@param _device The main ID3D12Device instance used
	@param _cmdList The main CommandList instance to register the resource creation command on
	@param _name Name of the 2D texture resource residing on the disk. The application will look within the "../textures/" folder.
	The parameter name has to contain a jpg or png extension.
	@return void
	*/
	virtual void LoadResource(ID3D12Device* _device, CommandList* _cmdList, const std::string& _name) override;

	/**Loads a 2D texture from the disk.
	@param _device The main ID3D12Device instance used
	@param _handle A descriptor handle which will be used for the loaded resource
	@param _cmdList The main CommandList instance to register the resource creation command on
	@param _name Name of the 2D texture resource residing on the disk. The application will look within the "../textures/" folder.
	The parameter name has to contain a jpg or png extension.
	@return void
	*/
	void LoadResource(ID3D12Device* _device, DescriptorHandle _handle, CommandList* _cmdList, const std::string& _name);

	/**Removes a resource from the cache and memory.
	@param _name Name of the resource to remove. Is the same as the name used during Texture2DCache::LoadResource()
	@return void
	*/
	virtual void RemoveResource(const std::string& _name) override;
};

