#include "Texture2DCache.h"

Texture2DCache::Texture2DCache()
	:ResourceCache()
{
}

Texture2DCache::~Texture2DCache()
{
	for (auto& [name, texture] : resourceMap)
	{
		texture.uploadBuffer->Release();
		texture.GetResource()->Release();
	}
}

void Texture2DCache::LoadResource(ID3D12Device* _device, CommandList* _cmdList, const std::string& _name)
{
	if (resourceMap.count(_name) > 0)
		return;
	Texture2D tempResource;
	tempResource.Init(_device, _cmdList, "../textures/", _name, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, DXGI_FORMAT_R8G8B8A8_UNORM);
	resourceMap.emplace(_name, tempResource);
}

void Texture2DCache::LoadResource(ID3D12Device* _device, DescriptorHandle _handle, CommandList* _cmdList, const std::string& _name)
{
	if (resourceMap.count(_name) > 0)
		return;
	Texture2D tempResource;
	tempResource.Init(_device, _cmdList, _handle, "../textures/", _name, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, DXGI_FORMAT_R8G8B8A8_UNORM);
	indexToStr.emplace(tempResource.GetHandle().GetHeapIndex(), _name);
	resourceMap.emplace(_name, tempResource);
}

void Texture2DCache::RemoveResource(const std::string& _name)
{
	if (resourceMap.count(_name) > 0)
	{
		indexToStr.erase(resourceMap.at(_name).GetHandle().GetHeapIndex());
		resourceMap.erase(_name);
	}
}
