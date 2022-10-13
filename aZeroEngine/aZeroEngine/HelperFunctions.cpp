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

void Helper::GetWindowDimensions(UINT& _width, UINT& _height)
{
	RECT desktop;
	HWND desktopHandle = GetDesktopWindow();
	GetWindowRect(desktopHandle, &desktop);
	_width = desktop.right;
	_height = desktop.bottom;
}

void Helper::Print(Vector3 _vec)
{
	printf("[%f] : [%f] : [%f]", _vec.x, _vec.y, _vec.z);
}
