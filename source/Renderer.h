#pragma once
#include "Mesh.h"
#include "Camera.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render() const;
		void CycleFilteringMode() const;
		void CycleCullMode();


	private:

		enum class Culling
		{
			None, Back, Front
		};

		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		bool m_IsInitialized{ false };

		Mesh* m_pVehicleMesh;
		Mesh* m_pFireMesh;
		Camera m_Camera{};
		float m_AspectRatio{};

		Culling m_CurrentCullingMode{ Culling::None };

		Texture* m_pDiffuseVehicle;
		Texture* m_pNormalVehicle;
		Texture* m_pGlossVehicle;
		Texture* m_pSpecularVehicle;
		Texture* m_pDiffuseFire;


		void RenderHardware() const;

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
		//...
	};
}
