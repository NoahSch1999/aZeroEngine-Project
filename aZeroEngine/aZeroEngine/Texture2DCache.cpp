#include "Texture2DCache.h"

Texture2DCache::Texture2DCache()
	:ResourceCache()
{
}

Texture2DCache::~Texture2DCache()
{
}

void Texture2DCache::Init(ID3D12Device* _device, CommandList& _cmdList, ResourceManager& _rManager)
{
	if(!Exists("defaultDiffuse.png"))
		LoadResource(_device, _cmdList, _rManager, "defaultDiffuse.png");
}

void Texture2DCache::LoadResource(ID3D12Device* _device, CommandList& _cmdList, const std::string& _name)
{
	static_assert(true, "Unavailable method");
}

void Texture2DCache::LoadResource(ID3D12Device* _device, CommandList& _cmdList, ResourceManager& _rManager, const std::string& _name)
{
	if (resourceMVec.Exists(_name) > 0)
		return;
	ID3D12Resource* intermediateResource;
	Texture2D tempResource;
	tempResource.Init(_device, &_cmdList, intermediateResource, _rManager.GetTexture2DDescriptor(), "../textures/", _name, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, DXGI_FORMAT_R8G8B8A8_UNORM);
	indexToStr.emplace(tempResource.GetHandle().GetHeapIndex(), _name);
	intermediateResources.emplace_back(intermediateResource);
	resourceMVec.Add(_name, tempResource);
}

void Texture2DCache::RemoveResource(const std::string& _name)
{
	if (resourceMVec.Exists(_name) > 0)
	{
		resourceMVec.Remove(_name);
	}
}
