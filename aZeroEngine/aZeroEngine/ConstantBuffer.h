#pragma once
#include "BaseResource.h"
#include "ShaderDescriptorHeap.h"

/** @brief Encapsulates a constant buffer.
Can be either static or dynamic (single or tripple-buffered).
*/
class ConstantBuffer : public BaseResource
{
private:
	
public:

	ConstantBuffer();
	virtual ~ConstantBuffer();

	/**Initiates the resource as a Constant Buffer View with the input DescriptorHandle and asigns the input DescriptorHandle to the internal resource handle.
	@param _device Device to use when creating the view.
	@param _handle Handle to copy to the internal handle.
	@return void
	*/
	void InitAsCBV(ID3D12Device* _device, const DescriptorHandle& _handle);

	/**Returns whether the resource is static or not.
	@return bool
	*/
	bool IsStatic() { return isStatic; }

	/**Returns whether the resource is tripple buffered or not.
	@return bool
	*/
	bool IsTrippleBuffered() { return isTrippleBuffered; }


	// Inherited via BaseResource

	// has to be tested
	virtual void InitStatic(ID3D12Device* _device, CommandList* _cmdList, void* _initData, int _numBytes, int _numElements, const std::wstring& _mainResourceName) override;

	// has to be tested with single buffering
	virtual void InitDynamic(ID3D12Device* _device, CommandList* _cmdList, void* _initData, int _numBytes, int _numElements, bool _trippleBuffered, const std::wstring& _mainResourceName) override;

};

