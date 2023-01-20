#include "pch.h"
#include "Renderer.h"

#include "BRDFs.h"
#include "Utils.h"
#include "Vertex.h"
#include "Effect.h"
#include "VehicleEffect.h"
#include "FireEffect.h"

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		//Initialize DirectX pipeline
		const HRESULT result = InitializeDirectX();
		if (result == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}

		//Create Buffers software.
		m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
		m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
		m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

		m_pDepthBufferPixels = new float[m_Width * m_Height];

		m_AspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);

		// Initialize Camera.
		m_Camera.Initialize(45.f, { 0.0f, 0.0f, -50.f }, m_AspectRatio);

		std::vector<Vertex_In> vertices
		{
			/*{{0.0f, 3.0f , 2.0f}, {1.f, 0.f, 0.f}},
			{{3.0f, -3.0f , 2.0f}, {0.f, 0.f, 1.f}},
			{{-3.0f, -3.0f , 2.0f}, {0.f, 1.f, 0.f}},*/

			/*{{-3, 3, -2}, { }, {0, 0}},
			{{0, 3, -2}, { }, Vector2{0.5, 0}},
			{{3, 3, -2}, { }, Vector2{1, 0}},
			{{-3, 0, -2}, { }, Vector2{0, 0.5}},
			{{0, 0, -2}, { }, Vector2{0.5, 0.5}},
			{{3, 0, -2}, { }, Vector2{1, 0.5}},
			{{-3, -3, -2}, { }, Vector2{0, 1}},
			{{0, -3, -2}, { }, Vector2{0.5, 1}},
			{{3, -3, -2}, { }, Vector2{1, 1}},*/
		};

		//const std::vector<uint32_t> indices{ 0, 1, 2 };

		std::vector<uint32_t> indices
		{
			/*3, 0, 1,	1, 4, 3,	4, 1, 2,
			2, 5, 4,	6, 3, 4,	4, 7, 6,
			7, 4, 5,	5, 8, 7*/
		};

		VehicleEffect* pVehicleEffect = new VehicleEffect{ m_pDevice, L"Resources/PosCol3D.fx" };
		Utils::ParseOBJ("Resources/vehicle.obj", vertices, indices);

		m_pDiffuseVehicle = new Texture{ "Resources/vehicle_diffuse.png" , m_pDevice };
		m_pNormalVehicle = new Texture{ "Resources/vehicle_normal.png" , m_pDevice };
		m_pGlossVehicle = new Texture{ "Resources/vehicle_gloss.png" , m_pDevice };
		m_pSpecularVehicle = new Texture{ "Resources/vehicle_specular.png" , m_pDevice };

		m_pVehicleMesh = new Mesh{ m_pDevice, vertices, indices
			, m_pDiffuseVehicle
			, m_pNormalVehicle
			, m_pGlossVehicle
			, m_pSpecularVehicle
			, pVehicleEffect };

		FireEffect* pFireEffect = new FireEffect{ m_pDevice, L"Resources/FireShader.fx" };
		Utils::ParseOBJ("Resources/fireFX.obj", vertices, indices);

		m_pDiffuseFire = new Texture{ "Resources/fireFX_diffuse.png" , m_pDevice };

		m_pFireMesh = new Mesh{ m_pDevice, vertices, indices
			, m_pDiffuseFire
			, nullptr
			, nullptr
			, nullptr
			, pFireEffect };
	}

	Renderer::~Renderer()
	{
		if (m_pDeviceContext)
		{
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
		}

		if (m_pDevice)
		{
			m_pDevice->Release();
		}

		if (m_pSwapChain)
		{
			m_pSwapChain->Release();
		}

		if (m_pDepthStencilBuffer)
		{
			m_pDepthStencilBuffer->Release();
		}

		if (m_pDepthStencilView)
		{
			m_pDepthStencilView->Release();
		}

		if (m_pRenderTargetBuffer)
		{
			m_pRenderTargetBuffer->Release();
		}

		if (m_pRenderTargetView)
		{
			m_pRenderTargetView->Release();
		}

		if (m_pNoneRasterizerState)
		{
			m_pNoneRasterizerState->Release();
		}

		if (m_pBackRasterizerState)
		{
			m_pBackRasterizerState->Release();
		}

		if (m_pFrontRasterizerState)
		{
			m_pFrontRasterizerState->Release();
		}

		delete[] m_pDepthBufferPixels;
		m_pDepthBufferPixels = nullptr;

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

	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_Camera.Update(pTimer);
		m_pVehicleMesh->Update(m_Camera, pTimer);
		m_pFireMesh->Update(m_Camera, pTimer);
	}


	void Renderer::Render() const
	{
		//RenderHardware();
		RenderSoftware();
	}

	void Renderer::RenderHardware() const
	{
		if (!m_IsInitialized)
			return;

		// Clear RenderTargetView and DepthStencilView

		const ColorRGB clearColor = ColorRGB{ 0.0f, 0.0f, 0.3f };
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		// Set Pipeline + Invoke Drawcalls (=Render)
		//...

		m_pVehicleMesh->Render(m_pDeviceContext);
		m_pFireMesh->Render(m_pDeviceContext);

		// Present Backbuffer (swap)

		m_pSwapChain->Present(0, 0);

	}

	void Renderer::RenderSoftware() const
	{

		//@START
		//Lock BackBuffer
		SDL_LockSurface(m_pBackBuffer);


		// Main Mesh vector.
		std::vector<Mesh*> meshes_world{};
		meshes_world.push_back(m_pVehicleMesh);


		VertexTransformationFunction(meshes_world);

		std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);
		SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));

		//RENDER LOGIC
		ColorRGB finalColor{};
		for (const auto& mesh : meshes_world)
		{
			if (mesh->m_PrimitiveTopology == Mesh::PrimitiveTopology::TriangleList)
			{
				for (size_t i = 0; i < mesh->m_Indices.size(); i += 3)
				{
					Vertex_Out v0{ mesh->m_VerticesOut[mesh->m_Indices[i]] };
					Vertex_Out v1{ mesh->m_VerticesOut[mesh->m_Indices[i + 1]] };
					Vertex_Out v2{ mesh->m_VerticesOut[mesh->m_Indices[i + 2]] };

					// Clipping.
					if (v0.position.x < -1 || v0.position.x > 1 ||
						v0.position.y < -1 || v0.position.y > 1)
					{
						continue;
					}

					if (v1.position.x < -1 || v1.position.x > 1 ||
						v1.position.y < -1 || v1.position.y > 1)
					{
						continue;
					}

					if (v2.position.x < -1 || v2.position.x > 1 ||
						v2.position.y < -1 || v2.position.y > 1)
					{
						continue;
					}

					// NDC to raster.
					v0.position.x = ((v0.position.x + 1) / 2) * m_Width;
					v0.position.y = ((1 - v0.position.y) / 2) * m_Height;

					v1.position.x = ((v1.position.x + 1) / 2) * m_Width;
					v1.position.y = ((1 - v1.position.y) / 2) * m_Height;

					v2.position.x = ((v2.position.x + 1) / 2) * m_Width;
					v2.position.y = ((1 - v2.position.y) / 2) * m_Height;

					PixelRenderLoop(v0, v1, v2, finalColor);
				}
			}
			else if (mesh->m_PrimitiveTopology == Mesh::PrimitiveTopology::TriangleStrip)
			{
				for (size_t i = 0; i < mesh->m_Indices.size() - 2; i++)
				{
					uint32_t index1{ mesh->m_Indices[i] };
					uint32_t index2{ mesh->m_Indices[i + 1] };
					uint32_t index3{ mesh->m_Indices[i + 2] };

					if (index3 % 2 == 1)
					{
						index2 = mesh->m_Indices[i + 2];
						index3 = mesh->m_Indices[i + 1];
					}

					Vertex_Out v0{ mesh->m_VerticesOut[index1] };
					Vertex_Out v1{ mesh->m_VerticesOut[index2] };
					Vertex_Out v2{ mesh->m_VerticesOut[index3] };

					PixelRenderLoop(v0, v1, v2, finalColor);
				}
			}
		}
		//@END
		//Update SDL Surface
		SDL_UnlockSurface(m_pBackBuffer);
		SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
		SDL_UpdateWindowSurface(m_pWindow);
	}

	void Renderer::VertexTransformationFunction(std::vector<Mesh*>& mesh) const
	{
		for (auto& m : mesh)
		{
			const auto worldViewProjectionMatrix{ m->m_WorldMatrix * m_Camera.viewMatrix * m_Camera.projectionMatrix };

			for (const auto& vertices : m->m_VerticesIn)
			{
				// Projection.
				Vector4 projectedVertex{ worldViewProjectionMatrix.TransformPoint(vertices.position.x, vertices.position.y, vertices.position.z, 1) };

				// World space normal calculation.
				Vector3 worldSpaceNormal{ m->m_WorldMatrix.TransformVector(vertices.normal).Normalized() };

				// Tangent calculation.
				Vector3 tangent{ m->m_WorldMatrix.TransformVector(vertices.tangent).Normalized() };

				// View Direction Calculation.
				//Vector3 viewDirection{ m.worldMatrix.TransformPoint(vertices.position) - m_Camera.origin };
				Vector3 viewDirection{ m_Camera.origin - m->m_WorldMatrix.TransformPoint(vertices.position) };
				//Vector3 viewDirection{ vertices.position };

				// Perspective Divide.
				projectedVertex.x /= projectedVertex.w;
				projectedVertex.y /= projectedVertex.w;
				projectedVertex.z /= projectedVertex.w;

				//// Clipping.
				//if ( projectedVertex.x < -1 || projectedVertex.x > 1 ||
				//	 projectedVertex.y < -1 || projectedVertex.y > 1 )
				//{
				//	continue;
				//}

				//// NDC to screenspace.
				//projectedVertex.x = ((projectedVertex.x + 1) / 2) * m_Width;
				//projectedVertex.y = ((1 - projectedVertex.y) / 2) * m_Height;

				//Vertex_Out temp{ projectedVertex, {vertices.color}, {vertices.uv}, {worldSpaceNormal}, {tangent}, {viewDirection} };
				Vertex_Out temp{ projectedVertex, vertices.color, vertices.uv, worldSpaceNormal, tangent, viewDirection };
				m->m_VerticesOut.emplace_back(temp);
			}
		}
	}

	void dae::Renderer::PixelRenderLoop(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, ColorRGB color) const
	{
		// Bounding Box.
		Vector3 min{}, max{};
		min = Vector3::Min(v0.position, Vector3::Min(v1.position, v2.position));
		max = Vector3::Max(v0.position, Vector3::Max(v1.position, v2.position));

		Vector3 margin{ 1.f, 1.f, 1.f };
		min -= margin;
		max += margin;

		min = Vector3::Max({ 0, 0, 0 }, Vector3::Min({ static_cast<float>(m_Width - 1), static_cast<float>(m_Height - 1), 0.f }, min));
		max = Vector3::Max({ 0, 0, 0 }, Vector3::Min({ static_cast<float>(m_Width - 1), static_cast<float>(m_Height - 1), 0.f }, max));

		for (int px{ static_cast<int>(min.x) }; px < static_cast<int>(max.x); ++px)
		{
			for (int py{ static_cast<int>(min.y) }; py < static_cast<int>(max.y); ++py)
			{
				Vector2 pixel{ static_cast<float>(px), static_cast<float>(py) };

				//edges
				const Vector2 v0v1{ v1.position.GetXY() - v0.position.GetXY() };
				const Vector2 v1v2{ v2.position.GetXY() - v1.position.GetXY() };
				const Vector2 v2v0{ v0.position.GetXY() - v2.position.GetXY() };

				const Vector2 v0v2{ v2.position.GetXY() - v0.position.GetXY() };

				// Vector from vertex to pixel.
				Vector2 vertexToPixel1{ pixel - v0.position.GetXY() };
				Vector2 vertexToPixel2{ pixel - v1.position.GetXY() };
				Vector2 vertexToPixel3{ pixel - v2.position.GetXY() };

				//Cross of vertex to pixel and vertex
				float signedArea1{ Vector2::Cross(v0v1, vertexToPixel1) };
				float signedArea2{ Vector2::Cross(v1v2, vertexToPixel2) };
				float signedArea3{ Vector2::Cross(v2v0, vertexToPixel3) };

				// Pixel inside triangle.
				if (signedArea1 > 0 && signedArea2 > 0 && signedArea3 > 0)
				{
					float areaTotalParallelogram{ Vector2::Cross(v0v1, v0v2) };

					float W0{ signedArea2 / areaTotalParallelogram };
					float W1{ signedArea3 / areaTotalParallelogram };
					float W2{ signedArea1 / areaTotalParallelogram };

					//Vector3 pixelPosition{ (v0.position * W0) + (v1.position * W1) + (v2.position * W2) };

					float zBufferValue{ ZBufferValue(v0, v1, v2, W0, W1, W2) };

					float depth = m_pDepthBufferPixels[py * m_Width + px];
					//if (pixelPosition.z < depth)
					if (zBufferValue < depth)
					{
						m_pDepthBufferPixels[py * m_Width + px] = zBufferValue;
						//Vector2 uv = { (v0.uv * W0) + (v1.uv * W1) + (v2.uv * W2) };
						float wInterpolated{ WInterpolated(v0, v1, v2, W0, W1, W2) };
						Vector2 uv{ UVInterpolated(v0, v1, v2, W0, W1, W2, wInterpolated) };
						Vector3 normal{ NormalInterpolated(v0, v1, v2, W0, W1, W2, wInterpolated).Normalized() };
						Vector3 tangent{ TangentInterpolated(v0, v1, v2, W0, W1, W2, wInterpolated).Normalized() };
						Vector3 viewDirection{ ViewDirInterpolated(v0, v1, v2, W0, W1, W2, wInterpolated).Normalized() };

						ColorRGB finalColor{};

						if (!m_DepthBufferVisualized)
						{
							//finalColor = m_pTexture->Sample(uv);

							const Vector4 pixel{ static_cast<float>(px), static_cast<float>(py), zBufferValue, wInterpolated };

							Vertex_Out pixelVertex{ pixel, finalColor, uv, normal, tangent, viewDirection };

							//finalColor += PixelShading(pixelVertex);
							finalColor = PixelShading(pixelVertex);
						}
						else
						{
							finalColor = ColorRGB{ 1, 1, 1 } *Remap(zBufferValue, 0.985f, 1.f, 0.f, 1.f);
						}

						color = finalColor;

						//Update Color in Buffer
						color.MaxToOne();

						m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
							static_cast<uint8_t>(color.r * 255),
							static_cast<uint8_t>(color.g * 255),
							static_cast<uint8_t>(color.b * 255));
					}
				}
			}
		}
	}

	float dae::Renderer::ZBufferValue(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const float w0, const float w1, const float w2) const
	{
		float denominator = (1.0f / v0.position.z) * w0 + (1.0f / v1.position.z) * w1 + (1.0f / v2.position.z) * w2;
		return (1 / denominator);
	}

	float dae::Renderer::WInterpolated(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const float w0, const float w1, const float w2) const
	{
		float denominator = (1.0f / v0.position.w) * w0 + (1.0f / v1.position.w) * w1 + (1.0f / v2.position.w) * w2;
		return (1 / denominator);
	}

	Vector2 dae::Renderer::UVInterpolated(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const float w0, const float w1, const float w2, const float wInterpolated) const
	{
		return ((v0.uv / v0.position.w) * w0 + (v1.uv / v1.position.w) * w1 + (v2.uv / v2.position.w) * w2) * wInterpolated;
	}

	Vector3 dae::Renderer::NormalInterpolated(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const float w0, const float w1, const float w2, const float wInterpolated) const
	{
		return ((v0.normal / v0.position.w) * w0 + (v1.normal / v1.position.w) * w1 + (v2.normal / v2.position.w) * w2) * wInterpolated;
	}

	Vector3 dae::Renderer::TangentInterpolated(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const float w0, const float w1, const float w2, const float wInterpolated) const
	{
		return ((v0.tangent / v0.position.w) * w0 + (v1.tangent / v1.position.w) * w1 + (v2.tangent / v2.position.w) * w2) * wInterpolated;
	}

	Vector3 dae::Renderer::ViewDirInterpolated(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const float w0, const float w1, const float w2, const float wInterpolated) const
	{
		return ((v0.viewDirection / v0.position.w) * w0 + (v1.viewDirection / v1.position.w) * w1 + (v2.viewDirection / v2.position.w) * w2) * wInterpolated;
	}

	float dae::Renderer::Remap(float value, float oldRangeL, float oldRangeN, float newRangeL, float newRangeN) const
	{
		float newVal{ std::clamp(value, oldRangeL, oldRangeN) };
		return newRangeL + (newVal - oldRangeL) * (newRangeN - newRangeL) / (newRangeN - oldRangeL);
	}

	ColorRGB dae::Renderer::PixelShading(const Vertex_Out& v) const
	{
		Vector3 lightDirection{ 0.577f, -0.577f, 0.577f };
		float lightIntensity{ 7.0f };
		const ColorRGB ambient{ 0.025f, 0.025f, 0.025f };

		//// Calculate Binormal.
		Vector3 binormal{ Vector3::Cross(v.normal, v.tangent).Normalized() };

		//// Calculate Tangent space Matrix.
		Matrix tangentSpaceMatrix{ v.tangent, binormal, v.normal, Vector3::Zero };

		//// Calculate Normal according to the Normal Map.
		ColorRGB normalMapCol{ (2 * m_pNormalVehicle->Sample(v.uv)) - colors::White };
		//ColorRGB normalMapCol{ ( m_pNormalMap->Sample(v.uv)) };
		Vector3 normalMapVector{ normalMapCol.r, normalMapCol.g, normalMapCol.b };
		//Vector3 normalMapVector{ (normalMapCol.r * 2.0f) - 1.0f, (normalMapCol.g * 2.0f) - 1.0f, (normalMapCol.b * 2.0f) - 1.0f };
		normalMapVector /= 255.f;
		Vector3 tangentSpaceVector = tangentSpaceMatrix.TransformVector(normalMapVector).Normalized();
		//v.normal = tangentSpaceMatrix.TransformVector(normalMapVector).Normalized();

		float lambertCosineLaw{ GetLambertCosine(tangentSpaceVector, lightDirection) };

		if (lambertCosineLaw < 0.f)
		{
			return { 0,0,0 };
		}

		float specularShininess{ 25.f };
		float specularExp{ specularShininess * m_pGlossVehicle->Sample(v.uv).r };
		ColorRGB specular{ BRDF::Phong(m_pSpecularVehicle->Sample(v.uv), 1.f, specularExp, lightDirection, v.viewDirection, tangentSpaceVector) };

		ColorRGB lambert{ BRDF::Lambert(1.0f, m_pDiffuseVehicle->Sample(v.uv)) };
		//ColorRGB lambert{ BRDF::Lambert(1.0f, {0.5,0.5,0.5}) };



		//return (lambert * lambertCosineLaw * lightIntensity); // lambert final.
		//return specular; // specular final.
		return ((lambert * lightIntensity) + specular) * lambertCosineLaw; // combined.

		//return (lambert * lightIntensity) * lambertCosineLaw;
		//return lambertCosineLaw * ((lightIntensity * lambert) + specular + ambient);
		//return ColorRGB{1, 1, 1} * lambertCosineLaw;
	}

	float Renderer::GetLambertCosine(const dae::Vector3& normal, const dae::Vector3& lightDirection) const
	{
		float lambertCosine{};
		//lambertCosine = std::max(Vector3::Dot(normal, -lightDirection.Normalized()), 0.0f);
		lambertCosine = std::max(0.f, Vector3::Dot(normal, -lightDirection.Normalized()));
		return lambertCosine;
	}

	void Renderer::CycleFilteringMode() const
	{
		m_pVehicleMesh->CycleFilteringMode();
		m_pFireMesh->CycleFilteringMode();
	}

	void Renderer::CycleCullMode()
	{
		int count{ static_cast<int>(m_CurrentCullingMode) };
		count++;
		if (count > 2)
		{
			count = 0;
		}
		const Culling castEnum = static_cast<Culling>(count);
		m_CurrentCullingMode = castEnum;

		switch (m_CurrentCullingMode)
		{
		case Culling::None:
			m_pDeviceContext->RSSetState(m_pNoneRasterizerState);
			std::cout << "None\n";
			break;
		case Culling::Back:
			m_pDeviceContext->RSSetState(m_pBackRasterizerState);
			std::cout << "Back\n";
			break;
		case Culling::Front:
			m_pDeviceContext->RSSetState(m_pFrontRasterizerState);
			std::cout << "Front\n";
			break;
		}
	}

	

	HRESULT Renderer::InitializeDirectX()
	{
		// Creating Device and DeviceContext.

		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
		uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &featureLevel, 1, D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext);

		if (FAILED(result))
		{
			return result;
		}

		// Create DXGI Factory.

		IDXGIFactory1* pDXGIFactory{};
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDXGIFactory));
		
		if (FAILED(result))
		{
			return result;
		}

		// Create Swapchain discriptor.

		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		// Get the handle (HWND) from the SDL Backbuffer.

		SDL_SysWMinfo sysWMInfo{};
		SDL_VERSION(&sysWMInfo.version);
		SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

		// Create Swapchain.

		result = pDXGIFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
		if (FAILED(result))
		{
			return result;
		}

		// Create DepthStencil & DepthStencilView.		
		//Resource
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		// View
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilViewDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
		if (FAILED(result))
		{
			return result;
		}

		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
		if (FAILED(result))
		{
			return result;
		}

		// Create RenderTarget & RenderTargetView.
		// Resource
		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
		if (FAILED(result))
		{
			return result;
		}

		// View
		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
		if (FAILED(result))
		{
			return result;
		}

		// Bind RenderTargetView and DepthStencilView to Output Merger Stage.

		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		// Rasterizer States.

		// None Culling.
		D3D11_RASTERIZER_DESC descNone{};
		descNone.FillMode = D3D11_FILL_SOLID;
		descNone.CullMode = D3D11_CULL_NONE;
		descNone.FrontCounterClockwise = FALSE;
		descNone.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
		descNone.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
		descNone.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		descNone.DepthClipEnable = TRUE;
		descNone.ScissorEnable = FALSE;
		descNone.MultisampleEnable = FALSE;
		descNone.AntialiasedLineEnable = FALSE;

		result = m_pDevice->CreateRasterizerState(&descNone, &m_pNoneRasterizerState);
		if (FAILED(result))
		{
			return result;
		}

		// Back Culling.
		D3D11_RASTERIZER_DESC descBack{};
		descBack.FillMode = D3D11_FILL_SOLID;
		descBack.CullMode = D3D11_CULL_BACK;
		descBack.FrontCounterClockwise = FALSE;
		descBack.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
		descBack.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
		descBack.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		descBack.DepthClipEnable = TRUE;
		descBack.ScissorEnable = FALSE;
		descBack.MultisampleEnable = FALSE;
		descBack.AntialiasedLineEnable = FALSE;

		result = m_pDevice->CreateRasterizerState(&descBack, &m_pBackRasterizerState);
		if (FAILED(result))
		{
			return result;
		}

		// Front Culling.
		D3D11_RASTERIZER_DESC descFront{};
		descFront.FillMode = D3D11_FILL_SOLID;
		descFront.CullMode = D3D11_CULL_FRONT;
		descFront.FrontCounterClockwise = FALSE;
		descFront.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
		descFront.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
		descFront.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		descFront.DepthClipEnable = TRUE;
		descFront.ScissorEnable = FALSE;
		descFront.MultisampleEnable = FALSE;
		descFront.AntialiasedLineEnable = FALSE;

		result = m_pDevice->CreateRasterizerState(&descFront, &m_pFrontRasterizerState);
		if (FAILED(result))
		{
			return result;
		}

		// Set None culling as default.
		m_pDeviceContext->RSSetState(m_pNoneRasterizerState);

		// Set Viewport.

		D3D11_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(m_Width);
		viewport.Height = static_cast<float>(m_Height);
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		m_pDeviceContext->RSSetViewports(1, &viewport);

		return result;

	}
	
}
