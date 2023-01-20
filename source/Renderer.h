#pragma once
#include "Mesh.h"
#include "Camera.h"
#include "Hardware.h"
#include "Software.h"
#include "Utils.h"

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
		void CycleCullMode() const;

		void VisualizeDepthBuffer() const;

		void SwitchRenderMode()
		{
			m_ToggleRenderModeSoftware = !m_ToggleRenderModeSoftware;
		}

	private:

		Software* m_pSoftware;
		Hardware* m_pHardware;

		Mesh* m_pVehicleMesh;
		Mesh* m_pFireMesh;
		Camera m_Camera{};
		float m_AspectRatio{};

		Texture* m_pDiffuseVehicle;
		Texture* m_pNormalVehicle;
		Texture* m_pGlossVehicle;
		Texture* m_pSpecularVehicle;
		Texture* m_pDiffuseFire;

		bool m_ToggleRenderModeSoftware{ false };


	};
}
