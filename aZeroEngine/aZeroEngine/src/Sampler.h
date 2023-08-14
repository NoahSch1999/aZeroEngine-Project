#pragma once
#include "DescriptorManager.h"
#include "VertexDefinitions.h"

/** @brief Encapsulates a descriptor heap Sampler.
*/
class Sampler
{
private:
	DescriptorManager* m_descriptorManager = nullptr;
	DescriptorHandle m_handle;
	D3D12_SAMPLER_DESC m_desc = D3D12_SAMPLER_DESC();

public:
	Sampler() = default;

	/*Creates a Sampler with a descriptor handle.
	@param device ID3D12Device used to create the Sampler. Should be the same as used for the descriptorManager argument.
	@param descriptorManager DescriptorManager to generate DescriptorHandle objects from.
	@param filter D3D12_FILTER used.
	@param addressModeU D3D12_TEXTURE_ADDRESS_MODE used in the U dimension.
	@param addressModeV D3D12_TEXTURE_ADDRESS_MODE used in the V dimension.
	@param addressModeW D3D12_TEXTURE_ADDRESS_MODE used in the W dimension.
	@param comparisonFunc D3D12_COMPARISON_FUNC used.
	@param maxAnisotropy Maximum anisotropic filtering count. Ignored unless the filter argument is of an anisotropic type. Values should be within a range of 1-16.
	@param borderColor Color of the out-of-bounds sampling when a input D3D12_TEXTURE_ADDRESS_MODE argument is of type D3D12_TEXTURE_ADDRESS_MODE_BORDER.
	@param midLodBias Mip LOD bias. If the runtime calculates that a texture should be sampled at mipmap level 3 and midLodBias is 2, the texture will be sampled at mipmap level 5.
	@param minLod Minimum Mip LOD level.
	@param maxLod Maximum Mip LOD level.
	*/
	Sampler(ID3D12Device* device, DescriptorManager& descriptorManager, D3D12_FILTER filter,
		D3D12_TEXTURE_ADDRESS_MODE addressModeU = D3D12_TEXTURE_ADDRESS_MODE_WRAP, 
		D3D12_TEXTURE_ADDRESS_MODE addressModeV = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE addressModeW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_COMPARISON_FUNC comparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
		int maxAnisotropy = 16, DXM::Vector4 borderColor = { 1,1,1,1 },
		float midLodBias = 0, float minLod = 0, float maxLod = D3D12_FLOAT32_MAX);

	~Sampler()
	{
		if (m_handle.getHeapIndex() != -1)
			m_descriptorManager->freeSamplerDescriptor(m_handle);
	}

	Sampler(const Sampler& other);

	Sampler& operator=(const Sampler& other);

	Sampler(Sampler&& other) noexcept;

	Sampler& operator=(Sampler&& other) noexcept;

	/*Returns a copy of the Sampler objects' DescriptorHandle.
	@return DescriptorHandle
	*/
	DescriptorHandle getHandle() const { return m_handle; }
};

