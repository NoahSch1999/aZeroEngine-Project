#include "Texture.h"
#include "HelperFunctions.h"

Texture::Texture(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const TextureSettings& settings,
	DescriptorManager& descriptorManager, ResourceTrashcan& trashcan)
	:m_settings(settings), m_descriptorManager(&descriptorManager), m_trashcan(&trashcan)
{
	m_rowPitch = ((m_settings.m_width * m_settings.m_bytesPerTexel + 128) / 256) * 256;

	if (m_settings.m_flags == D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
	{
		m_gpuOnlyResource = Helper::CreateTextureResource(device, m_settings.m_width, m_settings.m_height,
			m_settings.m_dsvFormat, m_settings.m_flags, m_settings.m_initialState, &m_settings.m_clearValue);
	}
	else if (m_settings.m_flags == D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
	{
		m_gpuOnlyResource = Helper::CreateTextureResource(device, m_settings.m_width, m_settings.m_height,
			m_settings.m_rtvFormat, m_settings.m_flags, m_settings.m_initialState, &m_settings.m_clearValue);
	}
	else if (m_settings.m_flags == (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS))
	{
		m_gpuOnlyResource = Helper::CreateTextureResource(device, m_settings.m_width, m_settings.m_height,
			m_settings.m_rtvFormat, m_settings.m_flags, m_settings.m_initialState, &m_settings.m_clearValue);
	}
	else if (m_settings.m_flags == D3D12_RESOURCE_FLAG_NONE)
	{
		m_gpuOnlyResource = Helper::CreateTextureResource(device, m_settings.m_width, m_settings.m_height,
			m_settings.m_srvFormat, m_settings.m_flags, D3D12_RESOURCE_STATE_COPY_DEST, nullptr);
	}
	else
		throw;

	if (m_settings.m_srvFormat != -1)
	{
		m_handleSRV = m_descriptorManager->getResourceDescriptor();
		Helper::CreateSRVHandle(device, m_gpuOnlyResource, m_handleSRV.getCPUHandle(), m_settings.m_srvFormat);
	}

	if (m_settings.m_rtvFormat != -1)
	{
		m_handleRTVDSV = m_descriptorManager->getRTVDescriptor();
		Helper::CreateRTVHandle(device, m_gpuOnlyResource, m_handleRTVDSV.getCPUHandle(), m_settings.m_rtvFormat);
	}
	else if (m_settings.m_dsvFormat != -1)
	{
		m_handleRTVDSV = m_descriptorManager->getDSVDescriptor();
		Helper::CreateDSVHandle(device, m_gpuOnlyResource, m_handleRTVDSV.getCPUHandle(), m_settings.m_dsvFormat);
	}

	if (m_settings.m_uploadSettings.m_initialData)
	{
		m_uploadResource = Helper::CreateUploadResource(device, m_settings.m_width, m_settings.m_height, m_settings.m_srvFormat, 
			m_settings.m_flags, D3D12_RESOURCE_STATE_COPY_SOURCE);
		m_uploadResourceState = D3D12_RESOURCE_STATE_COPY_SOURCE;

		D3D12_SUBRESOURCE_DATA sData = {};
		sData.pData = m_settings.m_uploadSettings.m_initialData;
		sData.RowPitch = m_settings.m_width * m_settings.m_bytesPerTexel;
		sData.SlicePitch = m_settings.m_width * m_settings.m_bytesPerTexel * m_settings.m_height;

		UpdateSubresources(commandList, m_gpuOnlyResource.Get(), m_uploadResource.Get(), 0, 0, 1, &sData);

		if (m_settings.m_uploadSettings.m_discardUpload)
		{
			m_trashcan->resources.push_back(m_uploadResource);
			m_uploadResource = nullptr;
		}
		else
		{
			m_uploadResource->Map(0, NULL, reinterpret_cast<void**>(&m_mappedBuffer));
		}
	}

	if (m_settings.m_flags == D3D12_RESOURCE_FLAG_NONE)
	{
		m_gpuOnlyResourceState = D3D12_RESOURCE_STATE_COPY_DEST;
		transition(commandList, m_settings.m_initialState);
	}
	else
	{
		m_gpuOnlyResourceState = m_settings.m_initialState;
	}
}

Texture::~Texture()
{
	if (m_gpuOnlyResource)
	{
		if (m_handleSRV.getHeapIndex() != -1)
			m_descriptorManager->freeResourceDescriptor(m_handleSRV);

		if (m_settings.m_rtvFormat != -1)
		{
			m_descriptorManager->freeRTVDescriptor(m_handleRTVDSV);
		}
		else if (m_settings.m_dsvFormat != -1)
		{
			m_descriptorManager->freeDSVDescriptor(m_handleRTVDSV);
		}

		if (m_handleUAV.getHeapIndex() != -1)
		{
			m_descriptorManager->freeResourceDescriptor(m_handleUAV);
		}

		m_trashcan->resources.push_back(m_gpuOnlyResource);
	}

	if (m_uploadResource)
	{
		m_uploadResource->Unmap(0, NULL);
		m_trashcan->resources.push_back(m_uploadResource);
	}
}

Texture::Texture(Texture&& other) noexcept
{
	m_gpuOnlyResource = other.m_gpuOnlyResource;
	m_uploadResource = other.m_uploadResource;
	m_handleSRV = other.m_handleSRV;
	m_handleRTVDSV = other.m_handleRTVDSV;
	m_handleUAV = other.m_handleUAV;
	m_descriptorManager = other.m_descriptorManager;
	m_gpuOnlyResourceState = other.m_gpuOnlyResourceState;
	m_uploadResourceState = other.m_uploadResourceState;
	m_mappedBuffer = other.m_mappedBuffer;
	m_trashcan = other.m_trashcan;
	m_settings = other.m_settings;
	m_rowPitch = other.m_rowPitch;

	other.m_gpuOnlyResource = nullptr;
	other.m_uploadResource = nullptr;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
	if (this != &other)
	{
		if (m_gpuOnlyResource)
		{
			if (m_handleSRV.getHeapIndex() != -1)
				m_descriptorManager->freeResourceDescriptor(m_handleSRV);

			if (m_settings.m_rtvFormat != -1)
			{
				m_descriptorManager->freeRTVDescriptor(m_handleRTVDSV);
			}
			else if (m_settings.m_dsvFormat != -1)
			{
				m_descriptorManager->freeDSVDescriptor(m_handleRTVDSV);
			}
			m_trashcan->resources.push_back(m_gpuOnlyResource);
		}

		if (m_uploadResource)
		{
			m_uploadResource->Unmap(0, NULL);
			m_trashcan->resources.push_back(m_uploadResource);
		}

		m_gpuOnlyResource = other.m_gpuOnlyResource;
		m_uploadResource = other.m_uploadResource;
		m_handleSRV = other.m_handleSRV;
		m_handleRTVDSV = other.m_handleRTVDSV;
		m_handleUAV = other.m_handleUAV;
		m_descriptorManager = other.m_descriptorManager;
		m_gpuOnlyResourceState = other.m_gpuOnlyResourceState;
		m_uploadResourceState = other.m_uploadResourceState;
		m_mappedBuffer = other.m_mappedBuffer;
		m_trashcan = other.m_trashcan;
		m_settings = other.m_settings;
		m_rowPitch = other.m_rowPitch;

		other.m_gpuOnlyResource = nullptr;
		other.m_uploadResource = nullptr;
	}

	return *this;
}

void Texture::transition(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES newState)
{
	D3D12_RESOURCE_BARRIER barrier(CD3DX12_RESOURCE_BARRIER::Transition(m_gpuOnlyResource.Get(), m_gpuOnlyResourceState, newState));
	commandList->ResourceBarrier(1, &barrier);
	m_gpuOnlyResourceState = newState;
}

void Texture::initUAV(ID3D12Device* device, DXGI_FORMAT format)
{
	m_handleUAV = m_descriptorManager->getResourceDescriptor();
	Helper::createUAVHandle(device, m_gpuOnlyResource.Get(), m_handleUAV.getCPUHandle(), format);
}
