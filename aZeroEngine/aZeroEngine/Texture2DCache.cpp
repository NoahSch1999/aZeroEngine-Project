#include "Texture2DCache.h"

Texture2DCache::Texture2DCache()
	:ResourceCache()
{
}

Texture2DCache::~Texture2DCache()
{
}

void Texture2DCache::LoadResource(ID3D12Device* _device, CommandList* _cmdList, const std::string& _name)
{
	if (resourceMap.count(_name) > 0)
		return;
	resourceMap.emplace(_name, new Texture2D());
	Texture2D* tempResource = resourceMap.at(_name);
	tempResource->Init(_device, _cmdList, "../textures/", _name, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, DXGI_FORMAT_R8G8B8A8_UNORM);
}

void Texture2DCache::LoadResource(ID3D12Device* _device, DescriptorHandle _handle, CommandList* _cmdList, const std::string& _name)
{
	if (resourceMap.count(_name) > 0)
		return;
	resourceMap.emplace(_name, new Texture2D());
	Texture2D* tempResource = resourceMap.at(_name);
	tempResource->Init(_device, _cmdList, _handle, "../textures/", _name, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, DXGI_FORMAT_R8G8B8A8_UNORM);
	indexToStr.emplace(tempResource->GetHandle().GetHeapIndex(), _name);
}

void Texture2DCache::RemoveResource(const std::string& _name)
{
	if (resourceMap.count(_name) > 0)
	{
		Texture2D* tempResource = resourceMap.at(_name);
		indexToStr.erase(tempResource->GetHandle().GetHeapIndex());
		delete tempResource;
		resourceMap.erase(_name);
	}
}
