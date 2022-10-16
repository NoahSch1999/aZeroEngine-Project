#include "ShaderResource.h"
#include "stb_image.h"

ShaderResource::ShaderResource()
	:BaseResource()
{
}

ShaderResource::~ShaderResource()
{
	uploadBuffer->Release();
}

void ShaderResource::InitAsTextureImplicit(ID3D12Device* _device, ShaderDescriptorHeap* _heap, CommandList* _cmdList, const std::string& _path, 
	D3D12_RESOURCE_STATES _state, const std::wstring& _resourceName)
{
	// Since it will transition from this state to the other, this has to be set here (BaseResource constructor initially sets D3D12_RESOURCE_STATE_COMMON)
	state = D3D12_RESOURCE_STATE_COPY_DEST;

	// Load texture data
	const char* filePath = _path.c_str();
	int width, height, channels;
	unsigned char* image = stbi_load(filePath, &width, &height, nullptr, STBI_rgb_alpha);

	D3D12_RESOURCE_DESC rDesc = {};
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

	D3D12_HEAP_PROPERTIES props;
	props.Type = D3D12_HEAP_TYPE_DEFAULT;							// The type of the implicit heap that will be created
	props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;		
	props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;			// What type of memory pool the heap will be placed in. L0, L1 or UNKNOWN
	props.CreationNodeMask = 0;										
	props.VisibleNodeMask = 0;

	// Resource created in D3D12_RESOURCE_STATE_COPY_DEST state since it will be written to using the UpdateSubresources function
	HRESULT hr = _device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &rDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&resource));
	if (FAILED(hr))
		throw;

	// Getting the size in bytes of the heap that the textures uses (CAN BE USED TO GET SUBRESOURCE FOOTPRINT FOR GIGACHAD non-D3DX12 UpdateSubresources() function call)
	//UINT64 uploadHeapSize;
	//_device->GetCopyableFootprints(&rDesc, 0, 1, 0, nullptr, nullptr, nullptr, &uploadHeapSize);

	// Specify that the implicit heap should be a upload type
	props.Type = D3D12_HEAP_TYPE_UPLOAD;

	// Get the size of the resource in bytes
	D3D12_RESOURCE_ALLOCATION_INFO allocInfo = _device->GetResourceAllocationInfo(0, 1, &rDesc);

	// Using normie helper function to create the resource desc...
	CD3DX12_RESOURCE_DESC uDesc = CD3DX12_RESOURCE_DESC::Buffer(allocInfo.SizeInBytes);

	// Resource created in D3D12_RESOURCE_STATE_GENERIC_READ state since it will be read from in order to copy its contents from it
	hr = _device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &uDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));
	if (FAILED(hr))
		throw;

	// Define the subresource of the resource that we want to copy to
	D3D12_SUBRESOURCE_DATA sData = {};
	sData.pData = image;
	sData.RowPitch = width * STBI_rgb_alpha;
	sData.SlicePitch = sData.RowPitch * height;
	
	// Actually update the subresource of the resource. Copy from "uploadbuffer" to "resource"
	UpdateSubresources(_cmdList->graphic, resource, uploadBuffer, 0, 0, 1, &sData);

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

// not up to date
void ShaderResource::InitAsTextureImplicit(ID3D12Device* _device, HiddenDescriptorHeap* _heap, CommandList* _cmdList, const std::string& _path, D3D12_RESOURCE_STATES _state, const std::wstring& _resourceName)
{
	// Since it will transition from this state to the other, this has to be set here (BaseResource constructor initially sets D3D12_RESOURCE_STATE_COMMON)
	state = D3D12_RESOURCE_STATE_COPY_DEST;

	// Load texture data
	const char* filePath = _path.c_str();
	int width, height, channels;
	unsigned char* image = stbi_load(filePath, &width, &height, &channels, STBI_rgb_alpha);

	D3D12_RESOURCE_DESC rDesc = {};
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

	D3D12_HEAP_PROPERTIES props;
	props.Type = D3D12_HEAP_TYPE_DEFAULT;							// The type of the implicit heap that will be created
	props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;			// What type of memory pool the heap will be placed in. L0, L1 or UNKNOWN
	props.CreationNodeMask = 0;
	props.VisibleNodeMask = 0;

	// Resource created in D3D12_RESOURCE_STATE_COPY_DEST state since it will be written to using the UpdateSubresources function
	HRESULT hr = _device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &rDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&resource));
	if (FAILED(hr))
		throw;

	// Getting the size in bytes of the heap that the textures uses (CAN BE USED TO GET SUBRESOURCE FOOTPRINT FOR GIGACHAD non-D3DX12 UpdateSubresources() function call)
	//UINT64 uploadHeapSize;
	//_device->GetCopyableFootprints(&rDesc, 0, 1, 0, nullptr, nullptr, nullptr, &uploadHeapSize);

	// Specify that the implicit heap should be a upload type
	props.Type = D3D12_HEAP_TYPE_UPLOAD;

	// Get the size of the resource in bytes
	D3D12_RESOURCE_ALLOCATION_INFO allocInfo = _device->GetResourceAllocationInfo(0, 1, &rDesc);

	// Using normie helper function to create the resource desc...
	CD3DX12_RESOURCE_DESC uDesc = CD3DX12_RESOURCE_DESC::Buffer(allocInfo.SizeInBytes);

	// Resource created in D3D12_RESOURCE_STATE_GENERIC_READ state since it will be read from in order to copy its contents from it
	hr = _device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &uDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));
	if (FAILED(hr))
		throw;

	// Define the subresource of the resource that we want to copy to
	D3D12_SUBRESOURCE_DATA sData = {};
	sData.pData = image;
	sData.RowPitch = width * 4;
	sData.SlicePitch = width * 4 * height;

	// Actually update the subresource of the resource. Copy from "uploadbuffer" to "resource"
	UpdateSubresources(_cmdList->graphic, resource, uploadBuffer, 0, 0, 1, &sData);

	// Define the SRV desc for the creation of the SRV
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;


	// Get an empty handle from the descriptor heap
	handle = _heap->GetNewDescriptorHandle(1);

	_device->CreateShaderResourceView(resource, &srvDesc, handle.cpuHandle);

	// Change usage state from current state to D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE to enable usage within the pixel shader
	Transition(_cmdList->graphic, _state);

	// Set resource name for debugging purposes
	resource->SetName(_resourceName.c_str());
}
