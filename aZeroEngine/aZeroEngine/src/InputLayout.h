#pragma once
#include "D3D12Include.h"

class InputLayout
{
private:
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_descs;

public:
	InputLayout() = default;

	void addElement(const char* semantic, DXGI_FORMAT format, 
		D3D12_INPUT_CLASSIFICATION inputClassification = D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		UINT instanceStep = 0, UINT semanticIndex = 0, UINT inputAssemblerSlot = 0)
	{
		D3D12_INPUT_ELEMENT_DESC desc;
		desc.SemanticName = semantic;
		desc.Format = format;
		desc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		desc.SemanticIndex = semanticIndex;
		desc.InputSlot = inputAssemblerSlot;
		desc.InputSlotClass = inputClassification;
		desc.InstanceDataStepRate = instanceStep;
		m_descs.push_back(desc);
	}

	const D3D12_INPUT_ELEMENT_DESC* getDescription() const { return m_descs.data(); }

	UINT getNumElements() const { return m_descs.size(); }
};