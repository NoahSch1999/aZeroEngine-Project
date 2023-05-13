#include "Texture2DCache.h"

Texture2DCache::Texture2DCache(DescriptorManager& descriptorManager, ResourceTrashcan& trashcan)
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

	Helper::STBIImageData loadedData = Helper::LoadSTBIImage("../textures/" + name);

	int id = resources.Add(name, Texture());
	Texture& tempResource = *resources.GetObjectByKey(id);

	TextureSettings settings;
	/*settings.m_createReadback = false;*/
	settings.m_flags = D3D12_RESOURCE_FLAG_NONE;
	settings.m_bytesPerTexel = loadedData.channels;
	settings.m_height = loadedData.height;
	settings.m_width = loadedData.width;
	settings.m_initialState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	settings.m_srvFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	settings.m_clearValue.Color[0] = 0;
	settings.m_clearValue.Color[1] = 0;
	settings.m_clearValue.Color[2] = 0;
	settings.m_clearValue.Color[3] = 0;
	settings.m_uploadSettings.m_discardUpload = true;
	settings.m_uploadSettings.m_initialData = loadedData.rawData;

	tempResource = std::move(Texture(device, context.getList(), settings, descriptorManager, m_trashcan));

	heapIndexToStr.emplace(tempResource.getSRVHandle().getHeapIndex(), name);
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
