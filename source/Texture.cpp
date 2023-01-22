#include "pch.h"
#include "Texture.h"
#include "Vector2.h"
#include <algorithm>
#include <SDL_image.h>

namespace dae
{
	Texture::Texture(const std::string& path, ID3D11Device* pDevice)
	{
		SDL_Surface* pSurface = IMG_Load(path.c_str());
		m_pSurface = pSurface;
		m_pSurfacePixels = static_cast<uint32_t*>(pSurface->pixels);

		// Create Texture2D Resource.
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = m_pSurface->w;
		desc.Height = m_pSurface->h;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = m_pSurface->pixels;
		initData.SysMemPitch = static_cast<UINT>(m_pSurface->pitch);
		initData.SysMemSlicePitch = static_cast<UINT>(m_pSurface->h * m_pSurface->pitch);

		HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &m_pResource);

		// Create Texture2D Resource View.
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
		SRVDesc.Format = format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		hr = pDevice->CreateShaderResourceView(m_pResource, &SRVDesc, &m_pSRV);

	}

	Texture::~Texture()
	{
		m_pSRV->Release();
		m_pResource->Release();

		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}
	}


	ID3D11ShaderResourceView* Texture::GetSRV() const
	{
		return m_pSRV;
	}

	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		Uint8 r{}, g{}, b{};

		Vector2 UV;

		// Wrapping.
		const float integerPartX{ static_cast<float>(abs(static_cast<int>(uv.x))) };
		const float integerPartY{ static_cast<float>(abs(static_cast<int>(uv.y))) };

		if (uv.x > 1.f)
		{
			UV.x = uv.x - integerPartX;
		}
		else if (uv.x < 0.f)
		{
			UV.x = uv.x + integerPartX;
		}
		else
		{
			UV.x = uv.x;
		}

		if (uv.y > 1.f)
		{
			UV.y = uv.y - integerPartY;
		}
		else if (uv.y < 0.f)
		{
			UV.y = uv.y + integerPartY;
		}
		else
		{
			UV.y = uv.y;
		}

		const int x{ static_cast<int>(UV.x * m_pSurface->w) };
		const int y{ static_cast<int>(UV.y * m_pSurface->h) };

		const Uint32 pixel{ m_pSurfacePixels[x + y * m_pSurface->w] };

		SDL_GetRGB(pixel, m_pSurface->format, &r, &g, &b);

		constexpr float remap{ 1 / 255.f };
		return { r * remap, g * remap, b * remap };
	}
}
