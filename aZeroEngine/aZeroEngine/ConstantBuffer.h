#pragma once
#include "BaseResource.h"
#include "ShaderDescriptorHeap.h"
#include "HiddenDescriptorHeap.h"
#include "CommandList.h"

/** @brief Encapsulates a constant buffer.
Can be either static or dynamic (single or tripple-buffered).
*/
class ConstantBuffer : public BaseResource
{
private:
	ID3D12Resource* uploadBuffer;
	D3D12_RESOURCE_STATES uploadState = D3D12_RESOURCE_STATE_COMMON;
	int totalSize;
	bool isStatic;
	bool isTrippleBuffered;
	int sizePerSubresource;
	void* mappedBuffer;
public:
	ConstantBuffer();
	~ConstantBuffer();

	/**Initiates as a static constant buffer. This means that it DOESN'T support the ConstantBuffer::Update() methods.
	@param _device Device to use when creating the D3D12 resources.
	@param _cmdList CommandList to execute the resource initiation commands on.
	@param _data Data to initiate the buffer with.
	@param _size Size of the data to initiate with (will be aligned to 255 bytes).
	@param _name Optional name for the internal ID3D12Resource object.
	@return void
	*/
	void InitStatic(ID3D12Device* _device, CommandList* _cmdList, void* _data, int _size, const std::wstring& _name = L"");

	/**Initiates as a dynamic constant buffer. This means that it DOES support the ConstantBuffer::Update() methods.
	@param _device Device to use when creating the D3D12 resources.
	@param _cmdList CommandList to execute the resource initiation commands on.
	@param _data Data to initiate the buffer with.
	@param _size Size of the data to initiate with (will be aligned to 255 bytes).
	@param _trippleBuffered Defaulted to single buffered. If this argument is true, ALWAYS use the ConstantBuffer::Update(CommandList* _cmdList, const void* _data, int _size, int _frameIndex) method when updating it to avoid data races between the GPU and CPU.
	@param _name Optional name for the internal ID3D12Resource object.
	@return void
	*/
	void InitAsDynamic(ID3D12Device* _device, CommandList* _cmdList, void* _data, int _size, bool _trippleBuffering = false, const std::wstring& _name = L"");

	/**Initiates the resource as a Constant Buffer View with the input DescriptorHandle and asigns the input DescriptorHandle to the internal resource handle.
	@param _device Device to use when creating the view.
	@param _handle Handle to copy to the internal handle.
	@return void
	*/
	void InitAsCBV(ID3D12Device* _device, const DescriptorHandle& _handle);

	/**Updates the constant buffer with the input data. Only use when the resource is single buffered AND dynamic AND if the CPU and GPU just got synchronized.
	@param _data Data to copy to the constant buffer.
	@param _size Size of the data.
	@return void
	*/
	void Update(const void* _data, int _size);

	/**Updates the constant buffer with the input data. Only use when the resource is tripple buffered.
	@param _cmdList Data to copy to the constant buffer.
	@param _data Data to copy to the constant buffer.
	@param _size Size of the data.
	@param _frameIndex Current frame index (0, 1, or 2).
	@return void
	*/
	void Update(CommandList* _cmdList, const void* _data, int _size, int _frameIndex);

	/**Returns whether the resource is static or not.
	@return bool
	*/
	bool IsStatic() { return isStatic; }

	/**Returns whether the resource is tripple buffered or not.
	@return bool
	*/
	bool IsTrippleBuffered() { return isTrippleBuffered; }
};

