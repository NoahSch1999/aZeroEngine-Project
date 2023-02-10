#pragma once
#include "D3D12Include.h"

class RootParameters
{
private:
	std::vector<D3D12_ROOT_PARAMETER> parameters;
	D3D12_DESCRIPTOR_RANGE ranges[100];
	int num = 0;
public:
	RootParameters() = default;

	~RootParameters() {

	}

	D3D12_ROOT_PARAMETER* GetParameterData() { return parameters.data(); }
	int GetParameterNum() { return parameters.size(); }

	void AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE _rangeType, UINT _baseShaderRegister, UINT _numDescriptors = 1, D3D12_SHADER_VISIBILITY _shaderVisability = D3D12_SHADER_VISIBILITY_ALL, UINT _offsetFromStart = 0, UINT _registerSpace = 0)
	{
		D3D12_DESCRIPTOR_RANGE range{ _rangeType, _numDescriptors, _baseShaderRegister, _registerSpace, _offsetFromStart };
		ranges[num] = range;
		D3D12_ROOT_DESCRIPTOR_TABLE table{ 1, &ranges[num] };
		num++;
		D3D12_ROOT_PARAMETER param{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, table, _shaderVisability };
		parameters.emplace_back(param);
	}

	void AddRootDescriptor(UINT _shaderRegister, D3D12_ROOT_PARAMETER_TYPE _paramType, D3D12_SHADER_VISIBILITY _shaderVisability = D3D12_SHADER_VISIBILITY_ALL, UINT _registerSpace = 0)
	{
		D3D12_ROOT_DESCRIPTOR descriptor{ _shaderRegister, _registerSpace };
		D3D12_ROOT_PARAMETER param;
		param.Descriptor = descriptor;
		param.ParameterType = _paramType;
		param.ShaderVisibility = _shaderVisability;
		parameters.emplace_back(param);
	}

	void AddRootConstants(UINT _shaderRegister, UINT _numBitValues, D3D12_SHADER_VISIBILITY _shaderVisability = D3D12_SHADER_VISIBILITY_ALL, UINT _registerSpace = 0)
	{
		D3D12_ROOT_CONSTANTS constants{ _shaderRegister, _registerSpace, _numBitValues };
		D3D12_ROOT_PARAMETER param;
		ZeroMemory(&param, sizeof(D3D12_ROOT_PARAMETER));
		param.Constants = constants;
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		param.ShaderVisibility = _shaderVisability;
		parameters.emplace_back(param);
	}
};