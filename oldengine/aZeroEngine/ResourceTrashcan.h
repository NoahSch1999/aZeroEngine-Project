#pragma once
#include "D3D12Include.h"
#include <vector>

struct ResourceTrashcan
{
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> resources;
	ResourceTrashcan() = default;
};