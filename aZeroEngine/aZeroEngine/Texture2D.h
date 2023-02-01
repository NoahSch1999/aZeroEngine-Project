#pragma once
#include "BaseResource.h"
#include "ShaderDescriptorHeap.h"
#include "CommandList.h"
#include "HelperFunctions.h"

/** @brief Encapsulates a 2D texture resource.
*/
class Texture2D : public BaseResource
{
private:
	std::string fileName;
	DXGI_FORMAT format;
	ID3D12Resource* uploadBuffer;
	void InitSRV(ID3D12Device* _device, D3D12_RESOURCE_DESC _desc);

public:
	Texture2D();

	~Texture2D();

	/**Loads and initiates the texture WITH a descriptor handle.
	@param _device Device to use when creating the D3D12 resources.
	@param _cmdList CommandList to execute the resource initiation commands on.
	@param _handle Handle to copy to the internal descriptor handle variable.
	@param _path Directory to the texture to load.
	@param _name File name of the texture to load.
	@param _state State of the texture once the resource is initialized.
	@param _format Format of the texture to load.
	@return void
	*/
	void Init(ID3D12Device* _device, CommandList* _cmdList, DescriptorHandle _handle, const std::string& _path, const std::string& _name,
		D3D12_RESOURCE_STATES _state, DXGI_FORMAT _format);

	/**Loads and initiates the texture WITHOUT a descriptor handle.
	@param _device Device to use when creating the D3D12 resources.
	@param _cmdList CommandList to execute the resource initiation commands on.
	@param _path Directory to the texture to load.
	@param _name File name of the texture to load.
	@param _state State of the texture once the resource is initialized.
	@param _format Format of the texture to load.
	@return void
	*/
	void Init(ID3D12Device* _device, CommandList* _cmdList, const std::string& _path, const std::string& _name,
		D3D12_RESOURCE_STATES _state, DXGI_FORMAT _format);

	/**Returns a constant pointer to the name of the texture file.
	@return const char*
	*/
	const char* GetFileName() const { return fileName.c_str(); }

	/**Sets the internal name of the texture file.
	@return void
	*/
	void SetFileName(const char* _fileName) { fileName.assign(_fileName); }
};