#pragma once
#include "ResourceCache.h"
#include "VertexBuffer.h"

/** @brief Subclass of Cache. Stores loaded vertex buffers and relevant information which benefits from being loaded once.
*/
class VertexBufferCache : public ResourceCache<VertexBuffer>
{
private:

public:
	VertexBufferCache();
	~VertexBufferCache();

	/**Overriden version of ResourceCache::LoadResource(). Loads resource by filename.
	@param _device Device used to create the resource
	@param _cmdList CommandList used to create the resource
	@param _name Name of the resource
	@return void
	*/
	virtual void LoadResource(ID3D12Device* _device, CommandList* _cmdList, const std::string& _name) override;

	/**Overriden version of Cache::RemoveResource(). Removes resources by filename.
	@param _name Filename of the resource to remove
	@return void
	*/
	virtual void RemoveResource(const std::string& _name) override;

};