#include "Texture2DCache.h"

Texture2DCache::Texture2DCache(ResourceEngine& _resourceEngine, DescriptorManager& _dManager)
	:ResourceCache(_resourceEngine), dManager(_dManager) { }

Texture2DCache::~Texture2DCache() { }

void Texture2DCache::Init(ID3D12Device* _device)
{
	if(!Exists("defaultDiffuse.png"))
		LoadResource(_device, "defaultDiffuse.png", "..\\textures\\");
}

void Texture2DCache::LoadResource(ID3D12Device* _device, const std::string& _name, const std::string& _directory)
{
	if (resources.Exists(_name))
		return;

	ID3D12Resource* intermediateResource = nullptr;
	Texture2D r;

	int id = resources.Add(_name, r);
	Texture2D* tempResource = resources.GetObjectByKey(id);
	Helper::STBIImageData loadedData = Helper::LoadSTBIImage("../textures/" + _name);
	resourceEngine.CreateResource(_device, *tempResource, dManager.GetTexture2DDescriptor(), loadedData.rawData, loadedData.width,
		loadedData.height, loadedData.channels, DXGI_FORMAT_R8G8B8A8_UNORM, _name);
	
#ifdef DEBUG
	std::wstring wstrName(_name.begin(), _name.end());
	tempResource.GetMainResource()->SetName(wstrName.c_str());
#endif // DEBUG

	heapIndexToStr.emplace(tempResource->GetHandle().GetHeapIndex(), _name);
	resourceEngine.RemoveResource(intermediateResource);
}

void Texture2DCache::RemoveResource(const std::string& _key)
{
	if (resources.Exists(_key))
	{
		int id = resources.GetID(_key);
		dManager.FreeTexture2DDescriptor(resources.GetObjectByKey(id)->GetHandle().GetHeapIndex());
		heapIndexToStr.erase(resources.GetObjectByKey(id)->GetHandle().GetHeapIndex());
		Texture2D* t = resources.GetObjectByKey(id);
		resourceEngine.RemoveResource(*t);
		resources.Remove(id);
	}
}

void Texture2DCache::RemoveResource(int _key)
{
	if (resources.Exists(_key))
	{
		dManager.FreeTexture2DDescriptor(resources.GetObjectByKey(_key)->GetHandle().GetHeapIndex());
		heapIndexToStr.erase(resources.GetObjectByKey(_key)->GetHandle().GetHeapIndex());
		Texture2D* t = resources.GetObjectByKey(_key);
		resourceEngine.RemoveResource(*t);
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
	Texture2D* text = resources.GetObjectByKey(_key);
	if (text)
		return text->GetHandle().GetHeapIndex();

	return -1;
}

int Texture2DCache::GetTextureHeapID(int _key)
{
	Texture2D* text = resources.GetObjectByKey(_key);
	if (text)
		return text->GetHandle().GetHeapIndex();

	return -1;
}
