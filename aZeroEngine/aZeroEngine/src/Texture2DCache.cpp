#include "Texture2DCache.h"

Texture2DCache::Texture2DCache(DescriptorManager& descriptorManager, ResourceRecycler& trashcan)
	:ResourceCache(trashcan), descriptorManager(descriptorManager) { }

Texture2DCache::~Texture2DCache() { }

void Texture2DCache::Init(ID3D12Device* device, GraphicsContextHandle& context, UINT frameIndex)
{
	if(!Exists("defaultDiffuse.png"))
		LoadResource(device, context, frameIndex, "defaultDiffuse.png", "..\\textures\\");
}

void Texture2DCache::LoadResource(ID3D12Device* device, GraphicsContextHandle& context, UINT frameIndex, const std::string& name, const std::string& directory)
{
	if (resources.Exists(name))
		return;

	Helper::LoadedTextureFileData loadedContainer;

	if (Helper::LoadTextureFile(device, "../textures/" + name, loadedContainer))
	{
		D3D12_RESOURCE_DESC rDesc = loadedContainer.m_resource->GetDesc();
		Texture::Description desc;
		desc.m_height = rDesc.Height;
		desc.m_width = rDesc.Width;
		desc.m_mainFormat = rDesc.Format;
		desc.m_mipLevels = rDesc.MipLevels;
		desc.m_bytesPerTexel = 4;
		desc.m_srvFormat = rDesc.Format;
		desc.m_uavFormat = rDesc.Format;
		desc.m_usage = SRV | UAV;

		resources.Add(name, std::move(Texture(device, descriptorManager, m_trashcan, desc)));
		//resources.Add(name, std::move(Texture(device, descriptorManager, m_trashcan, context.getList(), loadedContainer, {})));
		Texture& tempResource = *resources.GetObjectByKey(name);
		tempResource.upload(device, context.getList(), loadedContainer.m_subresourceData);
		heapIndexToStr.emplace(tempResource.getSRVHandle().getHeapIndex(), name);
	}
}

void Texture2DCache::RemoveResource(const std::string& _key)
{
	if (resources.Exists(_key))
	{
		int id = resources.GetID(_key);
		heapIndexToStr.erase(resources.GetObjectByKey(id)->getSRVHandle().getHeapIndex());
		resources.Remove(id);
	}
}

void Texture2DCache::RemoveResource(int _key)
{
	if (resources.Exists(_key))
	{
		heapIndexToStr.erase(resources.GetObjectByKey(_key)->getSRVHandle().getHeapIndex());
		resources.Remove(_key);
	}
}

std::string Texture2DCache::GetTextureName(int _index) const
{
	if (heapIndexToStr.count(_index) > 0)
		return heapIndexToStr.at(_index); 
	
	return "";
}

int Texture2DCache::GetTextureHeapID(const std::string& _key)
{
	Texture* text = resources.GetObjectByKey(_key);
	if (text)
		return text->getSRVHandle().getHeapIndex();

	return -1;
}

int Texture2DCache::GetTextureHeapID(int _key)
{
	Texture* text = resources.GetObjectByKey(_key);
	if (text)
		return text->getSRVHandle().getHeapIndex();

	return -1;
}
