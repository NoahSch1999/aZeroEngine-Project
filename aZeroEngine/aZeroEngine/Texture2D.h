#pragma once
#include "TextureResource.h"

class BaseResource;
class ShaderDescriptorHeap;

/** @brief Encapsulates a 2D texture resource.
*/
class Texture2D : public TextureResource
{
private:
	std::string name = "";

public:
	Texture2D() = default;

	~Texture2D() = default;

	/** Initiates the Texture2D object with the input data.
	@param _device ID3D12Device to use when creating the neccessary resources.
	@param _transitionList CommandList used to record the state transition to D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE.
	@param _copyList CommandList used to record the command to upload the input data to the GPU resource.
	@param _uploadResource Upload resource which will be used to copy the input data from.
	@param _srvHandle DescriptorHandle which will become a handle for a SRV.
	@param _data Initial data for the Textur2D.
	@param _width Width of the Texture2D.
	@param _height Height of the Texture2D.
	@param _bytePerTexel Number of bytes per pixel of the Texture2D.
	@param _format DXGI_FORMAT of the Texture2D GPU resource.
	@param _name Name of the Texture2D.
	@return void
	*/
	void Init(ID3D12Device* _device, CommandList& _transitionList, CommandList& _copyList, Microsoft::WRL::ComPtr<ID3D12Resource>& _uploadResource,
		DescriptorHandle _srvHandle, void* _data, UINT _width, UINT _height, UINT _bytePerTexel, DXGI_FORMAT _format, const std::string& _name);

	/**Returns a std::string to the name of the Texture2D.
	@return std::string
	*/
	std::string GetName() const { return name; }

	/**Sets the internal name of the Texture2D.
	@param _name The new name of the Texture2D.
	@return void
	*/
	void SetName(const std::string& _name) { name = _name; }
};