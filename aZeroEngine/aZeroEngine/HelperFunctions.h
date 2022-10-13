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
}