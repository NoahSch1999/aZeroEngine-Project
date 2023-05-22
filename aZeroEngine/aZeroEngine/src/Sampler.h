#pragma once
#include "DescriptorHandle.h"
#include "VertexDefinitions.h"

/** @brief Encapsulates a sampler.
NOTE! Will be reworked to reduce creation overhead and memory consumption.
*/
class Sampler
{
private:
	DescriptorHandle m_handle;
	D3D12_STATIC_SAMPLER_DESC m_staticDesc = D3D12_STATIC_SAMPLER_DESC();

public:
	Sampler() = default;

	/* TO BE EDITED
	@param
	@return
	*/
	Sampler(ID3D12Device* _device, DescriptorHandle handle, D3D12_FILTER filter,
		D3D12_TEXTURE_ADDRESS_MODE addressModeU = D3D12_TEXTURE_ADDRESS_MODE_WRAP, 
		D3D12_TEXTURE_ADDRESS_MODE addressModeV = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE addressModeW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_COMPARISON_FUNC comparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
		int maxAnisotropy = 16, DXM::Vector4 borderColor = { 1,1,1,1 },
		float midLodBias = 0, float minLod = 0, float maxLod = D3D12_FLOAT32_MAX);

	/* TO BE EDITED
	@param
	@return
	*/
	Sampler(D3D12_FILTER filter, 
		int shaderRegister, D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL, int registerSpace = 0,
		D3D12_TEXTURE_ADDRESS_MODE addressModeU = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE addressModeV = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE addressModeW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_COMPARISON_FUNC comparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
		int maxAnisotropy = 16, DXM::Vector4 borderColor = { 1,1,1,1 },
		float midLodBias = 0, float minLod = 0, float maxLod = D3D12_FLOAT32_MAX); // _maxLod has to be above 0 for some reason.

	~Sampler() = default;

	/* TO BE EDITED
	@param
	@return
	*/
	void init(ID3D12Device* device, DescriptorHandle handle, D3D12_FILTER filter,
		D3D12_TEXTURE_ADDRESS_MODE addressModeU = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE addressModeV = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE addressModeW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_COMPARISON_FUNC comparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
		int maxAnisotropy = 16, DXM::Vector4 borderColor = { 1,1,1,1 },
		float midLodBias = 0, float minLod = 0, float maxLod = D3D12_FLOAT32_MAX);

	/* TO BE EDITED
	@param
	@return
	*/
	D3D12_STATIC_SAMPLER_DESC getStaticDesc() const { return m_staticDesc; }

	/* TO BE EDITED
	@param
	@return
	*/
	DescriptorHandle getHandle() const { return m_handle; }

	/* TO BE EDITED
	@param
	@return
	*/
	void setHandle(const DescriptorHandle& handle) { m_handle = handle; }
};

