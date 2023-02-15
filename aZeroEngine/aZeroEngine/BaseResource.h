#pragma once
#include "DescriptorHandle.h"
#include "HelperFunctions.h"

/** @brief Enumeration to be used in conjunction with the BaseResource::Init() method to specify what type of resource to initialize.
*/
enum class RESOURCETYPE { DYNAMIC, STATIC };

/** @brief The base class for all resources.
*/
class BaseResource
{
protected:
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
	ID3D12Resource* mainResource = nullptr;
	D3D12_RESOURCE_STATES mainResourceState = D3D12_RESOURCE_STATE_COMMON;
	ID3D12Resource* intermediateResource = nullptr;
	D3D12_RESOURCE_STATES intermediateResourceState = D3D12_RESOURCE_STATE_COMMON;
	DescriptorHandle handle;
	bool isStatic = false;
	bool isTrippleBuffered = false;
	int sizePerSubresource = -1;
	void* mappedBuffer = nullptr;

	template<RESOURCETYPE ResourceType>
	void Init(ID3D12Device* _device, CommandList* _cmdList, const D3D12_RESOURCE_DESC& _mDesc, D3D12_RESOURCE_DESC _iDesc, void* _initData);

public:
	BaseResource() = default;
	virtual ~BaseResource();

	/**Returns whether or not the resource is static or not.
	@return bool (TRUE: Is static, FALSE: Is dynamic)
	*/
	bool GetIsStatic() const { return isStatic; }

	/**Returns whether the resource is tripple buffered or not.
	@return bool
	*/
	bool IsTrippleBuffered() { return isTrippleBuffered; }

	/**Transitions the main GPU resource from one state to another.
	@param _gCmdList Pointer to a ID3D12GraphicsCommandList to register the transitioning to.
	@param _newState The state to transition to.
	@return void
	*/
	void TransitionMain(ID3D12GraphicsCommandList* _gCmdList, D3D12_RESOURCE_STATES _newState);

	/**Transitions the intermediate resource from one state to another.
	@param _gCmdList Pointer to a ID3D12GraphicsCommandList to register the transitioning to.
	@param _newState The state to transition to.
	@return void
	*/
	void TransitionIntermediate(ID3D12GraphicsCommandList* _gCmdList, D3D12_RESOURCE_STATES _newState);

	/**Return the GPU virtual address.
	@return D3D12_GPU_VIRTUAL_ADDRESS
	*/
	virtual D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const { return gpuAddress; }

	/**Sets the GPU virtual address.
	@param _gpuAddress The new virtual GPU address.
	@return void
	*/
	void SetGPUAddress(D3D12_GPU_VIRTUAL_ADDRESS _gpuAddress) { gpuAddress = _gpuAddress; }

	/**Returns a pointer reference to the internal main ID3D12Resource object.
	@return ID3D12Resource*&
	*/
	ID3D12Resource*& GetMainResource() { return mainResource; }

	/**Returns a pointer reference to the internal intermediate ID3D12Resource object.
	@return ID3D12Resource*&
	*/
	ID3D12Resource*& GetIntermediateResource() { return intermediateResource; }

	/**Returns the current resource state.
	@return D3D12_RESOURCE_STATES
	*/
	D3D12_RESOURCE_STATES GetMainState() const { return mainResourceState; }

	/**Returns the current resource state.
	@return D3D12_RESOURCE_STATES
	*/
	D3D12_RESOURCE_STATES GetIntermediateState() const { return intermediateResourceState; }

	/**Sets the current main resource state.
	@param _newState New state.
	@return void
	*/
	void SetMainState(D3D12_RESOURCE_STATES _newState) { mainResourceState = _newState; }

	/**Sets the current intermediate resource state.
	@param _newState New state.
	@return void
	*/
	void SetIntermediateState(D3D12_RESOURCE_STATES _newState) { intermediateResourceState = _newState; }

	/**Returns a pointer reference to the internal ID3D12Resource object.
	@return ID3D12Resource*&
	*/
	DescriptorHandle& GetHandle() { return handle; }

	/**A pure virtual method that is overridden by the subclasses.
	* When this overridden method is used, the resource cannot be modified.
	@param _device ID3D12Device to create the resource with.
	@param _cmdList CommandList to record the commands for.
	@param _initData Initial data for the resource.
	@param _numBytes Number of bytes that the resource should be allocated with.
	@param _mainResourceName Optional ID3D12Resource name.
	@return void
	*/
	virtual void InitStatic(ID3D12Device* _device, CommandList* _cmdList, void* _initData, int _numBytes, const std::wstring& _mainResourceName) = 0;

	/**A pure virtual method that is overridden by the subclasses.
	* When this overridden method is used, the resource is modifiable.
	@param _device ID3D12Device to create the resource with.
	@param _cmdList CommandList to record the commands for.
	@param _initData Initial data for the resource.
	@param _numBytes Number of bytes that the resource should be allocated with.
	@param _trippleBuffered Whether or not the resource should be tripple or single bufferd.
	@param _mainResourceName Optional ID3D12Resource name.
	@return void
	*/
	virtual void InitDynamic(ID3D12Device* _device, CommandList* _cmdList, void* _initData, int _numBytes, bool _trippleBuffered, const std::wstring& _mainResourceName) = 0;

	/**Method to be used if the resource is single buffered and dynamic.
	* Updates the mapped pointer that is pointing to the uploaded GPU resource.
	@param _data Data to copy to the resource.
	@param _numBytes Number of bytes to copy to the resource. The method will throw and exception if this is larger than the sizePerSubresource member variable, a.k.a. the size of each element rounded up to 255 bytes.
	@return void
	*/
	void Update(const void* _data, int _numBytes);

	/**Method to be used if the resource is tripple buffered and dynamic.
	* Updates the correct subsection of the mapped intermediate buffer and then copies that section to the main GPU resource.
	* The copying is recorded on the CommandList argument.
	@param _cmdList CommandList to record the copy and resource state transitioning commands on.
	@param _data Data to copy to the resource.
	@param _numBytes Number of bytes to copy to the resource. The method will throw and exception if this is larger than the sizePerSubresource member variable, a.k.a. the size of each element rounded up to 255 bytes.
	@param _frameIndex Current frame index of the engine. Used for copying to the correct subsection of the intermediate resource.
	@return void
	*/
	void Update(CommandList* _cmdList, const void* _data, int _numBytes, int _frameIndex, int _offset = 0);
};

template<RESOURCETYPE ResourceType>
inline void BaseResource::Init(ID3D12Device* _device, CommandList* _cmdList, const D3D12_RESOURCE_DESC& _mDesc, D3D12_RESOURCE_DESC _iDesc, void* _initData)
{
	if constexpr (ResourceType == RESOURCETYPE::STATIC)
	{
		isStatic = true;
		Helper::CreateCommitedResourceStatic(_device, mainResource, _mDesc, intermediateResource, _iDesc, _cmdList, _initData, sizePerSubresource, sizePerSubresource);
		gpuAddress = mainResource->GetGPUVirtualAddress();
		mainResourceState = D3D12_RESOURCE_STATE_COPY_DEST;
		TransitionMain(_cmdList->GetGraphicList(), D3D12_RESOURCE_STATE_GENERIC_READ);
	}
	else if constexpr (ResourceType == RESOURCETYPE::DYNAMIC)
	{
		isStatic = false;
		if (isTrippleBuffered)
		{
			_iDesc.Width = sizePerSubresource * 3;
			Helper::CreateCommitedResourceStatic(_device, mainResource, _mDesc, intermediateResource, _iDesc, _cmdList, _initData, sizePerSubresource, sizePerSubresource);
			intermediateResource->Map(0, NULL, reinterpret_cast<void**>(&mappedBuffer));
			intermediateResourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
			mainResourceState = D3D12_RESOURCE_STATE_COPY_DEST;
		}
		else
		{
			Helper::CreateCommitedResourceDynamic(_device, mainResource, _mDesc);
			mainResource->Map(0, NULL, reinterpret_cast<void**>(&mappedBuffer));
		}
	}
	else
	{
		static_assert(ResourceType != RESOURCETYPE::STATIC || ResourceType != RESOURCETYPE::DYNAMIC, "T is an invalid input.");
	}

	gpuAddress = mainResource->GetGPUVirtualAddress();
}
