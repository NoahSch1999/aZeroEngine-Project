#pragma once
#include "HiddenDescriptorHeap.h"
#include "ShaderDescriptorHeap.h"

/** @brief Manages DescriptorHandle creation within the engine.
Contains all ID3D12DescriptorHeap objects and necessary methods to access them.
*/
class DescriptorManager
{
private:
	ShaderDescriptorHeap resourceHeap;
	ShaderDescriptorHeap samplerHeap;
	HiddenDescriptorHeap dsvHeap;
	HiddenDescriptorHeap rtvHeap;

public:
	DescriptorManager() = default;

	/** Initializes the object.
	@param _device Device to use when creating the necessary D3D12 resources.
	@param _numResourceDescriptors Max number of D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV DescriptorHandle objects supported.
	@param _numSamplerDescriptors Max number of D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER DescriptorHandle objects supported.
	@param _numRTVDescriptors Max number of D3D12_DESCRIPTOR_HEAP_TYPE_RTV DescriptorHandle objects supported.
	@param _numDSVDescriptors Max number of D3D12_DESCRIPTOR_HEAP_TYPE_DSV DescriptorHandle objects supported.
	*/
	DescriptorManager(ID3D12Device* _device, int _numResourceDescriptors, int _numSamplerDescriptors = 1000, int _numRTVDescriptors = 1000, int _numDSVDescriptors = 1000);

	~DescriptorManager() = default;

	/** Initializes the object.
	@param _device Device to use when creating the necessary D3D12 resources.
	@param _numResourceDescriptors Max number of D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV DescriptorHandle objects supported.
	@param _numSamplerDescriptors Max number of D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER DescriptorHandle objects supported.
	@param _numRTVDescriptors Max number of D3D12_DESCRIPTOR_HEAP_TYPE_RTV DescriptorHandle objects supported.
	@param _numDSVDescriptors Max number of D3D12_DESCRIPTOR_HEAP_TYPE_DSV DescriptorHandle objects supported.
	@return void
	*/
	void Init(ID3D12Device* _device, int _numResourceDescriptors, int _numSamplerDescriptors = 1000, int _numRTVDescriptors = 1000, int _numDSVDescriptors = 1000);

	/** Returns a pointer to the internal ID3D12DescriptorHeap that contains D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV descriptors.
	@return ID3D12DescriptorHeap*
	*/
	ID3D12DescriptorHeap* GetResourceHeap() const { return resourceHeap.GetHeap(); }

	/** Returns a pointer to the internal ID3D12DescriptorHeap that contains D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER descriptors.
	@return ID3D12DescriptorHeap*
	*/
	ID3D12DescriptorHeap* GetSamplerHeap() const { return samplerHeap.GetHeap(); }

	/** Returns a pointer to the internal ID3D12DescriptorHeap that contains D3D12_DESCRIPTOR_HEAP_TYPE_RTV descriptors.
	@return ID3D12DescriptorHeap*
	*/
	ID3D12DescriptorHeap* GetRTVHeap() const { return rtvHeap.GetHeap(); }

	/** Returns a pointer to the internal ID3D12DescriptorHeap that contains D3D12_DESCRIPTOR_HEAP_TYPE_DSV descriptors.
	@return ID3D12DescriptorHeap*
	*/
	ID3D12DescriptorHeap* GetDSVHeap() const { return dsvHeap.GetHeap(); }

	/** Returns a new unused DescriptorHandle for D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV resources.
	@return DescriptorHandle
	*/
	DescriptorHandle GetResourceDescriptor() { return resourceHeap.GetDescriptorHandle(); }

	/**Returns a std::vector of new unused DescriptorHandle objects for D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV resources.
	@return std::vector<DescriptorHandle>
	*/
	std::vector<DescriptorHandle> GetResourceDescriptor(int _numDescriptors) { return resourceHeap.GetDescriptorHandles(_numDescriptors); }

	/** Frees up the input DescriptorHandle which allows it to be reused by calling DescriptorManager::GetResourceDescriptor().
	@param _handle The DescriptorHandle to free. The reference is modified and will contain a heap index of -1 to indicate it being freed.
	@return void
	*/
	void FreeResourceDescriptor(DescriptorHandle& _handle) { resourceHeap.FreeDescriptorHandle(_handle); }

	/** Returns a copy of a new unused DescriptorHandle for D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER resources.
	@return DescriptorHandle
	*/
	DescriptorHandle GetSamplerDescriptor() { return samplerHeap.GetDescriptorHandle(); }

	/** Frees up the input DescriptorHandle for a D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER resource which allows it to be reused by calling DescriptorManager::GetSamplerDescriptor().
	@param _handle The DescriptorHandle to free. The reference is modified and will contain a heap index of -1 to indicate it being freed.
	@return void
	*/
	void FreeSamplerDescriptor(DescriptorHandle& _handle) { resourceHeap.FreeDescriptorHandle(_handle); }

	/** Returns a copy of a new unused DescriptorHandle for D3D12_DESCRIPTOR_HEAP_TYPE_RTV resources.
	@return DescriptorHandle
	*/
	DescriptorHandle GetRTVDescriptor() { return rtvHeap.GetDescriptorHandle(); }

	/**Returns a std::vector of new unused DescriptorHandle objects for D3D12_DESCRIPTOR_HEAP_TYPE_RTV resources.
	@return std::vector<DescriptorHandle>
	*/
	std::vector<DescriptorHandle> GetRTVDescriptor(int _numDescriptors) { return rtvHeap.GetDescriptorHandles(_numDescriptors); }

	/** Frees up the input DescriptorHandle for a D3D12_DESCRIPTOR_HEAP_TYPE_RTV resource which allows it to be reused by calling DescriptorManager::GetRTVDescriptor().
	@param _handle The DescriptorHandle to free. The reference is modified and will contain a heap index of -1 to indicate it being freed.
	@return void
	*/
	void FreeRTVDescriptor(DescriptorHandle& _handle) { rtvHeap.FreeDescriptorHandle(_handle); }

	/** Returns a copy of a new unused DescriptorHandle for D3D12_DESCRIPTOR_HEAP_TYPE_DSV resources.
	@return DescriptorHandle
	*/
	DescriptorHandle GetDSVDescriptor() { return dsvHeap.GetDescriptorHandle(); }

	/**Returns a std::vector of new unused DescriptorHandle objects for D3D12_DESCRIPTOR_HEAP_TYPE_DSV resources.
	@return std::vector<DescriptorHandle>
	*/
	std::vector<DescriptorHandle> GetDSVDescriptor(int _numDescriptors){	return dsvHeap.GetDescriptorHandles(_numDescriptors);}

	/** Frees up the input DescriptorHandle for a D3D12_DESCRIPTOR_HEAP_TYPE_DSV resource which allows it to be reused by calling DescriptorManager::GetDSVDescriptor().
	@param _handle The DescriptorHandle to free. The reference is modified and will contain a heap index of -1 to indicate it being freed.
	@return void
	*/
	void FreeDSVDescriptor(DescriptorHandle& _handle) { dsvHeap.FreeDescriptorHandle(_handle); }

};