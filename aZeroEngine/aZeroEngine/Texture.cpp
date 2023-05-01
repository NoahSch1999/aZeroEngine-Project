#include "Texture.h"
#include "HelperFunctions.h"

Texture::Texture(ID3D12Device* _device, ID3D12GraphicsCommandList* _commandList, const TextureSettings& _settings,
	DescriptorManager& _descriptorManager, ResourceTrashcan& _trashcan)
	:settings(_settings), descriptorManager(&_descriptorManager), trashcan(&_trashcan)
{
	if (settings.flags == D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
	{
		gpuOnlyResource = Helper::CreateTextureResource(_device, settings.width, settings.height,
			settings.dsvFormat, settings.flags, settings.initialState, &settings.clearValue);
	}
	else if (settings.flags == D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
	{
		gpuOnlyResource = Helper::CreateTextureResource(_device, settings.width, settings.height,
			settings.rtvFormat, settings.flags, settings.initialState, &settings.clearValue);
	}
	else if (settings.flags == D3D12_RESOURCE_FLAG_NONE)
	{
		gpuOnlyResource = Helper::CreateTextureResource(_device, settings.width, settings.height,
			settings.srvFormat, settings.flags, D3D12_RESOURCE_STATE_COPY_DEST, nullptr);
	}
	else
		throw;

	if (settings.srvFormat != -1)
	{
		handleSRV = descriptorManager->GetResourceDescriptor();
		Helper::CreateSRVHandle(_device, gpuOnlyResource, handleSRV.GetCPUHandle(), settings.srvFormat);
	}

	if (settings.rtvFormat != -1)
	{
		handleRTVDSV = descriptorManager->GetRTVDescriptor();
		Helper::CreateRTVHandle(_device, gpuOnlyResource, handleRTVDSV.GetCPUHandle(), settings.rtvFormat);
	}
	else if (settings.dsvFormat != -1)
	{
		handleRTVDSV = descriptorManager->GetDSVDescriptor();
		Helper::CreateDSVHandle(_device, gpuOnlyResource, handleRTVDSV.GetCPUHandle(), settings.dsvFormat);
	}

	if (settings.uploadSettings.initialData)
	{
		uploadResource = Helper::CreateUploadResource(_device, settings.width, settings.height, settings.srvFormat, settings.flags, D3D12_RESOURCE_STATE_COPY_SOURCE);
		uploadResourceState = D3D12_RESOURCE_STATE_COPY_SOURCE;

		D3D12_SUBRESOURCE_DATA sData = {};
		sData.pData = settings.uploadSettings.initialData;
		sData.RowPitch = settings.width * settings.bytesPerTexel;
		sData.SlicePitch = settings.width * settings.bytesPerTexel * settings.height;

		UpdateSubresources(_commandList, gpuOnlyResource.Get(), uploadResource.Get(), 0, 0, 1, &sData);

		if (settings.uploadSettings.discardUpload)
		{
			trashcan->resources.push_back(uploadResource);
			uploadResource = nullptr;
		}
		else
		{
			uploadResource->Map(0, NULL, reinterpret_cast<void**>(&mappedBuffer));
		}
	}

	if (settings.createReadback)
	{
		rowPitch = ((settings.width * settings.bytesPerTexel + 128) / 256) * 256;
		readbackResource = Helper::CreateReadbackBuffer(_device, rowPitch, settings.height);
		readbackResource->Map(0, nullptr, reinterpret_cast<void**>(&readbackMappedPtr));
	}

	if (settings.flags == D3D12_RESOURCE_FLAG_NONE)
	{
		gpuOnlyResourceState = D3D12_RESOURCE_STATE_COPY_DEST;
		Transition(_commandList, settings.initialState);
	}
	else
	{
		gpuOnlyResourceState = settings.initialState;
	}
}

Texture::~Texture()
{
	if (gpuOnlyResource)
	{
		if (handleSRV.GetHeapIndex() != -1)
			descriptorManager->FreeResourceDescriptor(handleSRV);

		if (settings.rtvFormat != -1)
		{
			descriptorManager->FreeRTVDescriptor(handleRTVDSV);
		}
		else if (settings.dsvFormat != -1)
		{
			descriptorManager->FreeDSVDescriptor(handleRTVDSV);
		}
		trashcan->resources.push_back(gpuOnlyResource);
	}

	if (uploadResource)
	{
		uploadResource->Unmap(0, NULL);
		trashcan->resources.push_back(uploadResource);
	}

	if (readbackResource)
	{
		readbackResource->Unmap(0, NULL);
		trashcan->resources.push_back(readbackResource);
	}
}

Texture::Texture(Texture&& _other) noexcept
{
	gpuOnlyResource = _other.gpuOnlyResource;
	uploadResource = _other.uploadResource;
	readbackResource = _other.readbackResource;
	handleSRV = _other.handleSRV;
	handleRTVDSV = _other.handleRTVDSV;
	descriptorManager = _other.descriptorManager;
	gpuOnlyResourceState = _other.gpuOnlyResourceState;
	uploadResourceState = _other.uploadResourceState;
	mappedBuffer = _other.mappedBuffer;
	readbackMappedPtr = _other.readbackMappedPtr;
	trashcan = _other.trashcan;
	settings = _other.settings;
	rowPitch = _other.rowPitch;

	_other.gpuOnlyResource = nullptr;
	_other.uploadResource = nullptr;
	_other.readbackResource = nullptr;
}

Texture& Texture::operator=(Texture&& _other) noexcept
{
	if (this != &_other)
	{
		if (gpuOnlyResource)
		{
			if (handleSRV.GetHeapIndex() != -1)
				descriptorManager->FreeResourceDescriptor(handleSRV);

			if (settings.rtvFormat != -1)
			{
				descriptorManager->FreeRTVDescriptor(handleRTVDSV);
			}
			else if (settings.dsvFormat != -1)
			{
				descriptorManager->FreeDSVDescriptor(handleRTVDSV);
			}
			trashcan->resources.push_back(gpuOnlyResource);
		}

		if (uploadResource)
		{
			uploadResource->Unmap(0, NULL);
			trashcan->resources.push_back(uploadResource);
		}

		if (readbackResource)
		{
			readbackResource->Unmap(0, NULL);
			trashcan->resources.push_back(readbackResource);
		}

		gpuOnlyResource = _other.gpuOnlyResource;
		uploadResource = _other.uploadResource;
		readbackResource = _other.readbackResource;
		handleSRV = _other.handleSRV;
		handleRTVDSV = _other.handleRTVDSV;
		descriptorManager = _other.descriptorManager;
		gpuOnlyResourceState = _other.gpuOnlyResourceState;
		uploadResourceState = _other.uploadResourceState;
		mappedBuffer = _other.mappedBuffer;
		readbackMappedPtr = _other.readbackMappedPtr;
		trashcan = _other.trashcan;
		settings = _other.settings;
		rowPitch = _other.rowPitch;

		_other.gpuOnlyResource = nullptr;
		_other.uploadResource = nullptr;
		_other.readbackResource = nullptr;
	}

	return *this;
}

void Texture::Transition(ID3D12GraphicsCommandList* _commandList, D3D12_RESOURCE_STATES _newState)
{
	D3D12_RESOURCE_BARRIER barrier(CD3DX12_RESOURCE_BARRIER::Transition(gpuOnlyResource.Get(), gpuOnlyResourceState, _newState));
	_commandList->ResourceBarrier(1, &barrier);
	gpuOnlyResourceState = _newState;
}