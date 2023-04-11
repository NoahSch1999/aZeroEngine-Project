#pragma once
#include "D3D12Include.h"

struct InputLayout
{
	D3D12_INPUT_ELEMENT_DESC descs[4];
	InputLayout()
	{
		descs[0].SemanticName = "POSITION";
		descs[0].SemanticIndex = 0;
		descs[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		descs[0].InputSlot = 0;
		descs[0].AlignedByteOffset = 0;
		descs[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		descs[0].InstanceDataStepRate = 0;

		descs[1].SemanticName = "UV";
		descs[1].SemanticIndex = 0;
		descs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		descs[1].InputSlot = 0;
		descs[1].AlignedByteOffset = 12;
		descs[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		descs[1].InstanceDataStepRate = 0;

		descs[2].SemanticName = "NORMAL";
		descs[2].SemanticIndex = 0;
		descs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		descs[2].InputSlot = 0;
		descs[2].AlignedByteOffset = 20;
		descs[2].InputSlotClass = D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		descs[2].InstanceDataStepRate = 0;

		descs[3].SemanticName = "TANGENT";
		descs[3].SemanticIndex = 0;
		descs[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		descs[3].InputSlot = 0;
		descs[3].AlignedByteOffset = 32;
		descs[3].InputSlotClass = D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		descs[3].InstanceDataStepRate = 0;
	}
};