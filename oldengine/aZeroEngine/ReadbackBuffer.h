#pragma once
#include "ResourceTrashcan.h"
#include "HelperFunctions.h"

class ReadbackBuffer
{
private:
	ResourceTrashcan* m_trashCan = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_resource = nullptr;
	void* m_mappedPtr = nullptr;

public:
	ReadbackBuffer() = default;
	ReadbackBuffer(const ReadbackBuffer&) = delete;
	ReadbackBuffer& operator=(const ReadbackBuffer&) = delete;

	ReadbackBuffer(ID3D12Device* device, ResourceTrashcan& trashCan, UINT rowPitch, UINT height)
		:m_trashCan(&trashCan)
	{
		m_resource = Helper::CreateReadbackBuffer(device, rowPitch, height);
		m_resource->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedPtr));
	}

	~ReadbackBuffer()
	{
		if (m_resource)
		{
			m_resource->Unmap(0, nullptr);
			m_trashCan->resources.push_back(m_resource);
			m_resource = nullptr;
		}
	}

	ReadbackBuffer(ReadbackBuffer&& other) noexcept
	{
		m_resource = other.m_resource;
		m_mappedPtr = other.m_mappedPtr;
		m_trashCan = other.m_trashCan;

		other.m_resource = nullptr;
		other.m_mappedPtr = nullptr;
		other.m_trashCan = nullptr;
	}

	ReadbackBuffer& operator=(ReadbackBuffer&& other) noexcept
	{
		if (this != &other)
		{
			m_resource = other.m_resource;
			m_mappedPtr = other.m_mappedPtr;
			m_trashCan = other.m_trashCan;

			other.m_resource = nullptr;
			other.m_mappedPtr = nullptr;
			other.m_trashCan = nullptr;
		}

		return *this;
	}

	ID3D12Resource* getResource() const { return m_resource.Get(); }
	void* getMappedPointer() const { return m_mappedPtr; }
};