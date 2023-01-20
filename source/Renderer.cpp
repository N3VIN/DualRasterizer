#include "pch.h"
#include "Renderer.h"

#include "BRDFs.h"
#include "Utils.h"
#include "Vertex.h"
#include "Effect.h"
#include "VehicleEffect.h"
#include "FireEffect.h"

namespace dae
{

	Renderer::Renderer(SDL_Window* pWindow)
	{
		//Initialize
		int width{}, height{};
		SDL_GetWindowSize(pWindow, &width, &height);

		m_pSoftware = new Software{ pWindow, width, height };
		m_pHardware = new Hardware{ pWindow, width, height };

		const auto device = m_pHardware->GetDevice();

		m_AspectRatio = static_cast<float>(width) / static_cast<float>(height);

		// Initialize Camera.
		m_Camera.Initialize(45.f, { 0.0f, 0.0f, -50.f }, m_AspectRatio);

		std::vector<Vertex_In> vertices {};
		std::vector<uint32_t> indices {};

		VehicleEffect* pVehicleEffect = new VehicleEffect{ device, L"Resources/PosCol3D.fx" };
		Utils::ParseOBJ("Resources/vehicle.obj", vertices, indices);

		m_pDiffuseVehicle = new Texture{ "Resources/vehicle_diffuse.png" , device };
		m_pNormalVehicle = new Texture{ "Resources/vehicle_normal.png" , device };
		m_pGlossVehicle = new Texture{ "Resources/vehicle_gloss.png" , device };
		m_pSpecularVehicle = new Texture{ "Resources/vehicle_specular.png" , device };

		m_pVehicleMesh = new Mesh{ device, vertices, indices
			, m_pDiffuseVehicle
			, m_pNormalVehicle
			, m_pGlossVehicle
			, m_pSpecularVehicle
			, pVehicleEffect };

		FireEffect* pFireEffect = new FireEffect{ device, L"Resources/FireShader.fx" };
		Utils::ParseOBJ("Resources/fireFX.obj", vertices, indices);

		m_pDiffuseFire = new Texture{ "Resources/fireFX_diffuse.png" , device };

		m_pFireMesh = new Mesh{ device, vertices, indices
			, m_pDiffuseFire
			, nullptr
			, nullptr
			, nullptr
			, pFireEffect };

		m_pSoftware->SetMesh(m_pVehicleMesh);
		m_pSoftware->SetTextures(m_pDiffuseVehicle, m_pNormalVehicle, m_pGlossVehicle, m_pSpecularVehicle);

		m_pHardware->SetMeshs(m_pVehicleMesh, m_pFireMesh);

	}

	Renderer::~Renderer()
	{
		delete m_pVehicleMesh;
		m_pVehicleMesh = nullptr;

		delete m_pFireMesh;
		m_pFireMesh = nullptr;

		delete m_pDiffuseVehicle;
		m_pDiffuseVehicle = nullptr;

		delete m_pNormalVehicle;
		m_pNormalVehicle = nullptr;

		delete m_pGlossVehicle;
		m_pGlossVehicle = nullptr;

		delete m_pSpecularVehicle;
		m_pSpecularVehicle = nullptr;

		delete m_pDiffuseFire;
		m_pDiffuseFire = nullptr;

		delete m_pSoftware;
		m_pSoftware = nullptr;

		delete m_pHardware;
		m_pHardware = nullptr;

	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_Camera.Update(pTimer);
		m_pVehicleMesh->Update(m_Camera, pTimer);
		m_pFireMesh->Update(m_Camera, pTimer);

		//std::cout << m_Camera.origin.z << std::endl;
	}


	void Renderer::Render() const
	{
		if (m_ToggleRenderModeSoftware)
		{
			m_pSoftware->Render(m_Camera);
		}
		else
		{
			m_pHardware->Render();
		}
	}

	void Renderer::CycleFilteringMode() const
	{
		if (m_ToggleRenderModeSoftware)
		{
			std::cout << "Filtering not Supported in Software mode :(\n";
		}
		else
		{
			m_pHardware->CycleFilteringMode();
		}
	}

	void Renderer::CycleCullMode() const
	{
		m_pHardware->CycleCullMode();
	}

	void Renderer::VisualizeDepthBuffer() const
	{
		if (m_ToggleRenderModeSoftware)
		{
			m_pSoftware->VisualizeDepthBuffer();
		}
		else
		{
			std::cout << "Depth Buffer Visualization not Supported in Hardware mode :(\n";
		}
	}
}
