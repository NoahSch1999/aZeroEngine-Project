#pragma once
#include "D3D12Include.h"
#include <vector>

class Texture;

class RenderSurfaces
{
private:
	std::vector<Texture*> m_texturesResize;
	std::vector<Texture*> m_texturesResizeMSAA;

public:
	RenderSurfaces() = default;

	void addTextureResize(Texture& texture)
	{
		m_texturesResize.emplace_back(&texture);
	}

	void addTextureResizeMSAA(Texture& texture)
	{
		m_texturesResizeMSAA.emplace_back(&texture);
	}

	void recreateTextures(ID3D12Device* device, UINT width, UINT height, UINT sampleCount)
	{
		for (Texture* const texture : m_texturesResizeMSAA)
		{
			Texture::Description desc = texture->getDescription();
			desc.m_width = width;
			desc.m_height = height;
			desc.m_sampleCount = sampleCount;
			texture->recreate(device, desc);
		}

		for (Texture* const texture : m_texturesResize)
		{
			Texture::Description desc = texture->getDescription();
			desc.m_width = width;
			desc.m_height = height;
			texture->recreate(device, desc);
		}
	}
};