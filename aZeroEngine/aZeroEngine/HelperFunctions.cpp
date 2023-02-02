#include "pch.h"
#include "HelperFunctions.h"
#include <fstream>
#include "..\Assimp\include\assimp\scene.h"
#include "..\Assimp\include\assimp\Importer.hpp"
#include "..\Assimp\include\assimp\postprocess.h"

ID3DBlob* Helper::LoadBlobFromFile(const std::wstring& _filePath)
{
	std::ifstream fin(_filePath, std::ios::binary);
	fin.seekg(0, std::ios_base::end);
	std::streampos size = fin.tellg();
	fin.seekg(0, std::ios_base::beg);

	ID3DBlob* blob;
	HRESULT hr = D3DCreateBlob(size, &blob);
	if (FAILED(hr))
		throw;

	fin.read((char*)blob->GetBufferPointer(), size);
	fin.close();

	return blob;
}

void Helper::LoadVertexListFromFile(BasicVertexListInfo* _vInfo, const std::string& _path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(_path + ".fbx", aiProcess_SortByPType | aiProcess_FlipUVs);
	aiMesh* mesh = scene->mMeshes[0];

	_vInfo->verticeData.resize(mesh->mNumVertices);
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		aiVector3D value;
		value = mesh->mVertices[i];
		_vInfo->verticeData[i].position = { value.x, value.y, value.z };

		value = mesh->mTextureCoords[0][i];
		_vInfo->verticeData[i].uv = { value.x, value.y };

		value = mesh->mNormals[i];
		_vInfo->verticeData[i].normal = { value.x, value.y, value.z };
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		for (int h = 0; h < mesh->mFaces[i].mNumIndices; h++)
		{
			_vInfo->indexData.push_back(mesh->mFaces[i].mIndices[h]);
		}
	}

}

bool Helper::GetDisplaySettings(DEVMODEA* _devMode)
{
	std::string name = "";
	DWORD iModeNum = ENUM_CURRENT_SETTINGS;
	return EnumDisplaySettingsA(NULL, iModeNum, _devMode);
}

void Helper::GetWindowDimensions(AppWindow* _window)
{
	RECT desktop;
	HWND desktopHandle = GetDesktopWindow();
	GetWindowRect(desktopHandle, &desktop);
	_window->width = desktop.right;
	_window->height = desktop.bottom;
}

void Helper::GetWindowDimensions(UINT* _width, UINT* _height)
{
	RECT desktop;
	HWND desktopHandle = GetDesktopWindow();
	GetWindowRect(desktopHandle, &desktop);
	*_width = desktop.right;
	*_height = desktop.bottom;
}

void Helper::Print(Vector3 _vec)
{
	printf("[%f] : [%f] : [%f]", _vec.x, _vec.y, _vec.z);
}

void Helper::CreateCommitedResourceStatic(ID3D12Device* _device, ID3D12Resource*& _mainResource, const D3D12_RESOURCE_DESC& _rDesc,
	ID3D12Resource*& _interResource, const D3D12_RESOURCE_DESC& _uDesc,
	CommandList* _cmdList = nullptr, const void* _initData = nullptr, int _rowPitch = 0, int _slicePitch = 0)
{
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	HRESULT hr = _device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE, &_rDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&_mainResource));
	if (FAILED(hr))
		throw;

	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	hr = _device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE, &_uDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&_interResource));
	if (FAILED(hr))
		throw;

	if (_cmdList == nullptr)
		return;

	D3D12_SUBRESOURCE_DATA sData = {};
	sData.pData = _initData;
	sData.RowPitch = _rowPitch;
	sData.SlicePitch = _slicePitch;

	UpdateSubresources(_cmdList->graphic, _mainResource, _interResource, 0, 0, 1, &sData);

}

void Helper::CreateCommitedResourceDynamic(ID3D12Device* _device, ID3D12Resource*& _mainResource, const D3D12_RESOURCE_DESC& _rDesc)
{
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

	HRESULT hr = _device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &_rDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&_mainResource));
	if (FAILED(hr))
		throw;
}

void Helper::LoadVertexDataFromFile(ID3D12Device* _device, CommandList* _cmdList, const std::string& _path, VertexBuffer& _vBuffer)
{
	Helper::BasicVertexListInfo vertexInfo;
	Helper::LoadVertexListFromFile(&vertexInfo, _path);
	_vBuffer.InitStatic(_device, _cmdList, vertexInfo.verticeData.data(), (int)vertexInfo.verticeData.size(), sizeof(BasicVertex));
	_vBuffer.SetNumVertices((int)vertexInfo.verticeData.size());
}
