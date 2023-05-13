#pragma once
#include "HiddenDescriptorHeap.h"
#include "ShaderDescriptorHeap.h"

/** @brief Manages DescriptorHandle creation within the engine.
Contains all ID3D12DescriptorHeap objects and necessary methods to access them.
*/
class DescriptorManager
{
private:
	ShaderDescriptorHeap m_resourceHeap;
	ShaderDescriptorHeap m_samplerHeap;
	HiddenDescriptorHeap m_dsvHeap;
	HiddenDescriptorHeap m_rtvHeap;

public:
	DescriptorManager() = default;

	/** Initializes the object.
	@param _device Device to use when creating the necessary D3D12 resources.
	@param _numResourceDescriptors Max number of D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV DescriptorHandle objects supported.
	@param _numSamplerDescriptors Max number of D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER DescriptorHandle objects supported.
	@param _numRTVDescriptors Max number of D3D12_DESCRIPTOR_HEAP_TYPE_RTV DescriptorHandle objects supported.
	@param _numDSVDescriptors Max number of D3D12_DESCRIPTOR_HEAP_TYPE_DSV DescriptorHandle objects supported.
	*/
	DescriptorManager(ID3D12Device* device, int numResourceDescriptors, int numSamplerDescriptors = 1000, int numRTVDescriptors = 1000, int numDSVDescriptors = 1000);

	~DescriptorManager() = default;

	/** Initializes the object.
	@param _device Device to use when creating the necessary D3D12 resources.
	@param _numResourceDescriptors Max number of D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV DescriptorHandle objects supported.
	@param _numSamplerDescriptors Max number of D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER DescriptorHandle objects supported.
	@param _numRTVDescriptors Max number of D3D12_DESCRIPTOR_HEAP_TYPE_RTV DescriptorHandle objects supported.
	@param _numDSVDescriptors Max number of D3D12_DESCRIPTOR_HEAP_TYPE_DSV DescriptorHandle objects supported.
	@return void
	*/
	void Init(ID3D12Device* device, int numResourceDescriptors, int numSamplerDescriptors = 1000, int numRTVDescriptors = 1000, int numDSVDescriptors = 1000);

	/** Returns a pointer to the internal ID3D12DescriptorHeap that contains D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV descriptors.
	@return ID3D12DescriptorHeap*
	*/
	ID3D12DescriptorHeap* getResourceHeap() const { return m_resourceHeap.getHeap(); }

	ShaderDescriptorHeap& getResourceShaderDescriptorHeap() { return m_resourceHeap; }
	ShaderDescriptorHeap& getSamplerShaderDescriptorHeap() { return m_samplerHeap; }
	HiddenDescriptorHeap& getRTVDescriptorHeap() { return m_rtvHeap; }
	HiddenDescriptorHeap& getDSVDescriptorHeap() { return m_dsvHeap; }

	/** Returns a pointer to the internal ID3D12DescriptorHeap that contains D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER descriptors.
	@return ID3D12DescriptorHeap*
	*/
	ID3D12DescriptorHeap* getSamplerHeap() const { return m_samplerHeap.getHeap(); }

	/** Returns a pointer to the internal ID3D12DescriptorHeap that contains D3D12_DESCRIPTOR_HEAP_TYPE_RTV descriptors.
	@return ID3D12DescriptorHeap*
	*/
	ID3D12DescriptorHeap* getRTVHeap() const { return m_rtvHeap.getHeap(); }

	/** Returns a pointer to the internal ID3D12DescriptorHeap that contains D3D12_DESCRIPTOR_HEAP_TYPE_DSV descriptors.
	@return ID3D12DescriptorHeap*
	*/
	ID3D12DescriptorHeap* getDSVHeap() const { return m_dsvHeap.getHeap(); }

	/** Returns a new unused DescriptorHandle for D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV resources.
	@return DescriptorHandle
	*/
	DescriptorHandle getResourceDescriptor() { return m_resourceHeap.getDescriptorHandle(); }

	/**Returns a std::vector of new unused DescriptorHandle objects for D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV resources.
	@return std::vector<DescriptorHandle>
	*/
	std::vector<DescriptorHandle> getResourceDescriptor(int numDescriptors) { return m_resourceHeap.getDescriptorHandles(numDescriptors); }

	/** Frees up the input DescriptorHandle which allows it to be reused by calling DescriptorManager::GetResourceDescriptor().
	@param handle The DescriptorHandle to free. The reference is modified and will contain a heap index of -1 to indicate it being freed.
	@return void
	*/
	void freeResourceDescriptor(DescriptorHandle& handle) { m_resourceHeap.freeDescriptorHandle(handle); }

	/** Returns a copy of a new unused DescriptorHandle for D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER resources.
	@return DescriptorHandle
	*/
	DescriptorHandle getSamplerDescriptor() { return m_samplerHeap.getDescriptorHandle(); }

	/** Frees up the input DescriptorHandle for a D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER resource which allows it to be reused by calling DescriptorManager::GetSamplerDescriptor().
	@param handle The DescriptorHandle to free. The reference is modified and will contain a heap index of -1 to indicate it being freed.
	@return void
	*/
	void freeSamplerDescriptor(DescriptorHandle& handle) { m_resourceHeap.freeDescriptorHandle(handle); }

	/** Returns a copy of a new unused DescriptorHandle for D3D12_DESCRIPTOR_HEAP_TYPE_RTV resources.
	@return DescriptorHandle
	*/
	DescriptorHandle getRTVDescriptor() { return m_rtvHeap.getDescriptorHandle(); }

	/**Returns a std::vector of new unused DescriptorHandle objects for D3D12_DESCRIPTOR_HEAP_TYPE_RTV resources.
	@return std::vector<DescriptorHandle>
	*/
	std::vector<DescriptorHandle> getRTVDescriptor(int numDescriptors) { return m_rtvHeap.getDescriptorHandles(numDescriptors); }

	/** Frees up the input DescriptorHandle for a D3D12_DESCRIPTOR_HEAP_TYPE_RTV resource which allows it to be reused by calling DescriptorManager::GetRTVDescriptor().
	@param _handle The DescriptorHandle to free. The reference is modified and will contain a heap index of -1 to indicate it being freed.
	@return void
	*/
	void freeRTVDescriptor(DescriptorHandle& handle) { m_rtvHeap.freeDescriptorHandle(handle); }

	/** Returns a copy of a new unused DescriptorHandle for D3D12_DESCRIPTOR_HEAP_TYPE_DSV resources.
	@return DescriptorHandle
	*/
	DescriptorHandle getDSVDescriptor() { return m_dsvHeap.getDescriptorHandle(); }

	/**Returns a std::vector of new unused DescriptorHandle objects for D3D12_DESCRIPTOR_HEAP_TYPE_DSV resources.
	@return std::vector<DescriptorHandle>
	*/
	std::vector<DescriptorHandle> getDSVDescriptor(int numDescriptors) { return m_dsvHeap.getDescriptorHandles(numDescriptors); }

	/** Frees up the input DescriptorHandle for a D3D12_DESCRIPTOR_HEAP_TYPE_DSV resource which allows it to be reused by calling DescriptorManager::GetDSVDescriptor().
	@param _handle The DescriptorHandle to free. The reference is modified and will contain a heap index of -1 to indicate it being freed.
	@return void
	*/
	void freeDSVDescriptor(DescriptorHandle& handle) { m_dsvHeap.freeDescriptorHandle(handle); }

};