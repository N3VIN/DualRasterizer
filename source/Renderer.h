#pragma once
#include "Mesh.h"
#include "Camera.h"
#include "Hardware.h"
#include "Software.h"

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
		void VisualizeDepthBuffer() const;
		void CycleShadingMode() const;
		void ToggleNormalMap() const;

		void SwitchRenderMode()
		{
			m_ToggleRenderModeSoftware = !m_ToggleRenderModeSoftware;
			std::cout << (m_ToggleRenderModeSoftware ? "Render Mode: Software.\n" : "Render Mode: Hardware.\n");
		}
		void ToggleFireMesh() const;

		void CycleCullMode() const
		{
			m_pHardware->CycleCullMode();
			m_pSoftware->CycleCullMode();
		}

		void ToggleRotation() const
		{
			m_pVehicleMesh->ToggleRotation();
			m_pFireMesh->ToggleRotation();
		}

		void ToggleUniformBg() const
		{
			m_pHardware->ToggleUniformBg();
			m_pSoftware->ToggleUniformBg();
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
