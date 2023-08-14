#pragma once
#include "D3D12Include.h"
#include <vector>

class ResourceRecycler
{
private:
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_resources;

public:
	ResourceRecycler() = default;
	void destroyResource(Microsoft::WRL::ComPtr<ID3D12Resource> resource) { m_resources.push_back(resource); }
	void recycleResources() { m_resources.clear(); }
};