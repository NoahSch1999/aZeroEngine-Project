#pragma once
#include "ConstantBuffer.h"
#include "DepthStencil.h"
#include "RenderTarget.h"
#include "Texture2D.h"
#include "VertexBuffer.h"
#include "StructuredBuffer.h"
#include "CommandQueue.h"
#include "DescriptorManager.h"

class ResourceEngine
{
private:
	UINT64 lastSignal = 0;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> trashResources;

	DescriptorManager descriptorManager;

	CommandAllocator copyAllocator;
	CommandAllocator renderPassAllocator;

	CommandQueue directQueue;
	CommandQueue copyQueue;
	UINT frameIndex = 0;
	UINT frameCount = 0;

	ID3D12Device* device = nullptr;

	std::vector<std::shared_ptr<TextureResource>> readbackTextures;

public:

	CommandList renderPassList;
	CommandList copyList;

	ResourceEngine() = default;

	/** Initializes the ResourceEngine object.
	@param _device ID3D12Device to use for the DescriptorManager and CommandQueue objects.
	*/
	ResourceEngine(ID3D12Device* _device)
	{
		Init(_device);
	}

	/** Initializes the ResourceEngine object.
	@param _device ID3D12Device to use for the DescriptorManager and CommandQueue objects.
	@return void
	*/
	void Init(ID3D12Device* _device)
	{
		device = _device;
		descriptorManager.Init(_device, 1000, 1000);

		directQueue.Init(_device, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, D3D12_COMMAND_QUEUE_FLAG_NONE);
		copyQueue.Init(_device, D3D12_COMMAND_LIST_TYPE_COPY, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, D3D12_COMMAND_QUEUE_FLAG_NONE);

		renderPassAllocator.Init(_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
		renderPassList.Init(_device, renderPassAllocator);

		copyAllocator.Init(_device, D3D12_COMMAND_LIST_TYPE_COPY);
		copyList.Init(_device, copyAllocator);

#ifdef _DEBUG
		directQueue.GetQueue()->SetName(L"ResourceEngine Direct Queue");
		copyQueue.GetQueue()->SetName(L"ResourceEngine Copy Queue");

		copyAllocator.GetAllocator()->SetName(L"ResourceEngine Copy Allocator");

		renderPassAllocator.GetAllocator()->SetName(L"ResourceEngine Render Pass Allocator");
		renderPassList.GetBaseList()->SetName(L"ResourceEngine Render Pass Base List");
		renderPassList.GetGraphicList()->SetName(L"ResourceEngine Render Pass Graphics List");

		copyList.GetBaseList()->SetName(L"ResourceEngine Copy Base List");
		copyList.GetGraphicList()->SetName(L"ResourceEngine Copy Graphics List");
#endif // DEBUG
	}

	~ResourceEngine() = default;

	DescriptorManager& GetDescriptorManager() { return descriptorManager; }
	ID3D12DescriptorHeap* GetResourceHeap() { return descriptorManager.GetResourceHeap(); }
	ID3D12DescriptorHeap* GetSamplerHeap() { return descriptorManager.GetSamplerHeap(); }

	/** Should be called ASAP each frame.
	* Setups neccessary frame data for the ResourceEngine methods.
	@return void
	*/
	void BeginFrame()
	{
		frameIndex = frameCount % 3; // % num back buffers
	}

	/** Should be called rigth before the end of each frame.
	* Updates neccessary frame data for the ResourceEngine methods.
	@return void
	*/
	void EndFrame()
	{
		frameCount++;
	}

	/** Returns the current frame index (0, 1, or 2).
	@return UINT
	*/
	UINT GetFrameIndex() { return frameIndex; }

	/** Starts a CPU-side wait and then deallocates the ResourceEngine objects members.
	It also removes resources pending to be released that are added via ResourceEngine::RemoveResource().
	@return void
	*/
	void ShutDown()
	{
		directQueue.StallCPU(lastSignal);

		for (auto& resource : trashResources)
		{
			if (resource)
			{
				resource->Release();
				resource = nullptr;
			}
		}
	}

	/** Requests a readback from the CPU for the input TextureResource which has been created with _readback parameter as true.
	The data will be available once the Engine::EndFrame() has been called.

	TO DO:
	Enable the data to be available asap.

	@param _readbackTexture A shared pointer to TextureResource resource to read queue a GPU readback from.
	@return void
	*/
	void RequestReadback(std::shared_ptr<TextureResource> _readbackTexture)
	{
		//Footprint: It's a way to reinterpret a region of a buffer as if it was a texture, so that you can describe a 
		//   texture copy operation that will read/write to the buffer

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footPrint[1];
		UINT numRows = 0;
		UINT64 rowSize = 0;
		UINT64 totalSize = 0;

		D3D12_RESOURCE_DESC desc[1] = { _readbackTexture->GetDesc() };
		device->GetCopyableFootprints(desc, 0, 1, 0, footPrint, &numRows, &rowSize, &totalSize);

		D3D12_TEXTURE_COPY_LOCATION dest;
		dest.pResource = _readbackTexture->GetReadbackResource().Get();
		dest.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		dest.PlacedFootprint.Offset = 0;
		dest.PlacedFootprint = footPrint[0];

		D3D12_TEXTURE_COPY_LOCATION source;
		source.pResource = _readbackTexture->GetGPUOnlyResource().Get();
		source.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		source.SubresourceIndex = 0;

		_readbackTexture->Transition(renderPassList, D3D12_RESOURCE_STATE_COPY_SOURCE);

		renderPassList.GetGraphicList()->CopyTextureRegion(&dest, 0, 0, 0, &source, nullptr);

		_readbackTexture->Transition(renderPassList, D3D12_RESOURCE_STATE_RENDER_TARGET);

		readbackTextures.push_back(_readbackTexture);
	}

	/** Returns the ID3D12CommandQueue used for direct commands.
	@return ID3D12CommandQueue
	*/
	ID3D12CommandQueue* GetDirectQueue() { return directQueue.GetQueue(); }

	/** Returns the ID3D12CommandQueue used for copy commands.
	@return ID3D12CommandQueue
	*/
	ID3D12CommandQueue* GetCopyQueue() { return copyQueue.GetQueue(); }

	/** Starts a CPU-side wait for the direct CommandQueue to finnish.
	NOTE! This method doesn't execute any CommandList commands etc. It should be used to guarantee the direct 
	CommandQueue to be done with whathever it is executing, since ResourceEngine::Execute() doesn't add a CPU-side 
	wait (except every 3rd frame (when ResourceEngine::GetFrameIndex() returns 0)).
	@return void
	*/
	void FlushDirectQueue()
	{
		directQueue.StallCPU(lastSignal);
	}

	/** Initializes the input ConstantBuffer with the provided data.
	@param _resource ConstantBuffer object to initialize.
	@param _data Pointer to the data which the ConstantBuffer will get initiated with.
	@param _numBytes Number of bytes that the _data pointer points to.
	@param _dynamic Whether or not the ConstantBuffer is mutable (can be updated) or not. Default value is true.
	@param _tripple Whether or not the ConstantBuffer is tripple buffered or not. If _dynamic is false, this parameter won't have any effect. Default value is true.
	@return void
	*/
	void CreateResource(ConstantBuffer& _resource, void* _data, int _numBytes, bool _dynamic = true, bool _tripple = true)
	{
		_resource.Init(device, copyList, _data, _numBytes, _dynamic, _tripple);
		if (!_dynamic)
		{
			RemoveResource(_resource.GetUploadResource());
		}
	}

	/** Initializes the input StructuredBuffer with the provided data.
	@param _resource StructuredBuffer object to initialize.
	@param _data Pointer to the data which the StructuredBuffer will get initiated with.
	@param _numBytes Number of bytes that the _data pointer points to.
	@param _numElements Number of elements within the _data.
	@param _dynamic Whether or not the StructuredBuffer is mutable (can be updated) or not. Default value is true.
	@param _tripple Whether or not the StructuredBuffer is tripple buffered or not. If _dynamic is false, this parameter won't have any effect. Default value is true.
	@return void
	*/
	void CreateResource(StructuredBuffer& _resource, void* _data, int _numBytes, int _numElements, int _dynamic = true, int _tripple = true)
	{
		_resource.Init(device, copyList, _data, _numBytes, _numElements, _dynamic, _tripple);
		if (!_dynamic)
		{
			RemoveResource(_resource.GetUploadResource());
		}
	}

	/** Initializes the input DepthStencil with the provided data as a DSV and optionally also a SRV depending on paramater input.
	@param _resource DepthStencil object to initialize.
	@param _width Number of pixels in the x-axis of the DepthStencil.
	@param _height Number of pixels in the y-axis of the DepthStencil.
	@param _withSRV Whether or not the DepthStencil should be available as a SRV as well.
	@return void
	*/
	void CreateResource(DepthStencil& _resource, UINT _width, UINT _height, bool _withSRV)
	{
		if(!_withSRV)
			_resource.Init(device, descriptorManager.GetDSVDescriptor(), _width, _height);
		else
			_resource.Init(device, descriptorManager.GetDSVDescriptor(), descriptorManager.GetResourceDescriptor(), _width, _height);
	}

	/** Initializes the input RenderTarget with the provided data as a RTV and optionally also a SRV depending on paramater input.
	@param _resource RenderTarget object to initialize.
	@param _width Number of pixels in the x-axis of the RenderTarget.
	@param _height Number of pixels in the y-axis of the RenderTarget.
	@param _channels Number of channels per pixel. Should match _format parameter.
	@param _format DXGI_FORMAT of each pixel of the RenderTarget.
	@param _withSRV Whether or not the RenderTarget should be available as a SRV as well.
	@param _readback Whether or not the RenderTarget should be available as a readback resource as well. This enables it to be used with ResourceEngine::RequestReadback().
	@param _clearColor The clear color of the RenderTarget.
	@return void
	*/
	void CreateResource(RenderTarget& _resource, UINT _width, UINT _height, 
		UINT _channels, DXGI_FORMAT _format, bool _withSRV, bool _readback = false, const Vector4& _clearColor = Vector4(1.f, 1.f, 1.f, 1.f))
	{
		if(!_withSRV)
			_resource.Init(device, descriptorManager.GetRTVDescriptor(), _width, _height, _channels, _format, _readback);
		else
			_resource.Init(device, descriptorManager.GetRTVDescriptor(), descriptorManager.GetResourceDescriptor(),
			_width, _height, _channels, _format, _clearColor, _readback);
	}

	/** Initializes the input Texture2D with the provided data.
	@param _resource Texture2D object to initialize.
	@param _data Pointer to the data which the Texture2D will get initiated with.
	@param _width Number of pixels in the x-axis of the texture file.
	@param _height Number of pixels in the y-axis of the texture file.
	@param _channels Number of channels (RGB, RGBA etc.) per pixel of the texture file.
	@param _format DXGI_FORMAT of each pixel of the texture file.
	@param _name Name of the Texture2D object which later can be returned using Texture2D::GetFileName().
	@return void
	*/
	void CreateResource(Texture2D& _resource, void* _data, UINT _width, 
		UINT _height, UINT _channels, DXGI_FORMAT _format, const std::string& _name)
	{
		Microsoft::WRL::ComPtr<ID3D12Resource>& tempInterm = GetFrameResource();
		_resource.Init(device, renderPassList, copyList, tempInterm, descriptorManager.GetResourceDescriptor(), 
			_data, _width, _height, _channels, _format, _name);

#ifdef _DEBUG
		const std::string tName(_name + " Intermediate Temp");
		std::wstring wstr(tName.begin(), tName.end());
		tempInterm->SetName(wstr.c_str());
#endif // _DEBUG
	}

	/** Initializes the input VertexBuffer with the provided data.
	@param _resource VertexBuffer object to initialize.
	@param _data Pointer to the data which the VertexBuffer will get initiated with.
	@param _numBytes Number of bytes that the _data pointer points to.
	@param _numElements Number of vertices within the _data.
	@param _name Name of the VertexBuffer object which later can be returned using VertexBuffer::GetFileName().
	@return void
	*/
	void CreateResource(VertexBuffer& _resource, void* _data, int _numBytes, int _numElements, const std::string& _name)
	{
		_resource.Init(device, copyList, _data, _numBytes, _numElements, _name);
	}

	/** Sets the input non-nullptr BufferResource GPU-only and Upload ID3D12Resource to nullptr and 
	queues it for a GPU-side release by the end of every third frame (which guarantees it not to be in use when released).
	@param _resource The BufferResource to be deallocated.
	@return void
	*/
	void RemoveResource(BufferResource& _resource)
	{
		if (_resource.GetGPUOnlyResource())
		{
			trashResources.push_back(_resource.GetGPUOnlyResource());
			_resource.GetGPUOnlyResource() = nullptr;
		}

		if (_resource.GetUploadResource())
		{
			trashResources.push_back(_resource.GetUploadResource());
			_resource.GetUploadResource() = nullptr;
		}
	}

	/** Sets the input non-nullptr Texture2D GPU-only to nullptr and
	queues it for a GPU-side release by the end of every third frame (which guarantees it not to be in use when released).
	@param _resource The Texture2D to be deallocated.
	@return void
	*/
	void RemoveResource(Texture2D& _resource)
	{
		if (_resource.GetGPUOnlyResource())
		{
			trashResources.push_back(_resource.GetGPUOnlyResource());
			_resource.GetGPUOnlyResource() = nullptr;
			descriptorManager.FreeResourceDescriptor(_resource.GetHandle());
		}
	}

	/** Sets the input non-nullptr RenderTarget GPU-only and Readback resources to nullptr and
	queues them for a GPU-side release by the end of every third frame (which guarantees them not to be in use when released).
	@param _resource The RenderTarget to be deallocated.
	@return void
	*/
	void RemoveResource(RenderTarget& _resource)
	{
		if (_resource.GetGPUOnlyResource())
		{
			trashResources.push_back(_resource.GetGPUOnlyResource());
			_resource.GetGPUOnlyResource() = nullptr;
			descriptorManager.FreeRTVDescriptor(_resource.GetHandle());
		}

		if (_resource.GetReadbackResource())
		{
			trashResources.push_back(_resource.GetReadbackResource());
			_resource.GetReadbackResource() = nullptr;
		}

		if(_resource.GetSrvHandle().GetHeapIndex() != -1)
			descriptorManager.FreeResourceDescriptor(_resource.GetSrvHandle());
	}

	/** Sets the input non-nullptr DepthStencil GPU-only and Readback resources to nullptr and
	queues them for a GPU-side release by the end of every third frame (which guarantees them not to be in use when released).
	@param _resource The DepthStencil to be deallocated.
	@return void
	*/
	void RemoveResource(DepthStencil& _resource)
	{
		if (_resource.GetGPUOnlyResource())
		{
			trashResources.push_back(_resource.GetGPUOnlyResource());
			_resource.GetGPUOnlyResource() = nullptr;
			descriptorManager.FreeDSVDescriptor(_resource.GetHandle());
		}

		if (_resource.GetReadbackResource())
		{
			trashResources.push_back(_resource.GetReadbackResource());
			_resource.GetReadbackResource() = nullptr;
		}

		if (_resource.GetSrvHandle().GetHeapIndex() != -1)
			descriptorManager.FreeResourceDescriptor(_resource.GetSrvHandle());
	}

	/** Sets the input non-nullptr Microsoft::WRL::ComPtr<ID3D12Resource> to nullptr and
	queues it for a GPU-side release by the end of every third frame (which guarantees it not to be in use when released).
	@param _resource The Microsoft::WRL::ComPtr<ID3D12Resource> to be deallocated.
	@return void
	*/
	void RemoveResource(Microsoft::WRL::ComPtr<ID3D12Resource>& _resource)
	{
		if (_resource)
		{
			trashResources.push_back(_resource);
			_resource = nullptr;
		}
	}

	/** Returns a reference to a Microsoft::WRL::ComPtr<ID3D12Resource>* that will be released on the GPU-side 
	by the end of every third frame if it isn't nullptr by then.
	@return Microsoft::WRL::ComPtr<ID3D12Resource>&
	*/
	Microsoft::WRL::ComPtr<ID3D12Resource>& GetFrameResource()
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> tempResource = nullptr;
		trashResources.push_back(tempResource);
		return trashResources[trashResources.size() - 1];
	}

	/** Records a GPU-side update for the input ConstantBuffer which will be executed in the ResourceEngine::Execute() method.
	@param _resource ConstantBuffer to update.
	@param _data Pointer to the data that the ConstantBuffer will get updated with. Shouldn't be larger than the returned value from ConstantBuffer::GetSizePerSubresource().
	@return void
	*/
	void Update(ConstantBuffer& _resource, void* _data)
	{
		if (_resource.IsDynamic())
		{
			UINT64 offset = _resource.GetSizePerSubresource() * frameIndex;
			memcpy((char*)_resource.GetMappedBuffer() + offset, (char*)_data, _resource.GetSizePerSubresource());

			/*if (_resource.dirty)
				return;

			_resource.dirty = true;
			dirtyFlags.emplace_back(&_resource.dirty);*/

			copyList.GetGraphicList()->CopyBufferRegion(_resource.GetGPUOnlyResource().Get(), 0, _resource.GetUploadResource().Get(), offset, _resource.GetSizePerSubresource());
		}
	}

	/** Records a GPU-side update for the input StructuredBuffer which will be executed in the ResourceEngine::Execute() method.
	@param _resource StructuredBuffer to update.
	@param _data Pointer to the data that the StructuredBuffer will get updated with. Shouldn't be larger than the returned value from StructuredBuffer::GetSizePerSubresource().
	@return void
	*/
	void Update(StructuredBuffer& _resource, void* _data)
	{
		if (_resource.IsDynamic())
		{
			UINT64 offset = _resource.GetSizePerSubresource() * frameIndex;
			memcpy((char*)_resource.GetMappedBuffer() + offset, (char*)_data, _resource.GetSizePerSubresource());

			/*if (_resource.dirty)
				return;

			_resource.dirty = true;
			dirtyFlags.emplace_back(&_resource.dirty);*/
			
			// Crashes here occasionally for some reason...
			// Exception thrown at 0x00007FF6775545BB in aZeroEngine.exe: 0xC0000005: Access violation reading location 0x0000019642B4E000.
			copyList.GetGraphicList()->CopyBufferRegion(_resource.GetGPUOnlyResource().Get(), 0, _resource.GetUploadResource().Get(), offset, _resource.GetSizePerSubresource());
		}
	}

	/** Executes all recorded commands.
	* Does a GPU-side wait every frame to wait for neccessary CommandQueue objects.
	* Does a CPU-side wait every third frame to release GPU-side resources added via ResourceEngine::RemoveResource() or RemoveResource::GetFrameBuffer().
	@return void
	*/
	void Execute()
	{
		UINT64 copySignal = copyQueue.Execute(copyList);

		directQueue.WaitForOther(copyQueue, copySignal);
		UINT64 renderPassSignal = directQueue.Execute(renderPassList);

		if (frameIndex % 3 == 0)
		{
			directQueue.StallCPU(renderPassSignal);

			copyAllocator.Reset();
			renderPassAllocator.Reset();

			if (trashResources.size() > 0)
			{
				trashResources.clear();
			}
		}
		else
		{
			directQueue.WaitForFence(renderPassSignal);
			
		}

		if (readbackTextures.size() > 0)
		{
			D3D12_RANGE emptyRange{ 0,0 };
			for (auto& readbackTexture : readbackTextures)
			{
				D3D12_RANGE readbackBufferRange{ 0, readbackTexture->GetReadbackSize() };

				readbackTexture->GetReadbackResource()->Map(0, &readbackBufferRange, reinterpret_cast<void**>(&readbackTexture->GetReadbackData()));
				readbackTexture->GetReadbackResource()->Unmap(0, &emptyRange);
			}
			readbackTextures.clear();
		}

		copyList.ResetGraphic(copyAllocator);
		renderPassList.ResetGraphic(renderPassAllocator);
	}
};