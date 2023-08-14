#pragma once
#include <d3dcompiler.h>
#include "D3D12Include.h"
#include "HelperFunctions.h"
#include <d3d12shader.h>
#include <dxcapi.h>

class Shader
{
private:
	Microsoft::WRL::ComPtr<ID3DBlob> m_compiledShader = nullptr;

public:
	Shader(std::string_view srcPath);

	ID3DBlob* getShaderBlob() { return m_compiledShader.Get(); }
};