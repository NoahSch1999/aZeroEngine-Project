#pragma once
#include "D3D12Include.h"

struct InputLayout
{
	D3D12_INPUT_ELEMENT_DESC m_descs[4];
	constexpr InputLayout()
	{
		m_descs[0].SemanticName = "POSITION";
		m_descs[0].SemanticIndex = 0;
		m_descs[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		m_descs[0].InputSlot = 0;
		m_descs[0].AlignedByteOffset = 0;
		m_descs[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		m_descs[0].InstanceDataStepRate = 0;

		m_descs[1].SemanticName = "UV";
		m_descs[1].SemanticIndex = 0;
		m_descs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		m_descs[1].InputSlot = 0;
		m_descs[1].AlignedByteOffset = 12;
		m_descs[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		m_descs[1].InstanceDataStepRate = 0;

		m_descs[2].SemanticName = "NORMAL";
		m_descs[2].SemanticIndex = 0;
		m_descs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		m_descs[2].InputSlot = 0;
		m_descs[2].AlignedByteOffset = 20;
		m_descs[2].InputSlotClass = D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		m_descs[2].InstanceDataStepRate = 0;

		m_descs[3].SemanticName = "TANGENT";
		m_descs[3].SemanticIndex = 0;
		m_descs[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		m_descs[3].InputSlot = 0;
		m_descs[3].AlignedByteOffset = 32;
		m_descs[3].InputSlotClass = D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		m_descs[3].InstanceDataStepRate = 0;
	}
};