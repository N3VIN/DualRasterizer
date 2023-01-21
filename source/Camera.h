#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}

		Vector3 origin{};
		float fovAngle{45.f};
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };
		float aspectRatio{};

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{};
		float totalYaw{};

		Matrix invViewMatrix{};
		Matrix viewMatrix{};
		Matrix projectionMatrix{};

		float nearPlane{ 0.1f };
		float farPlane{ 100.f };

		void Initialize(float _fovAngle = 90.f, Vector3 _origin = {0.f,0.f,0.f}, float _aspectRatio = 0.f)
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			origin = _origin;
			aspectRatio = _aspectRatio;

			CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
		}

		void CalculateViewMatrix()
		{
			invViewMatrix = Matrix::CreateLookAtLH(origin, forward, up, right);
			viewMatrix = invViewMatrix.Inverse();
		}

		void CalculateProjectionMatrix()
		{
			projectionMatrix = Matrix::CreatePerspectiveFovLH(fov, aspectRatio, nearPlane, farPlane);

		}

		void Update(const Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Camera Update Logic

			//Keyboard Input
			KeyboardMovement(deltaTime);

			//Mouse Input
			MouseMovement(deltaTime);

			//Update Matrices
			CalculateViewMatrix();
		}

		void KeyboardMovement(float deltaTime)
		{
			float velocity{ 5.f };
			const Uint8* pKeyboardState = SDL_GetKeyboardState(NULL);
			if (pKeyboardState[SDL_SCANCODE_LSHIFT])
			{
				velocity = 15.f;
			}
			if (pKeyboardState[SDL_SCANCODE_W])
			{
				origin += forward * velocity * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_UP])
			{
				origin += forward * velocity * deltaTime;
			}

			if (pKeyboardState[SDL_SCANCODE_S])
			{
				origin -= forward * velocity * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_DOWN])
			{
				origin -= forward * velocity * deltaTime;
			}

			if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin -= right * velocity * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_LEFT])
			{
				origin -= right * velocity * deltaTime;
			}

			if (pKeyboardState[SDL_SCANCODE_D])
			{
				origin += right * velocity * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_RIGHT])
			{
				origin += right * velocity * deltaTime;
			}

			// unreal style up and down.
			if (pKeyboardState[SDL_SCANCODE_Q])
			{
				origin -= up * velocity * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_E])
			{
				origin += up * velocity * deltaTime;
			}

			//// fov change.
			//if (pKeyboardState[SDL_SCANCODE_KP_PLUS])
			//{
			//	degrees++;
			//	updateFovAngle(degrees);
			//}
			//if (pKeyboardState[SDL_SCANCODE_KP_MINUS])
			//{
			//	degrees--;
			//	updateFovAngle(degrees);

			//}
		}

		void MouseMovement(float deltaTime)
		{

			int mouseX, mouseY;
			float sensitivity{ 0.025f };
			auto mouse = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			if (mouse == SDL_BUTTON(3)) // rotate yaw and pitch.
			{
				totalYaw -= mouseY * sensitivity;
				totalPitch += mouseX * sensitivity;

			}
			else if (mouse == SDL_BUTTON(1) && mouseY != 0) // move forward and backward.
			{
				origin += forward * mouseY * sensitivity;
			}
			else if (mouse == SDL_BUTTON(1) && mouseX != 0) // rotate yaw.
			{
				totalPitch -= mouseX * sensitivity;
			}
			// mouse == 5 because SDL_BUTTON is a mask and combining them for 2 buttons.
			else if (mouse == 5 && mouseY > 0) // move down.
			{
				origin -= up * 2.f;
			}
			else if (mouse == 5 && mouseY < 0) // move up.
			{
				origin += up * 2.f;
			}

			Matrix rotationMatrix = Matrix::CreateRotation(totalYaw, totalPitch, 0.f);
			forward = rotationMatrix.TransformVector(Vector3::UnitZ);
			forward.Normalize();

		}

	};
}
