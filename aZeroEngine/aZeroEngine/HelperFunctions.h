#pragma once
#include "D3D12Include.h"
#include "VertexDefinitions.h"
#include "CommandList.h"
#include <fstream>
#include <type_traits>

namespace Helper
{
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
			int len = _data.length();
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
}