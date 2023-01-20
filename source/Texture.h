#pragma once
#include <SDL_surface.h>
#include <string>
#include "ColorRGB.h"

namespace dae
{
	struct Vector2;

	class Texture
	{
	public:
		Texture(const std::string& path, ID3D11Device* pDevice);
		~Texture();

		// DX
		ID3D11ShaderResourceView* GetSRV() const;

		// Software.
		ColorRGB Sample(const Vector2& uv) const;

	private:

		// DX.
		ID3D11Texture2D* m_pResource;
		ID3D11ShaderResourceView* m_pSRV;

		// Software.

		SDL_Surface* m_pSurface{ nullptr };
		uint32_t* m_pSurfacePixels{ nullptr };
	};
}