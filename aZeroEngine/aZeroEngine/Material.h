#pragma once
#include "Texture2DCache.h"
#include "ResourceEngine.h"
#include "DescriptorManager.h"

/** @brief The base class of all the materials within the engine.
* It contains the CPU-side data, the ConstantBuffer, and its name.
*/
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

	/** Returns a reference to the CPU-side data member variable.
	@return T&
	*/
	T& GetInfoPtr() { return info; }

	/** Returns a reference to the ConstantBuffer member variable.
	@return ConstantBuffer&
	*/
	ConstantBuffer& GetBuffer() { return buffer; }

	/** Returns the D3D12_GPU_VIRTUAL_ADDRESS of the ConstantBuffer
	@return D3D12_GPU_VIRTUAL_ADDRESS
	*/
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const { return buffer.GetGPUAddress(); }

	/** OBSOLETE!?
	*/
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(int _frameIndex) const { return buffer.GetGPUAddress(_frameIndex); }

	/** Returns the DescriptorHandle of the ConstantBuffer
	@return DescriptorHandle
	*/
	DescriptorHandle GetHandle() const { return buffer.GetHandle(); }

	/** Records an update for the T CPU-side data to the GPU using ResourceEngine::Update().
	@param _resourceEngine The ResourceEngine to use for the update.
	@param _frameIndex The current frame index.
	@return void
	*/
	void Update(ResourceEngine& _resourceEngine) { _resourceEngine.Update(buffer, (void*)&info); }
	
	/** Sets the name of the Material.
	@param _name The new name of the Material.
	@return void
	*/
	void SetName(const std::string& _name) { name = _name; }

	/** Returns the name of the Material.
	@return std::string
	*/
	std::string GetName() const { return name; }
	
	/** The pure virtual method that should be overwritten by a subclass.
	* This method is used to save the Material to disc.
	@param _fileDirectory The directory to save the Material in.
	@param _textureCache The texture cache to get the neccessary Texture2D resources from.
	@return void
	*/
	virtual void Save(const std::string& _fileDirectory, const Texture2DCache& _textureCache) const = 0;
	
	/** The pure virtual method that should be overwritten by a subclass.
	* This method is used to load the Material from disc.
	@param _resourceEngine The ResourceEngine to record the neccessary commands on.
	@param _dManager OBSOLETE!
	@param _name The name of the Material to load.
	@param _textureCache The Texture2DCache which contains and will contain all the neccessary Texture2D objects for the Material.
	@return void
	*/
	virtual void Load(ResourceEngine& _resourceEngine, const std::string& _fileDirectory, const std::string& _name, Texture2DCache& _textureCache) = 0;
};
