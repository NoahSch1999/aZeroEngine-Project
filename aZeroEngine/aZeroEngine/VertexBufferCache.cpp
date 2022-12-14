#include "VertexBufferCache.h"

VertexBufferCache::VertexBufferCache()
	:ResourceCache()
{
}

VertexBufferCache::~VertexBufferCache()
{
}

void VertexBufferCache::LoadResource(ID3D12Device* _device, CommandList* _cmdList, const std::string& _name)
{
	if (resourceMap.count(_name) > 0)
		return;
	resourceMap.emplace(_name, new VertexBuffer());
	Helper::LoadVertexDataFromFile(_device, _cmdList, "../meshes/" + _name, resourceMap.at(_name));
}

void VertexBufferCache::RemoveResource(const std::string& _name)
{
	if (resourceMap.count(_name) > 0)
	{
		delete resourceMap.at(_name);
		resourceMap.erase(_name);
	}
}
