#include "pch.h"

#if defined(_DEBUG)
#include "vld.h"
#endif

#undef main
#include "Renderer.h"

using namespace dae;

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;

	SDL_Window* pWindow = SDL_CreateWindow(
		"DirectX - *** Nevin ***",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	const auto pTimer = new Timer();
	const auto pRenderer = new Renderer(pWindow);

	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;
	bool isDisplayFPS = false;
	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				switch (e.key.keysym.sym)
				{
				case SDLK_F1:
					pRenderer->SwitchRenderMode();
					break;
				case SDLK_F2:
					pRenderer->ToggleRotation();
					break;
				case SDLK_F3:
					pRenderer->ToggleFireMesh();
					break;
				case SDLK_F4:
					pRenderer->CycleFilteringMode();
					break;
				case SDLK_F5:
					pRenderer->CycleShadingMode();
					break;
				case SDLK_F6:
					pRenderer->ToggleNormalMap();
					break;
				case SDLK_F7:
					pRenderer->VisualizeDepthBuffer();
					break;
				case SDLK_F8:
					pRenderer->ToggleBoundingBox();
					break;
				case SDLK_F9:
					pRenderer->CycleCullMode();
					break;
				case SDLK_F10:
					pRenderer->ToggleUniformBg();
					break;
				case SDLK_F11:
					isDisplayFPS = !isDisplayFPS;
					std::cout << (isDisplayFPS ? "Toggle Print FPS ON.\n" : "Toggle Print FPS OFF.\n");
					break;
				}
				break;
			default: ;
			}
		}

		//--------- Update ---------
		pRenderer->Update(pTimer);

		//--------- Render ---------
		pRenderer->Render();

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			if (isDisplayFPS)
			{
				std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
			}
		}
	}
	pTimer->Stop();

	//Shutdown "framework"
	delete pRenderer;
	delete pTimer;

	ShutDown(pWindow);
	return 0;
}