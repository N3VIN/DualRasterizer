#pragma once
#include "Mesh.h"
#include "Utils.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Hardware
	{
	public:

		Hardware(SDL_Window* pWindow, int width, int height);
		~Hardware();

		Hardware(const Hardware&) = delete;
		Hardware(Hardware&&) noexcept = delete;
		Hardware& operator=(const Hardware&) = delete;
		Hardware& operator=(Hardware&&) noexcept = delete;

		void Render() const;

		void SetMeshs(Mesh* pMesh1, Mesh* pMesh2);

		void CycleFilteringMode() const;
		void CycleCullMode();

		ID3D11Device* GetDevice() const
		{
			return m_pDevice;
		}

		void ToggleFireMesh()
		{
			m_ToggleFireMesh = !m_ToggleFireMesh;
			std::cout << (m_ToggleFireMesh ? "Toggle Fire Mesh ON.\n" : "Toggle Fire Mesh OFF.\n");
		}

		void ToggleUniformBg()
		{
			m_UniformBg = !m_UniformBg;
			std::cout << (m_UniformBg ? "Uniform background ON.\n" : "Uniform background OFF.\n");
		}

	private:

		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		bool m_IsInitialized{ false };

		Mesh* m_pVehicleMesh;
		Mesh* m_pFireMesh;

		Culling m_CurrentCullingMode{ Culling::Back };

		bool m_ToggleFireMesh{ true };
		bool m_UniformBg{ false };


		//DIRECTX Variables
		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;
		IDXGISwapChain* m_pSwapChain;
		ID3D11Texture2D* m_pDepthStencilBuffer;
		ID3D11DepthStencilView* m_pDepthStencilView;
		ID3D11Texture2D* m_pRenderTargetBuffer;
		ID3D11RenderTargetView* m_pRenderTargetView;

		ID3D11RasterizerState* m_pNoneRasterizerState;
		ID3D11RasterizerState* m_pBackRasterizerState;
		ID3D11RasterizerState* m_pFrontRasterizerState;


		//DIRECTX Functions
		HRESULT InitializeDirectX();

	};

}
