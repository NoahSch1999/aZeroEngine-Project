#pragma once
#include "Texture2DCache.h"
#include "ResourceEngine.h"
#include "DescriptorManager.h"

/** @brief The base class of all the materials within the engine.
* It contains the CPU-side data and its name.
*/
template<typename T>
class Material
{
protected:
	T info;
	std::string name;

public:
	Material() = default;
	virtual ~Material() = default;

	/** Returns a reference to the CPU-side data member variable.
	@return T&
	*/
	T& GetInfo() { return info; }
	
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
	@param _fileDirectory The directory to save the Material in.
	@param _name The name of the Material to load.
	@param _textureCache The Texture2DCache which contains and will contain all the neccessary Texture2D objects for the Material.
	@return void
	*/
	virtual void Load(ResourceEngine& _resourceEngine, const std::string& _fileDirectory, const std::string& _name, Texture2DCache& _textureCache) = 0;
};
