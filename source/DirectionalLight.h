#pragma once
#include "Lights.h"

namespace dae
{
	class DirectionalLight : public Lights
	{
	public:
		DirectionalLight(const Vector3& direction, float lightIntensity);
		~DirectionalLight() = default;

		DirectionalLight(const DirectionalLight&) = default;
		DirectionalLight(DirectionalLight&&) noexcept = default;
		DirectionalLight& operator=(const DirectionalLight&) = default;
		DirectionalLight& operator=(DirectionalLight&&) noexcept = default;

		virtual Vector3 GetDirection() const override;
		virtual float GetlightIntensity() const override;

	private:
		Vector3 m_Direction;
		float m_LightIntensity;

	};
}

