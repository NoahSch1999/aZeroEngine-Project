#include "ShaderResource.h"
#include "stb_image.h"
#include "HelperFunctions.h"

ShaderResource::ShaderResource()
	:BaseResource()
{
}

ShaderResource::~ShaderResource()
{
	uploadBuffer->Release();
}

void ShaderResource::InitAsTextureAndDescriptor(ID3D12Device* _device, ShaderDescriptorHeap* _heap, CommandList* _cmdList, const std::string& _path,
	D3D12_RESOURCE_STATES _state, const std::wstring& _resourceName)
{
	// Since it will transition from this state to the other, this has to be set here (BaseResource constructor initially sets D3D12_RESOURCE_STATE_COMMON)
	state = D3D12_RESOURCE_STATE_COPY_DEST;

	// Load texture data
	const char* filePath = _path.c_str();
	int width, height, channels;
	unsigned char* image = stbi_load(filePath, &width, &height, nullptr, STBI_rgb_alpha);

	rDesc.MipLevels = 1;
	rDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;						// Texture format (1 byte per channel)
	rDesc.Width = width;
	rDesc.Height = height;
	rDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	rDesc.DepthOrArraySize = 1;
	rDesc.SampleDesc.Count = 1;										// Samples per pixel when sampling from the resource
	rDesc.SampleDesc.Quality = 0;
	rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;			// Dimension of the resource is a 2-Dimensional texture
	rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;					// Layout will be set automatically 
	rDesc.Alignment = 0;											// Alignment set at runtime. 64KB for normal textures and 4MB for multi-sampled textures.

		// Get the size of the resource in bytes
	D3D12_RESOURCE_ALLOCATION_INFO allocInfo = _device->GetResourceAllocationInfo(0, 1, &rDesc);

	// Using normie helper function to create the resource desc...
	CD3DX12_RESOURCE_DESC uDesc = CD3DX12_RESOURCE_DESC::Buffer(allocInfo.SizeInBytes);

	Helper::CreateCommitedResourceStatic(_device, resource, rDesc, uploadBuffer, uDesc, _cmdList, image, width * STBI_rgb_alpha, width * STBI_rgb_alpha * height);

	// Get an empty handle from the descriptor heap
	handle = _heap->GetNewDescriptorHandle(1);

	// Define the SRV desc for the creation of the SRV
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = rDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = rDesc.MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.f;

	// Specifies that the resource and the descriptor handle can be used as an SRV
	_device->CreateShaderResourceView(resource, &srvDesc, handle.cpuHandle);

	// Change usage state from current state to D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE to enable usage within the pixel shader
	Transition(_cmdList->graphic, _state);

	// Set resource name for debugging purposes
	resource->SetName(_resourceName.c_str());
}

void ShaderResource::InitAsTexture(ID3D12Device* _device, CommandList* _cmdList, const std::string& _path,
	D3D12_RESOURCE_STATES _state, const std::wstring& _resourceName)
{
	// Since it will transition from this state to the other, this has to be set here (BaseResource constructor initially sets D3D12_RESOURCE_STATE_COMMON)
	state = D3D12_RESOURCE_STATE_COPY_DEST;

	// Load texture data
	const char* filePath = _path.c_str();
	int width, height, channels;
	unsigned char* image = stbi_load(filePath, &width, &height, nullptr, STBI_rgb_alpha);

	rDesc.MipLevels = 1;
	rDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;						// Texture format (1 byte per channel)
	rDesc.Width = width;
	rDesc.Height = height;
	rDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	rDesc.DepthOrArraySize = 1;
	rDesc.SampleDesc.Count = 1;										// Samples per pixel when sampling from the resource
	rDesc.SampleDesc.Quality = 0;
	rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;			// Dimension of the resource is a 2-Dimensional texture
	rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;					// Layout will be set automatically 
	rDesc.Alignment = 0;											// Alignment set at runtime. 64KB for normal textures and 4MB for multi-sampled textures.

	// Get the size of the resource in bytes
	D3D12_RESOURCE_ALLOCATION_INFO allocInfo = _device->GetResourceAllocationInfo(0, 1, &rDesc);

	// Using normie helper function to create the resource desc...
	CD3DX12_RESOURCE_DESC uDesc = CD3DX12_RESOURCE_DESC::Buffer(allocInfo.SizeInBytes);

	Helper::CreateCommitedResourceStatic(_device, resource, rDesc, uploadBuffer, uDesc, _cmdList, image, width * STBI_rgb_alpha, width * STBI_rgb_alpha * height);

	Transition(_cmdList->graphic, _state);

	// Set resource name for debugging purposes
	std::wstring wsTemp(_path.begin(), _path.end());
	resource->SetName(wsTemp.c_str());
}

void ShaderResource::InitAsSRV(ID3D12Device* _device)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = rDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = rDesc.MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.f;

	// Specifies that the resource and the descriptor handle can be used as an SRV
	_device->CreateShaderResourceView(resource, &srvDesc, handle.cpuHandle);
}
