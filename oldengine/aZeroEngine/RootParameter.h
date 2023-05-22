#pragma once
#include "D3D12Include.h"

/** @brief Encapsulates a Root Parameter list object.
*/
class RootParameters
{
private:
	std::vector<D3D12_ROOT_PARAMETER> m_parameters;
	D3D12_DESCRIPTOR_RANGE m_ranges[100] = {};
	int m_numParameters = 0;

public:
	RootParameters() = default;

	~RootParameters() = default;

	/** @brief Returns a pointer to the start of the parameter list.
	@return D3D12_ROOT_PARAMETER*
	*/
	D3D12_ROOT_PARAMETER* getParameterData() { return m_parameters.data(); }

	/** @brief Returns the number of parameters within the list.
	@return int
	*/
	int getParameterNum() { return static_cast<int>(m_parameters.size()); }

	/** @brief Adds a descriptor table with the specified arguments to the parameter list.
	@param rangeType D3D12_DESCRIPTOR_RANGE_TYPE of the descriptor table.
	@param baseShaderRegister Shader register start of the descriptor table.
	@param numDescriptors Number of descriptors in the descriptor table.
	@param shaderVisability Shader visibility of the descriptor table. Defaulted to D3D12_SHADER_VISIBILITY_ALL.
	@param offsetFromStart Offset of the range from the start of the range set as the root argument value for this descriptor table parameter slot. Defaulted to 0, ie. no offset.
	@param registerSpace The register space for the descriptor table parameter. Defaulted to 0.
	@return void
	*/
	void addDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT baseShaderRegister, 
		UINT numDescriptors = 1, D3D12_SHADER_VISIBILITY shaderVisability = D3D12_SHADER_VISIBILITY_ALL, UINT offsetFromStart = 0, UINT registerSpace = 0)
	{
		D3D12_DESCRIPTOR_RANGE range = {};
		range.BaseShaderRegister = baseShaderRegister;
		range.NumDescriptors = numDescriptors;
		range.OffsetInDescriptorsFromTableStart = offsetFromStart;
		range.RangeType = rangeType;
		range.RegisterSpace = registerSpace;

		m_ranges[m_numParameters] = range;
		D3D12_ROOT_DESCRIPTOR_TABLE table{ 1, &m_ranges[m_numParameters] };
		m_numParameters++;
		D3D12_ROOT_PARAMETER param{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, table, shaderVisability };
		m_parameters.emplace_back(param);
	}

	/** @brief Adds a root descriptor with the specified arguments to the parameter list.
	@param shaderRegister Shader register of the root descriptor.
	@param paramType Parameter type of the root descriptor.
	@param shaderVisability Shader visibility of the root descriptor. Defaulted to D3D12_SHADER_VISIBILITY_ALL.
	@param registerSpace The register space for the root descriptor parameter. Defaulted to 0.	
	@return void
	*/
	void addRootDescriptor(UINT shaderRegister, D3D12_ROOT_PARAMETER_TYPE paramType, 
		D3D12_SHADER_VISIBILITY shaderVisability = D3D12_SHADER_VISIBILITY_ALL, UINT registerSpace = 0)
	{
		const D3D12_ROOT_DESCRIPTOR descriptor{ shaderRegister, registerSpace };
		D3D12_ROOT_PARAMETER param;
		param.Descriptor = descriptor;
		param.ParameterType = paramType;
		param.ShaderVisibility = shaderVisability;
		m_parameters.emplace_back(param);
	}

	/** @brief Adds a root constant with the specified arguments to the parameter list.
	@param shaderRegister Shader register of the root constant.
	@param num32BitValues Number of 32bit (4 bytes) values that the root constant should contain.
	@param shaderVisability Shader visibility of the root descriptor. Defaulted to D3D12_SHADER_VISIBILITY_ALL.
	@param registerSpace The register space for the root descriptor parameter. Defaulted to 0.	
	@return void
	*/
	void addRootConstants(UINT shaderRegister, UINT num32BitValues, 
		D3D12_SHADER_VISIBILITY shaderVisability = D3D12_SHADER_VISIBILITY_ALL, UINT registerSpace = 0)
	{
		const D3D12_ROOT_CONSTANTS constants{ shaderRegister, registerSpace, num32BitValues };
		D3D12_ROOT_PARAMETER param;
		ZeroMemory(&param, sizeof(D3D12_ROOT_PARAMETER));
		param.Constants = constants;
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		param.ShaderVisibility = shaderVisability;
		m_parameters.emplace_back(param);
	}
};