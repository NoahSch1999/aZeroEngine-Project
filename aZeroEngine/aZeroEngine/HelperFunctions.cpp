#include "pch.h"
#include "HelperFunctions.h"
#include <fstream>
#include "..\assimp\include\assimp\scene.h"
#include "..\assimp\include\assimp\Importer.hpp"
#include "..\assimp\include\assimp\postprocess.h"
#include "stb_image.h"

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
	const aiScene* scene = importer.ReadFile(_path + ".fbx", aiProcess_SortByPType | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
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
		value.Normalize();

		value = mesh->mTangents[i];
		value.Normalize();
		_vInfo->verticeData[i].tangent = { value.x, value.y, value.z };
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

//void Helper::GetWindowDimensions(AppWindow* _window)
//{
//	RECT desktop;
//	HWND desktopHandle = GetDesktopWindow();
//	GetWindowRect(desktopHandle, &desktop);
//	_window->width = desktop.right;
//	_window->height = desktop.bottom;
//}

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

	UpdateSubresources(_cmdList->GetGraphicList(), _mainResource, _interResource, 0, 0, 1, &sData);

}

void Helper::CreateCommitedResourceDynamic(ID3D12Device* _device, ID3D12Resource*& _mainResource, const D3D12_RESOURCE_DESC& _rDesc)
{
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

	HRESULT hr = _device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &_rDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&_mainResource));
	if (FAILED(hr))
		throw;
}

Helper::STBIImageData Helper::LoadSTBIImage(const std::string& _fileName)
{
	STBIImageData data;
	data.rawData = stbi_load(_fileName.c_str(), &data.width, &data.height, nullptr, STBI_rgb_alpha);
	data.channels = STBI_rgb_alpha;
	return data;
}

bool Helper::OpenFileDialogForExtension(const std::string& _extension, std::string& _storeFileStr)
{
	CHAR ogPath[MAX_PATH];
	DWORD dw = GetCurrentDirectoryA(MAX_PATH, ogPath);

	OPENFILENAMEA openDialog = { 0 };
	CHAR filePath[260] = { 0 };
	ZeroMemory(&openDialog, sizeof(openDialog));
	openDialog.lStructSize = sizeof(openDialog);
	openDialog.hwndOwner = NULL;
	openDialog.lpstrInitialDir = LPCSTR(ogPath);
	openDialog.lpstrFile = filePath;
	openDialog.nMaxFile = MAX_PATH;
	openDialog.lpstrTitle = "Select a file";
	openDialog.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;
	GetOpenFileNameA(&openDialog);

	const std::string selectedFilePath(filePath);

	if (selectedFilePath.ends_with(_extension))
	{
		std::string fileameWithExt = selectedFilePath.substr(selectedFilePath.find_last_of("/\\") + 1);
		_storeFileStr = fileameWithExt;
		return true;
	}
	return false;
}

Microsoft::WRL::ComPtr<ID3D12Resource> Helper::CreateReadbackBuffer(ID3D12Device* _device, int _rowPitch, int _numRows)
{
	D3D12_HEAP_PROPERTIES readbackHeapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK) };
	//int rowPitch = ((_rowPitch + 128) / 256) * 256;
	//int rowPitch = (_rowPitch + 256) & ~256;
	int totalSize = _rowPitch * _numRows;
	D3D12_RESOURCE_DESC readbackBufferDesc{ CD3DX12_RESOURCE_DESC::Buffer(totalSize) };
	Microsoft::WRL::ComPtr<ID3D12Resource> readbackBuffer;

	_device->CreateCommittedResource(
		&readbackHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&readbackBufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(readbackBuffer.GetAddressOf()));

	return readbackBuffer;
}

Microsoft::WRL::ComPtr<ID3D12Resource> Helper::CreateBufferResource(ID3D12Device* _device, int _width, D3D12_HEAP_TYPE _heapType)
{
	D3D12_RESOURCE_DESC rDesc = {};
	ZeroMemory(&rDesc, sizeof(rDesc));
	rDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	rDesc.Width = _width;
	rDesc.Height = 1;
	rDesc.DepthOrArraySize = 1;
	rDesc.MipLevels = 1;
	rDesc.Format = DXGI_FORMAT_UNKNOWN;
	rDesc.SampleDesc.Count = 1;
	rDesc.SampleDesc.Quality = 0;
	rDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	rDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = _heapType;

	Microsoft::WRL::ComPtr<ID3D12Resource> retResource;

	D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
	if (_heapType == D3D12_HEAP_TYPE_UPLOAD)
		state = D3D12_RESOURCE_STATE_GENERIC_READ;

	HRESULT hr = _device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE, &rDesc, state, nullptr, IID_PPV_ARGS(&retResource));
	if (FAILED(hr))
		throw;

	return retResource;
}

void Helper::CreateBufferResource(ID3D12Device* _device, CommandList& _cmdList, Microsoft::WRL::ComPtr<ID3D12Resource>& _gpuOnlyResource, int _gpuOnlyWidth, Microsoft::WRL::ComPtr<ID3D12Resource>& _mappedResource, int _mappedWidth, void* _data)
{
	_gpuOnlyResource = Helper::CreateBufferResource(_device, _gpuOnlyWidth, D3D12_HEAP_TYPE_DEFAULT);
	_mappedResource = Helper::CreateBufferResource(_device, _mappedWidth, D3D12_HEAP_TYPE_UPLOAD);

	D3D12_SUBRESOURCE_DATA sData = {};
	sData.pData = _data;
	sData.RowPitch = _gpuOnlyWidth;
	sData.SlicePitch = _gpuOnlyWidth;

	UpdateSubresources(_cmdList.GetGraphicList(), _gpuOnlyResource.Get(), _mappedResource.Get(), 0, 0, 1, &sData);
}

Microsoft::WRL::ComPtr<ID3D12Resource> Helper::CreateTextureResource(ID3D12Device* _device, D3D12_RESOURCE_DESC& _desc, int _width, int _height, DXGI_FORMAT _format, D3D12_RESOURCE_FLAGS _flags, D3D12_HEAP_TYPE _heapType, D3D12_CLEAR_VALUE _clearValue, D3D12_RESOURCE_STATES& _initialState)
{
	D3D12_RESOURCE_DESC rDesc = {};
	rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rDesc.Width = _width;
	rDesc.Height = _height;
	rDesc.DepthOrArraySize = 1;
	rDesc.MipLevels = 1;
	rDesc.Format = _format;
	rDesc.SampleDesc.Count = 1;
	rDesc.SampleDesc.Quality = 0;
	rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rDesc.Flags = _flags;

	D3D12_HEAP_PROPERTIES properties = {};
	properties.Type = D3D12_HEAP_TYPE_DEFAULT;
	if (_heapType == D3D12_HEAP_TYPE_UPLOAD)
		properties.Type = D3D12_HEAP_TYPE_UPLOAD;

	_clearValue.Format = _format;

	if (_flags == D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
		_initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	else if (_flags == D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
		_initialState = D3D12_RESOURCE_STATE_RENDER_TARGET;

	Microsoft::WRL::ComPtr<ID3D12Resource> retResource = nullptr;
	HRESULT hr = _device->CreateCommittedResource(&properties, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&rDesc, _initialState, &_clearValue, IID_PPV_ARGS(&retResource));
	if (FAILED(hr))
		throw;

	_desc = rDesc;

	return retResource;
}

void Helper::CreateTextureResource(ID3D12Device* _device, CommandList& _transitionList, CommandList& _copyList, Microsoft::WRL::ComPtr<ID3D12Resource>& _gpuOnlyResource,
	Microsoft::WRL::ComPtr<ID3D12Resource>& _intermediateResource, void* _data, int _width, int _height, int _channels, DXGI_FORMAT _format, D3D12_RESOURCE_STATES _initState)
{
	D3D12_RESOURCE_DESC rDesc;
	ZeroMemory(&rDesc, sizeof(D3D12_RESOURCE_DESC));
	rDesc.MipLevels = 1;
	rDesc.Format = _format;
	rDesc.Width = _width;
	rDesc.Height = _height;
	rDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	rDesc.DepthOrArraySize = 1;
	rDesc.SampleDesc.Count = 1;
	rDesc.SampleDesc.Quality = 0;
	rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rDesc.Alignment = 0;

	D3D12_RESOURCE_ALLOCATION_INFO allocInfo = _device->GetResourceAllocationInfo(0, 1, &rDesc);

	CD3DX12_RESOURCE_DESC uDesc = CD3DX12_RESOURCE_DESC::Buffer(allocInfo.SizeInBytes);

	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	HRESULT hr = _device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE, &rDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&_gpuOnlyResource));
	if (FAILED(hr))
		throw;

	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

	hr = _device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE, &uDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&_intermediateResource));
	if (FAILED(hr))
		throw;

	D3D12_SUBRESOURCE_DATA sData = {};
	sData.pData = _data;
	sData.RowPitch = _width * _channels;
	sData.SlicePitch = _width * _channels * _height;

	UpdateSubresources(_copyList.GetGraphicList(), _gpuOnlyResource.Get(), _intermediateResource.Get(), 0, 0, 1, &sData);

	D3D12_RESOURCE_BARRIER barrier(CD3DX12_RESOURCE_BARRIER::Transition(_gpuOnlyResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, _initState));
	_transitionList.GetGraphicList()->ResourceBarrier(1, &barrier);
}

void Helper::CreateRTVHandle(ID3D12Device* _device, Microsoft::WRL::ComPtr<ID3D12Resource> _resource, D3D12_CPU_DESCRIPTOR_HANDLE _cpuHandle, DXGI_FORMAT _format)
{
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	ZeroMemory(&rtvDesc, sizeof(D3D12_RENDER_TARGET_VIEW_DESC));
	rtvDesc.Format = _format;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	_device->CreateRenderTargetView(_resource.Get(), &rtvDesc, _cpuHandle);
}

void Helper::CreateDSVHandle(ID3D12Device* _device, Microsoft::WRL::ComPtr<ID3D12Resource> _resource, D3D12_CPU_DESCRIPTOR_HANDLE _cpuHandle, DXGI_FORMAT _format)
{
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	dsvDesc.Format = _format;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	_device->CreateDepthStencilView(_resource.Get(), &dsvDesc, _cpuHandle);
}

void Helper::CreateSRVHandle(ID3D12Device* _device, Microsoft::WRL::ComPtr<ID3D12Resource> _resource, D3D12_CPU_DESCRIPTOR_HANDLE _cpuHandle, DXGI_FORMAT _format)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = _format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.f;
	_device->CreateShaderResourceView(_resource.Get(), &srvDesc, _cpuHandle);
}
