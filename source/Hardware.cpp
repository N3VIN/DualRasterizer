#include "pch.h"
#include "Hardware.h"

#include "Effect.h"


namespace dae
{
	Hardware::Hardware(SDL_Window* pWindow, int width, int height)
		: m_pWindow(pWindow)
		, m_Width(width)
		, m_Height(height)
	{
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



	}

	Hardware::~Hardware()
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

	}

	void Hardware::Render() const
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

		if (m_ToggleFireMesh)
		{
			m_pFireMesh->Render(m_pDeviceContext);
		}

		// Present Backbuffer (swap)

		m_pSwapChain->Present(0, 0);
	}

	void Hardware::CycleFilteringMode() const
	{
		m_pVehicleMesh->CycleFilteringMode();
		m_pFireMesh->CycleFilteringMode();
	}

	void Hardware::CycleCullMode()
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

	HRESULT Hardware::InitializeDirectX()
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

	void Hardware::SetMeshs(Mesh* pMesh1, Mesh* pMesh2)
	{
		m_pVehicleMesh = pMesh1;
		m_pFireMesh = pMesh2;
	}
}
