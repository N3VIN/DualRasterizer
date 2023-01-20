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

		void VisualizeDepthBuffer()
		{
			m_DepthBufferVisualized = !m_DepthBufferVisualized;
		}

		void SwitchRenderMode()
		{
			m_ToggleRenderModeSoftware = !m_ToggleRenderModeSoftware;
		}

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

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		float* m_pDepthBufferPixels{};

		Culling m_CurrentCullingMode{ Culling::None };

		Texture* m_pDiffuseVehicle;
		Texture* m_pNormalVehicle;
		Texture* m_pGlossVehicle;
		Texture* m_pSpecularVehicle;
		Texture* m_pDiffuseFire;

		bool m_DepthBufferVisualized{ false };
		bool m_ToggleRenderModeSoftware{ false };



		void RenderHardware() const;
		void RenderSoftware() const;

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

		// Software Functions
		void VertexTransformationFunction(std::vector<Mesh*>& mesh) const; //W1 Version
		void PixelRenderLoop(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, ColorRGB color) const;
		float ZBufferValue(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const float w0, const float w1, const float w2) const;
		float WInterpolated(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const float w0, const float w1, const float w2) const;
		Vector2 UVInterpolated(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const float w0, const float w1, const float w2, const float wInterpolated) const;
		Vector3 NormalInterpolated(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const float w0, const float w1, const float w2, const float wInterpolated) const;
		Vector3 TangentInterpolated(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const float w0, const float w1, const float w2, const float wInterpolated) const;
		Vector3 ViewDirInterpolated(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const float w0, const float w1, const float w2, const float wInterpolated) const;
		float Remap(float value, float oldRangeL, float oldRangeN, float newRangeL, float newRangeN) const;
		ColorRGB PixelShading(const Vertex_Out& v) const;
		float GetLambertCosine(const dae::Vector3& normal, const dae::Vector3& lightDirection) const;



	};
}
