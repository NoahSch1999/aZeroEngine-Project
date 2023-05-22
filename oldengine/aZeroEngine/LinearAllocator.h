#pragma once
#include "D3D12Include.h"

namespace aZeroAlloc
{
	/** @brief Contains the range (start and end index) inside for an Allocation object.
	*/
	struct AllocationRange
	{
		UINT64 m_startIndex = 0;
		UINT64 m_endIndex = 0;
		AllocationRange() = default;
		AllocationRange(UINT64 startIndex, UINT64 endIndex)
			:m_startIndex(startIndex), m_endIndex(endIndex) { }

	};

	/** @brief Wraps a pointer pointing to a range within a LinearUploadAllocator object.
	*/
	template<typename T>
	class Allocation
	{
	private:
		AllocationRange m_range;
		UINT64 m_currentIndex = 0;
		T* m_startPointer = nullptr;

	public:
		Allocation(AllocationRange range, T* startPointer)
			:m_range(range), m_startPointer(startPointer) { }

		Allocation() = default;
		~Allocation() = default;

		void operator++() { m_currentIndex++; m_startPointer++; }
		T& operator*() { return *m_startPointer; }

		/**Returns the current offset into the allocation range as an index.
		@return UINT
		*/
		UINT64 currentOffset() const { return m_currentIndex; }
	};

	/** @brief Wraps a ID3D12Resource in a committed upload heap with a linear allocator.
	*/
	template<typename T>
	class LinearUploadAllocator
	{
	protected:
		D3D12_GPU_VIRTUAL_ADDRESS m_gpuAddress;

		void* m_mappedPointer = nullptr;
		T* m_framePointer = nullptr;

		UINT64 m_maxElements = 0;
		UINT64 m_nextElement = 0;
		UINT64 m_currentElement = 0;

	public:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadBuffer = nullptr;

		/**Creates the neccessary DirectX12 resources and initiates the allocator.
		@param device ID3D12Device to create the resources with
		@param maxElements Max elements of size T which the upload buffer should allocate for
		@param numSubresources Number of subresources. Should be the same as frames in flight
		*/
		LinearUploadAllocator(ID3D12Device* device, UINT64 maxElements, UINT64 numSubresources)
			:m_maxElements(maxElements)
		{
			int width = (maxElements * sizeof(T)) * numSubresources;
			m_uploadBuffer = Helper::CreateBufferResource(device, width, D3D12_HEAP_TYPE_UPLOAD);

			m_uploadBuffer->Map(0, NULL, reinterpret_cast<void**>(&m_mappedPointer));
			m_framePointer = (T*)m_mappedPointer;

			m_gpuAddress = m_uploadBuffer->GetGPUVirtualAddress();

#ifdef _DEBUG
			const std::string nameStr = "Linear Alloc Upload Heap Resource: " + std::to_string(sizeof(T));
			const std::wstring nameWStr(nameStr.begin(), nameStr.end());
			m_uploadBuffer->SetName(nameWStr.c_str());
#endif // _DEBUG
		}

		/**Prepares the object for the comming frame.
		* Should be called before using the object each frame.
		@param frameIndex The current frame index of the engine (0, 1, or 2 for three frames in flight)
		@return void
		*/
		void beginFrame(UINT64 frameIndex)
		{
			m_framePointer = (T*)m_mappedPointer + (frameIndex * m_maxElements);
			m_nextElement = (frameIndex * m_maxElements);
			m_currentElement = m_nextElement;
		}

		/**Returns an Allocation<T> object which can be used to upload data to the resource with.
		@param numElements Number of elements of type T that the returned Allocation object should support
		@return Allocation<T>
		*/
		Allocation<T> getAllocation(UINT64 numElements)
		{
			UINT64 startIndex = m_currentElement;
			UINT64 endIndex = m_currentElement + numElements;

			T* currentOffset = m_framePointer;
			m_framePointer += numElements;
			m_currentElement += numElements;

			return Allocation<T>(AllocationRange(startIndex, endIndex), currentOffset);
		}

		/**Returns the virtual gpu address of the ID3D12Resource that should be used when binding.
		@return D3D12_GPU_VIRTUAL_ADDRESS
		*/
		D3D12_GPU_VIRTUAL_ADDRESS getVirtualAddress() const { return m_gpuAddress; }
	};

	/** @brief Wraps a VRAM default heap resource and sysRAM upload heap resource with a linear allocator.
	*/
	template<typename T>
	class LinearDefaultAllocator : public LinearUploadAllocator<T>
	{
	private:

	public:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_defaultBuffer = nullptr;

		/**Creates the neccessary DirectX12 resources and initiates the allocator.
		@param device ID3D12Device to create the resources with
		@param maxElements Max elements of size T which the upload buffer should allocate for
		@param numSubresources Number of subresources. Should be the same as frames in flight
		*/
		LinearDefaultAllocator(ID3D12Device* device, UINT64 maxElements, UINT64 numSubresources)
			:LinearUploadAllocator<T>(device, maxElements, numSubresources)
		{
			this->m_defaultBuffer = Helper::CreateBufferResource(device, maxElements * sizeof(T), D3D12_HEAP_TYPE_DEFAULT);
			this->m_gpuAddress = this->m_defaultBuffer->GetGPUVirtualAddress();

#ifdef _DEBUG
			const std::string nameStr = "Linear Alloc Default Heap Resource: " + std::to_string(sizeof(T));
			const std::wstring nameWStr(nameStr.begin(), nameStr.end());
			m_defaultBuffer->SetName(nameWStr.c_str());
#endif // _DEBUG
		}

		/**Records a copy command of a portion of memory from the upload heap resource to the default heap resource.
		@param commandList The ID3D12GraphicsCommandList to record the copy commands on
		@param range The range/portion of the memory to copy (specified in element indices)
		@param frameIndex Index of the current frame
		@return void
		*/
		void updateRange(ID3D12GraphicsCommandList* commandList, AllocationRange range, UINT64 frameIndex)
		{
			UINT64 defaultOffset = range.m_startIndex * sizeof(T);
			UINT64 uploadOffset = (sizeof(T) * this->m_maxElements) * frameIndex + range.m_startIndex * sizeof(T);

			UINT64 numBytesToCopy = (range.m_endIndex - range.m_startIndex) * sizeof(T);

			commandList->CopyBufferRegion(this->m_defaultBuffer.Get(), defaultOffset, this->m_uploadBuffer.Get(), uploadOffset, numBytesToCopy);
		}
		
		/**Records a full copy of the current frame index upload resource section to the default heap resource.
		@param commandList The ID3D12GraphicsCommandList to record the copy commands on
		@param frameIndex Index of the current frame
		@return void
		*/
		void updateAll(ID3D12GraphicsCommandList* commandList, UINT64 frameIndex)
		{
			UINT64 uploadOffset = (sizeof(T) * this->m_maxElements) * frameIndex;
			UINT64 numBytesToCopy = (this->m_currentElement - this->m_maxElements * frameIndex) * sizeof(T);
			commandList->CopyBufferRegion(this->m_defaultBuffer.Get(), 0, this->m_uploadBuffer.Get(), uploadOffset, numBytesToCopy);
		}

	};
}