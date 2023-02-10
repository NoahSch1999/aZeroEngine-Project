#pragma once
#include "Texture2DCache.h"
#include "ConstantBuffer.h"
#include "ResourceManager.h"

class Texture2DCache;
class ConstantBuffer;
class ResourceManager;

template<typename T>
class Material
{
protected:
	T info;
	ConstantBuffer buffer;
	std::string name;
public:
	Material() = default;
	virtual ~Material() = default;

	T& GetInfoPtr() { return info; }
	ConstantBuffer& GetBufferPtr() { return buffer; }
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const { return buffer.GetGPUAddress(); }
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(int _frameIndex) const { return buffer.GetGPUAddress(_frameIndex); }

	DescriptorHandle GetHandle() { return buffer.GetHandle(); }
	void Update() { buffer.Update((void*)&info, sizeof(T)); }
	void Update(CommandList* _cmdList, int _frameIndex) { buffer.Update(_cmdList, (void*)&info, sizeof(T), _frameIndex); }
	
	void SetName(const std::string& _name) const { name = _name; }
	std::string GetName() const { return name; }
	int referenceCount = 0;
	

	virtual void Save(const std::string& _fileDirectory, const std::string& _name, const Texture2DCache& _textureCache, bool _debugASCII = false) const = 0;
	// Nytt med rmanager inp
	virtual void Load(ID3D12Device* _device, CommandList& _cmdList, ResourceManager& _rManager, const std::string& _fileDirectory, const std::string& _name, Texture2DCache& _textureCache) = 0;
};