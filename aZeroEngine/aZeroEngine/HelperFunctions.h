#pragma once
#include "D3D12Include.h"
#include "VertexDefinitions.h"
#include <fstream>
#include <type_traits>
#include "CommandList.h"

namespace Helper
{
#ifndef NDEBUG
#define DEBUGLOG(x) std::cout << x << std::endl
#else
#define DEBUGLOG(x)
#endif

	struct STBIImageData
	{
		unsigned char* rawData = nullptr;
		int height = -1;
		int width = -1;
		int channels = -1;
	};

	ID3DBlob* LoadBlobFromFile(const std::wstring& _filePath);

	struct BasicVertexListInfo
	{
		std::vector<BasicVertex> verticeData;
		std::vector<int> indexData;
	};

	void LoadVertexListFromFile(BasicVertexListInfo* _vInfo, const std::string& _path);

	bool GetDisplaySettings(DEVMODEA* _devMode);

	/*void GetWindowDimensions(AppWindow* _window);*/

	void GetWindowDimensions(UINT* _width, UINT* _height);

	void Print(Vector3 _vec);

	void CreateCommitedResourceStatic(ID3D12Device* _device, ID3D12Resource*& _mainResource, const D3D12_RESOURCE_DESC& _rDesc, 
		ID3D12Resource*& _interResource,  const D3D12_RESOURCE_DESC& _uDesc,
		CommandList* _cmdList, const void* _initData, int _rowPitch, int _slicePitch);

	void CreateCommitedResourceDynamic(ID3D12Device* _device, ID3D12Resource*& _mainResource, const D3D12_RESOURCE_DESC& _rDesc);

	template<typename T>
	void WriteToFile(std::ofstream& _file, const T& _data);

	template<typename T>
	void ReadFromFile(std::ifstream& _file, T& _data);

	template<typename T>
	void WriteToFile(std::ofstream& _file, const T& _data)
	{
		if constexpr (std::is_same_v<T, std::string>)
		{
			int len = (int)_data.length();
			_file.write((char*)&len, sizeof(int));
			_file.write(_data.c_str(), len);
		}
		else
		{
			static_assert(std::is_same_v<T, std::string>, "T is an invalid input.");
		}
	}

	template<typename T>
	void ReadFromFile(std::ifstream& _file, T& _data)
	{
		if constexpr (std::is_same_v<T, std::string>)
		{
			int len = -1;
			_file.read((char*)&len, sizeof(int));
			_data.resize(len);
			_file.read(_data.data(), len);
		}
		else
		{
			static_assert(std::is_same_v<T, std::string>, "T is an invalid input.");
		}
	}

	STBIImageData LoadSTBIImage(const std::string& _fileName);

	/** @brief Opens the file dialog and returns true if the user selects a file that ends with the input .xxx extension. Otherwise it returns false.
	* If it returns true, it will also copy the filename.xxx to the input _storeFileStr argument.
	@return TRUE: File with extension selected, FALSE: File with extension not selected
	*/
	bool OpenFileDialogForExtension(const std::string& _extension, std::string& _storeFileStr);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateReadbackBuffer(ID3D12Device* _device, int _rowPitch, int _numRows);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* _device, int _width, D3D12_HEAP_TYPE _heapType);

	void CreateBufferResource(ID3D12Device* _device, CommandList& _cmdList, Microsoft::WRL::ComPtr<ID3D12Resource>& _gpuOnlyResource, int _gpuOnlyWidth, Microsoft::WRL::ComPtr<ID3D12Resource>& _mappedResource, int _mappedWidth, void* _data);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(ID3D12Device* _device, D3D12_RESOURCE_DESC& _desc, int _width, int _height, DXGI_FORMAT _format, D3D12_RESOURCE_FLAGS _flags, D3D12_HEAP_TYPE _heapType,
		D3D12_CLEAR_VALUE _clearValue, D3D12_RESOURCE_STATES& _initialState);

	void CreateTextureResource(ID3D12Device* _device, CommandList& _transitionList, CommandList& _copyList, Microsoft::WRL::ComPtr<ID3D12Resource>& _gpuOnlyResource,
		Microsoft::WRL::ComPtr<ID3D12Resource>& _intermediateResource, void* _data, int _width, int _height, int _channels, DXGI_FORMAT _format, D3D12_RESOURCE_STATES _initState);

	void CreateRTVHandle(ID3D12Device* _device, Microsoft::WRL::ComPtr<ID3D12Resource> _resource, D3D12_CPU_DESCRIPTOR_HANDLE _cpuHandle, DXGI_FORMAT _format);

	void CreateDSVHandle(ID3D12Device* _device, Microsoft::WRL::ComPtr<ID3D12Resource> _resource, D3D12_CPU_DESCRIPTOR_HANDLE _cpuHandle, DXGI_FORMAT _format);

	void CreateSRVHandle(ID3D12Device* _device, Microsoft::WRL::ComPtr<ID3D12Resource> _resource, D3D12_CPU_DESCRIPTOR_HANDLE _cpuHandle, DXGI_FORMAT _format);

}