#pragma once
#include "Mesh.h"
#include "Camera.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Software
	{
	public:

		Software(SDL_Window* pWindow, int width, int height);
		~Software();

		Software(const Software&) = delete;
		Software(Software&&) noexcept = delete;
		Software& operator=(const Software&) = delete;
		Software& operator=(Software&&) noexcept = delete;

		void Render(const Camera& camera) const;
		// addcull mode.

		void CycleShadingMode();

		void SetMesh(Mesh* pMesh);
		void SetTextures(Texture* pDiffuse, Texture* pNormal, Texture* pGloss, Texture* pSpecular);

		void VisualizeDepthBuffer()
		{
			m_DepthBufferVisualized = !m_DepthBufferVisualized;
			std::cout << (m_DepthBufferVisualized ? "Depth Buffer Visualize ON.\n" : "Depth Buffer Visualize OFF.\n");
		}

		void ToggleNormalMap()
		{
			m_ToggleNormalMap = !m_ToggleNormalMap;
			std::cout << (m_ToggleNormalMap ? "Normal Map ON.\n" : "Normal Map OFF.\n");
		}

		void ToggleUniformBg()
		{
			m_UniformBg = !m_UniformBg;
			//std::cout << (m_UniformBg ? "Uniform background ON.\n" : "Uniform background OFF.\n");
		}

	private:

		enum class ShadingModes
		{
			Combined, ObservedArea, Diffuse, Specular
		};

		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		float* m_pDepthBufferPixels{};

		// add cullingmode.

		bool m_DepthBufferVisualized{ false };
		bool m_UniformBg{ false };

		Mesh* m_pVehicleMesh{ nullptr };
		Texture* m_pDiffuseVehicle{ nullptr };
		Texture* m_pNormalVehicle{ nullptr };
		Texture* m_pGlossVehicle{ nullptr };
		Texture* m_pSpecularVehicle{ nullptr };

		ShadingModes m_ShadingMode{ ShadingModes::Combined };

		bool m_ToggleNormalMap{ true };


		//

		void VertexTransformationFunction(std::vector<Mesh*>& mesh, const Camera& camera) const;
		void PixelRenderLoop(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, ColorRGB color) const;
		float ZBufferValue(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const float w0, const float w1, const float w2) const;
		float WInterpolated(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const float w0, const float w1, const float w2) const;
		Vector2 UVInterpolated(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const float w0, const float w1, const float w2, const float wInterpolated) const;
		Vector3 NormalInterpolated(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const float w0, const float w1, const float w2, const float wInterpolated) const;
		Vector3 TangentInterpolated(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const float w0, const float w1, const float w2, const float wInterpolated) const;
		Vector3 ViewDirInterpolated(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const float w0, const float w1, const float w2, const float wInterpolated) const;
		float Remap(float value, float oldRangeL, float oldRangeN, float newRangeL, float newRangeN) const;
		ColorRGB PixelShading(const Vertex_Out& v) const;
		float GetLambertCosine(const Vector3& normal, const Vector3& lightDirection) const;



	};

}
