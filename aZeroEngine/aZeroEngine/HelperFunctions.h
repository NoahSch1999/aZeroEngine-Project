#pragma once
#include "D3D12Include.h"
#include "VertexDefinitions.h"
#include "AppWindow.h"

namespace Helper
{
	ID3DBlob* LoadBlobFromFile(const std::wstring& _filePath);

	struct BasicVertexListInfo
	{
		std::vector<BasicVertex>verticeData;
	};

	void LoadVertexListFromFile(BasicVertexListInfo* _vInfo, const std::string& _path);

	bool GetDisplaySettings(DEVMODEA* _devMode);

	void GetWindowDimensions(AppWindow* _window);

	void GetWindowDimensions(UINT& _width, UINT& _height);

	void Print(Vector3 _vec);

	void CreateCommitedResourceStatic(ID3D12Device* _device, ID3D12Resource*& _mainResource, const D3D12_RESOURCE_DESC& _rDesc, 
		ID3D12Resource*& _interResource,  const D3D12_RESOURCE_DESC& _uDesc,
		CommandList* _cmdList, const void* _initData, int _rowPitch, int _slicePitch);

	void CreateCommitedResourceDynamic(ID3D12Device* _device, ID3D12Resource*& _mainResource, const D3D12_RESOURCE_DESC& _rDesc);
}