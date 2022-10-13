#pragma once
#include "ShaderResource.h"

class Texture2D
{
public:
	Texture2D();
	Texture2D(ID3D12Device* _device, ShaderDescriptorHeap* _heap, CommandList* _cmdList, const std::string& _path);
	Texture2D(ID3D12Device* _device, HiddenDescriptorHeap* _heap, CommandList* _cmdList, const std::string& _path);
	~Texture2D();

	void Init(ID3D12Device* _device, ShaderDescriptorHeap* _heap, CommandList* _cmdList, const std::string& _path);
	void Init(ID3D12Device* _device, HiddenDescriptorHeap* _heap, CommandList* _cmdList, const std::string& _path);
	void SetResource(ShaderResource* _sResource);

	ShaderResource* sResource;
};

