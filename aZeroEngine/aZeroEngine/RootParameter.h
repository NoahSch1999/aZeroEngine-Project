#pragma once
#include "D3D12Include.h"

/** @brief Encapsulates a Root Parameter list object.
*/
class RootParameters
{
private:
	std::vector<D3D12_ROOT_PARAMETER> parameters;
	D3D12_DESCRIPTOR_RANGE ranges[100] = {};
	int num = 0;
public:
	RootParameters() = default;

	~RootParameters() = default;

	/** @brief Resets the root parameter list in order for it to be reused.
	@return void
	*/
	void Reset()
	{
		num = 0;
		ZeroMemory(&ranges, sizeof(ranges));
		parameters.clear();
	}

	/** @brief Returns a pointer to the start of the parameter list.
	@return D3D12_ROOT_PARAMETER*
	*/
	D3D12_ROOT_PARAMETER* GetParameterData() { return parameters.data(); }

	/** @brief Returns the number of parameters within the list.
	@return int
	*/
	int GetParameterNum() { return (int)parameters.size(); }

	/** @brief Adds a descriptor table with the specified arguments to the parameter list.
	@param _rangeType D3D12_DESCRIPTOR_RANGE_TYPE of the descriptor table.
	@param _baseShaderRegister Shader register start of the descriptor table.
	@param _numDescriptors Number of descriptors in the descriptor table.
	@param _shaderVisability Shader visibility of the descriptor table. Defaulted to D3D12_SHADER_VISIBILITY_ALL.
	@param _offsetFromStart Offset of the range from the start of the range set as the root argument value for this descriptor table parameter slot. Defaulted to 0, ie. no offset.
	@param _registerSpace The register space for the descriptor table parameter. Defaulted to 0.
	@return void
	*/
	void AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE _rangeType, UINT _baseShaderRegister, UINT _numDescriptors = 1, D3D12_SHADER_VISIBILITY _shaderVisability = D3D12_SHADER_VISIBILITY_ALL, UINT _offsetFromStart = 0, UINT _registerSpace = 0)
	{
		D3D12_DESCRIPTOR_RANGE range{ _rangeType, _numDescriptors, _baseShaderRegister, _registerSpace, _offsetFromStart };
		ranges[num] = range;
		D3D12_ROOT_DESCRIPTOR_TABLE table{ 1, &ranges[num] };
		num++;
		D3D12_ROOT_PARAMETER param{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, table, _shaderVisability };
		parameters.emplace_back(param);
	}

	/** @brief Adds a root descriptor with the specified arguments to the parameter list.
	@param _shaderRegister Shader register of the root descriptor.
	@param _paramType Parameter type of the root descriptor.
	@param _shaderVisability Shader visibility of the root descriptor. Defaulted to D3D12_SHADER_VISIBILITY_ALL.
	@param _registerSpace The register space for the root descriptor parameter. Defaulted to 0.	
	@return void
	*/
	void AddRootDescriptor(UINT _shaderRegister, D3D12_ROOT_PARAMETER_TYPE _paramType, D3D12_SHADER_VISIBILITY _shaderVisability = D3D12_SHADER_VISIBILITY_ALL, UINT _registerSpace = 0)
	{
		D3D12_ROOT_DESCRIPTOR descriptor{ _shaderRegister, _registerSpace };
		D3D12_ROOT_PARAMETER param;
		param.Descriptor = descriptor;
		param.ParameterType = _paramType;
		param.ShaderVisibility = _shaderVisability;
		parameters.emplace_back(param);
	}

	/** @brief Adds a root constant with the specified arguments to the parameter list.
	@param _shaderRegister Shader register of the root constant.
	@param _num32BitValues Number of 32bit (4 bytes) values that the root constant should contain.
	@param _shaderVisability Shader visibility of the root descriptor. Defaulted to D3D12_SHADER_VISIBILITY_ALL.
	@param _registerSpace The register space for the root descriptor parameter. Defaulted to 0.	
	@return void
	*/
	void AddRootConstants(UINT _shaderRegister, UINT _num32BitValues, D3D12_SHADER_VISIBILITY _shaderVisability = D3D12_SHADER_VISIBILITY_ALL, UINT _registerSpace = 0)
	{
		D3D12_ROOT_CONSTANTS constants{ _shaderRegister, _registerSpace, _num32BitValues };
		D3D12_ROOT_PARAMETER param;
		ZeroMemory(&param, sizeof(D3D12_ROOT_PARAMETER));
		param.Constants = constants;
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		param.ShaderVisibility = _shaderVisability;
		parameters.emplace_back(param);
	}
};