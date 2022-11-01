#include "Texture2D.h"

Texture2D::Texture2D()
{
}

Texture2D::Texture2D(ID3D12Device* _device, ShaderDescriptorHeap* _heap, CommandList* _cmdList, const std::string& _path)
{
	Init(_device, _heap, _cmdList, _path);
}

Texture2D::~Texture2D()
{
	// cleanup of shader resource handled via textureCache
	delete sResource;
}

void Texture2D::Init(ID3D12Device* _device, ShaderDescriptorHeap* _heap, CommandList* _cmdList, const std::string& _path)
{
	sResource = new ShaderResource();
	sResource->InitAsTextureAndDescriptor(_device, _heap, _cmdList, _path);
}

void Texture2D::SetResource(ShaderResource* _sResource)
{
	sResource = _sResource;
}
