#include "Texture.h"
#include "HelperFunctions.h"
#include <optional>

void Texture::createDescriptors(ID3D12Device* device)
{
	if (checkBits(m_description.m_usage, SRV))
	{
		if(m_srvHandle.getHeapIndex() == -1)
			m_srvHandle = m_descriptorManager->getResourceDescriptor();
		Helper::createSRVHandle(device, m_gpuResource, m_srvHandle.getCPUHandle(),
			m_description.m_srvFormat, m_description.m_sampleCount > 1, m_description.m_mipLevels);
	}
	else
	{
		if (m_srvHandle.getHeapIndex() != -1)
			m_descriptorManager->freeResourceDescriptor(m_srvHandle);
	}

	if (checkBits(m_description.m_usage, UAV))
	{
		if (m_uavHandle.getHeapIndex() == -1)
			m_uavHandle = m_descriptorManager->getResourceDescriptor();
		Helper::createUAVHandle(device, m_gpuResource, m_uavHandle.getCPUHandle(),
			m_description.m_uavFormat, m_description.m_sampleCount > 1);
	}
	else
	{
		if (m_uavHandle.getHeapIndex() != -1)
			m_descriptorManager->freeResourceDescriptor(m_uavHandle);
	}

	if (checkBits(m_description.m_usage, DSV))
	{
		if (m_dsvHandle.getHeapIndex() == -1)
			m_dsvHandle = m_descriptorManager->getDSVDescriptor();
		Helper::CreateDSVHandle(device, m_gpuResource, m_dsvHandle.getCPUHandle(),
			m_description.m_mainFormat, m_description.m_sampleCount > 1);
	}
	else
	{
		if (m_dsvHandle.getHeapIndex() != -1)
			m_descriptorManager->freeDSVDescriptor(m_dsvHandle);
	}
	
	if (checkBits(m_description.m_usage, RTV))
	{
		if (m_rtvHandle.getHeapIndex() == -1)
			m_rtvHandle = m_descriptorManager->getRTVDescriptor();
		Helper::CreateRTVHandle(device, m_gpuResource, m_rtvHandle.getCPUHandle(),
			m_description.m_mainFormat, m_description.m_sampleCount > 1);
	}
	else
	{
		if (m_rtvHandle.getHeapIndex() != -1)
			m_descriptorManager->freeRTVDescriptor(m_rtvHandle);
	}
}

bool Texture::checkBits(TEXTUREUSAGE bits, TEXTUREUSAGE targetBits)
{
	return (bits & targetBits) == targetBits;
}

void Texture::createResource(ID3D12Device* device, D3D12_CLEAR_VALUE* clearValue, D3D12_RESOURCE_FLAGS resourceFlags)
{
	D3D12_RESOURCE_DESC rDesc = {};
	rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rDesc.Width = m_description.m_width;
	rDesc.Height = m_description.m_height;
	rDesc.DepthOrArraySize = 1;
	rDesc.MipLevels = m_description.m_mipLevels;
	rDesc.Format = m_description.m_mainFormat;
	rDesc.SampleDesc.Count = m_description.m_sampleCount;
	rDesc.SampleDesc.Quality = m_description.m_sampleQuality;
	rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rDesc.Flags = resourceFlags;

	D3D12_HEAP_PROPERTIES properties = {};
	properties.Type = D3D12_HEAP_TYPE_DEFAULT;

	HRESULT hr = device->CreateCommittedResource(&properties, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&rDesc, m_description.m_initialState, clearValue, IID_PPV_ARGS(&m_gpuResource));
	if (FAILED(hr))
		throw;

	m_gpuResourceState = m_description.m_initialState;
}

//void Texture::init(ID3D12Device* device, std::optional<D3D12_CLEAR_VALUE> clearColor)
//{
//	if (clearColor.has_value())
//	{
//		memcpy(&m_clearValue, (void*)&clearColor.value(), sizeof(D3D12_CLEAR_VALUE));
//	}
//	else
//	{
//		if (checkBits(m_specification.m_usage, TEXTUREUSAGE::DSV))
//		{
//			m_clearValue.Format = m_specification.m_dsvFormat;
//			m_clearValue.DepthStencil.Depth = 1;
//			m_clearValue.DepthStencil.Stencil = 0;
//		}
//		else if (checkBits(m_specification.m_usage, TEXTUREUSAGE::RTV))
//		{
//			m_clearValue.Format = m_specification.m_uavSrvRtvFormat;
//			m_clearValue.Color[0] = 0.f;
//			m_clearValue.Color[1] = 0.f;
//			m_clearValue.Color[2] = 0.f;
//			m_clearValue.Color[3] = 0.f;
//		}
//	}
//
//	if (checkBits(m_specification.m_usage, TEXTUREUSAGE::SRV))
//	{
//		m_srvHandle = m_descriptorManager->getResourceDescriptor();
//	}
//
//	if (checkBits(m_specification.m_usage, TEXTUREUSAGE::UAV))
//	{
//		m_resourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
//		m_uavHandle = m_descriptorManager->getResourceDescriptor();
//	}
//
//	if (checkBits(m_specification.m_usage, TEXTUREUSAGE::DSV))
//	{
//		m_resourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
//		m_dsvHandle = m_descriptorManager->getDSVDescriptor();
//	}
//	else if (checkBits(m_specification.m_usage, TEXTUREUSAGE::RTV))
//	{
//		m_resourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
//		m_rtvHandle = m_descriptorManager->getRTVDescriptor();
//	}
//
//	if (checkBits(m_specification.m_usage, TEXTUREUSAGE::SIMACCESS))
//	{
//		m_resourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
//	}
//
//	this->createResource(device);
//	this->createDescriptors(device);
//}

void Texture::moveData(Texture& target, Texture&& other)
{
	m_gpuResource = other.m_gpuResource;
	m_gpuResourceState = other.m_gpuResourceState;
	m_descriptorManager = other.m_descriptorManager;
	m_recycler = other.m_recycler;

	m_srvHandle = other.m_srvHandle;
	m_uavHandle = other.m_uavHandle;
	m_rtvHandle = other.m_rtvHandle;
	m_dsvHandle = other.m_dsvHandle;
	
	m_description = other.m_description;

	other.m_gpuResource = nullptr;
	other.m_description.m_usage = TEXTUREUSAGE::NONE;
}

void Texture::init(ID3D12Device* device, const Description& description)
{
	m_description = description;

	D3D12_CLEAR_VALUE* clearValue = nullptr;
	D3D12_RESOURCE_FLAGS resourceFlags = D3D12_RESOURCE_FLAG_NONE;

	if (checkBits(m_description.m_usage, TEXTUREUSAGE::DSV))
	{
		clearValue = &m_description.m_clearValue;
		resourceFlags |= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	}
	else if (checkBits(m_description.m_usage, TEXTUREUSAGE::RTV))
	{
		clearValue = &m_description.m_clearValue;
		resourceFlags |= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	}

	// Setup usage
	if (checkBits(m_description.m_usage, SIMACCESS))
		resourceFlags |= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;

	if (checkBits(m_description.m_usage, UAV))
		resourceFlags |= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	// Create main resource
	this->createResource(device, clearValue, resourceFlags);

	// Create descriptors
	this->createDescriptors(device);
}

Texture::Texture(ID3D12Device* device, DescriptorManager& descriptorManager, ResourceRecycler& recycler, const Description& description)
	:m_descriptorManager(&descriptorManager),
	m_recycler(&recycler)
{
	this->init(device, description);
}

Texture::~Texture()
{
	if (m_gpuResource)
	{
		if (m_srvHandle.getHeapIndex() != -1)
		{
			m_descriptorManager->freeResourceDescriptor(m_srvHandle);
		}

		if (m_uavHandle.getHeapIndex() != -1)
		{
			m_descriptorManager->freeResourceDescriptor(m_uavHandle);
		}

		if (checkBits(m_description.m_usage, TEXTUREUSAGE::RTV))
		{
			m_descriptorManager->freeRTVDescriptor(m_rtvHandle);
		}
		else if (checkBits(m_description.m_usage, TEXTUREUSAGE::DSV))
		{
			m_descriptorManager->freeDSVDescriptor(m_dsvHandle);
		}

		m_recycler->destroyResource(m_gpuResource);
	}
}

Texture::Texture(Texture&& other) noexcept
{
	this->moveData(*this, std::move(other));
}

Texture& Texture::operator=(Texture&& other) noexcept
{
	if (this != &other)
		this->moveData(*this, std::move(other));

	return *this;
}

void Texture::transition(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES newState)
{
	if (m_gpuResourceState != newState)
	{
		D3D12_RESOURCE_BARRIER barrier(CD3DX12_RESOURCE_BARRIER::Transition(m_gpuResource.Get(), m_gpuResourceState, newState));
		commandList->ResourceBarrier(1, &barrier);
		m_gpuResourceState = newState;
	}
}

void Texture::recreate(ID3D12Device* device, const Description& description)
{
	this->init(device, description);
}
