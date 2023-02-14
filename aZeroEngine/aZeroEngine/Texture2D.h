#pragma once
#include "BaseResource.h"
#include "ShaderDescriptorHeap.h"

class BaseResource;
class ShaderDescriptorHeap;

/** @brief Encapsulates a 2D texture resource.
*/
class Texture2D
{
private:
	std::string fileName;
	DXGI_FORMAT format;
	void InitSRV(ID3D12Device* _device, D3D12_RESOURCE_DESC _desc);
	ID3D12Resource* mainResource = nullptr;
	D3D12_RESOURCE_STATES mainResourceState = D3D12_RESOURCE_STATE_COMMON;
	/*ID3D12Resource* intermediateResource;
	D3D12_RESOURCE_STATES intermediateResourceState = D3D12_RESOURCE_STATE_COMMON;*/
	DescriptorHandle handle;
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
	void Init(ID3D12Device* _device, CommandList* _cmdList, ID3D12Resource*& _intermediateResource, DescriptorHandle _handle, const std::string& _path, const std::string& _name,
		D3D12_RESOURCE_STATES _state, DXGI_FORMAT _format);

	///**Loads and initiates the texture WITHOUT a descriptor handle.
	//@param _device Device to use when creating the D3D12 resources.
	//@param _cmdList CommandList to execute the resource initiation commands on.
	//@param _path Directory to the texture to load.
	//@param _name File name of the texture to load.
	//@param _state State of the texture once the resource is initialized.
	//@param _format Format of the texture to load.
	//@return void
	//*/
	//void Init(ID3D12Device* _device, CommandList* _cmdList, ID3D12Resource*& _intermediateResource, const std::string& _path, const std::string& _name,
	//	D3D12_RESOURCE_STATES _state, DXGI_FORMAT _format);

	/**Returns a constant pointer to the name of the texture file.
	@return const char*
	*/
	const char* GetFileName() const { return fileName.c_str(); }

	/**Sets the internal name of the texture file.
	@return void
	*/
	void SetFileName(const char* _fileName) { fileName.assign(_fileName); }

	DescriptorHandle GetHandle() { return handle; }

	/**Returns a pointer reference to the internal main ID3D12Resource object.
	@return ID3D12Resource*&
	*/
	ID3D12Resource*& GetMainResource() { return mainResource; }

	///**Returns a pointer reference to the internal intermediate ID3D12Resource object.
	//@return ID3D12Resource*&
	//*/
	//ID3D12Resource*& GetIntermediateResource() { return intermediateResource; }
};