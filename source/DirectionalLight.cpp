#include "pch.h"
#include "DirectionalLight.h"

namespace dae
{

	DirectionalLight::DirectionalLight(const Vector3& direction, float lightIntensity)
		: m_Direction{ direction.Normalized() }
		, m_LightIntensity{ lightIntensity }
	{
	}

	Vector3 DirectionalLight::GetDirection() const
	{
		return m_Direction;
	}

	float DirectionalLight::GetlightIntensity() const
	{
		return m_LightIntensity;
	}
}
