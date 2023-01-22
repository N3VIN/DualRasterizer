#include "pch.h"
#include "Renderer.h"

#include "BRDFs.h"
#include "Utils.h"
#include "Vertex.h"
#include "Effect.h"
#include "VehicleEffect.h"
#include "FireEffect.h"
#include "LightManager.h"
#include "DirectionalLight.h"


namespace dae
{
	Renderer::Renderer(SDL_Window* pWindow)
	{
		//Initialize
		int width{}, height{};
		SDL_GetWindowSize(pWindow, &width, &height);

		m_pSoftware = new Software{ pWindow, width, height };
		m_pHardware = new Hardware{ pWindow, width, height };

		Keybindings();

		const auto device = m_pHardware->GetDevice();

		m_AspectRatio = static_cast<float>(width) / static_cast<float>(height);

		// Initialize Camera.
		m_Camera.Initialize(45.f, { 0.0f, 0.0f, 0.f }, m_AspectRatio);

		LightManager::GetInstance().add(new DirectionalLight{ {0.577f, -0.577f, 0.577f}, 7.0f });

		const auto light = LightManager::GetInstance().GetLights().at(0); // Since we are only adding only one light.
		m_pSoftware->SetLight(light);

		std::vector<Vertex_In> vertices{};
		std::vector<uint32_t> indices{};

		VehicleEffect* pVehicleEffect = new VehicleEffect{ device, L"Resources/PosCol3D.fx" };
		Utils::ParseOBJ("Resources/vehicle.obj", vertices, indices);

		pVehicleEffect->SetLight(light);

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

		m_pVehicleMesh->Translate(0.f, 0.f, 50.f);
		m_pFireMesh->Translate(0.f, 0.f, 50.f);

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

		if (m_ToggleRotation)
		{
			m_pVehicleMesh->RotateY(45.0f * pTimer->GetElapsed());
			m_pFireMesh->RotateY(45.0f * pTimer->GetElapsed());
		}
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

	void Renderer::CycleShadingMode() const
	{
		if (m_ToggleRenderModeSoftware)
		{
			m_pSoftware->CycleShadingMode();
		}
		else
		{
			std::cout << "Cycle Shading Modes not Supported in Hardware mode :(\n";
		}
	}

	void Renderer::ToggleNormalMap() const
	{
		if (m_ToggleRenderModeSoftware)
		{
			m_pSoftware->ToggleNormalMap();
		}
		else
		{
			std::cout << "Toggle Normal Map not Supported in Hardware mode :(\n";
		}
	}

	void Renderer::ToggleBoundingBox() const
	{
		if (m_ToggleRenderModeSoftware)
		{
			m_pSoftware->ToggleBoundingBox();
		}
		else
		{
			std::cout << "Bounding Box Visualization not Supported in Hardware mode :(\n";
		}
	}

	void Renderer::SwitchRenderMode()
	{
		m_ToggleRenderModeSoftware = !m_ToggleRenderModeSoftware;
		std::cout << (m_ToggleRenderModeSoftware ? "Render Mode: Software.\n" : "Render Mode: Hardware.\n");
	}

	void Renderer::ToggleFireMesh() const
	{
		if (m_ToggleRenderModeSoftware)
		{
			std::cout << "Toggle Fire Mesh not Supported in Software mode :(\n";
		}
		else
		{
			m_pHardware->ToggleFireMesh();
		}
	}

	void Renderer::CycleCullMode() const
	{
		m_pHardware->CycleCullMode();
		m_pSoftware->CycleCullMode();
	}

	void Renderer::ToggleRotation()
	{
		m_ToggleRotation = !m_ToggleRotation;
		std::cout << (m_ToggleRotation ? "Rotation ON.\n" : "Rotation OFF.\n");
	}

	void Renderer::ToggleUniformBg() const
	{
		m_pHardware->ToggleUniformBg();
		m_pSoftware->ToggleUniformBg();
	}

	void Renderer::Keybindings() const
	{
		std::cout << "[Key Bindings - SHARED]\n";
		std::cout << "[F1] Switch Render Mode (Hardware / Software).\n";
		std::cout << "[F2] Toggle Vehicle Rotation (ON / OFF).\n";
		std::cout << "[F9] Cycle CullMode (Back / None / Front).\n";
		std::cout << "[F10] Toggle Uniform ClearColor (ON / OFF).\n";
		std::cout << "[F11] Toggle Print FPS (ON / OFF).\n";
		std::cout << "\n";

		std::cout << "[Key Bindings - HARDWARE]\n";
		std::cout << "[F3] Toggle Fire Mesh (ON / OFF).\n";
		std::cout << "[F4] Cycle Filtering Mode (Point / Linear / Anisotropic).\n";
		std::cout << "\n";

		std::cout << "[Key Bindings - SOFTWARE]\n";
		std::cout << "[F5] Switch Shading Mode (Combined / Observed Area / Diffuse / Specular).\n";
		std::cout << "[F6] Toggle NormalMap (ON / OFF).\n";
		std::cout << "[F7] Toggle Depth Buffer Visualization (ON / OFF).\n";
		std::cout << "[F8] Toggle Bounding Box Visualization (ON / OFF).\n";
		std::cout << "\n";

		std::cout << "[Features Added]\n";
		std::cout << "Multithreading.\n";
		std::cout << "Directional light and Manager.\n";
		std::cout << "[E] Local Up.\n";
		std::cout << "[Q] Local Down.\n";
		std::cout << "\n";

	}
}
