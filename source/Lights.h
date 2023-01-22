#pragma once
#include "Math.h"

namespace dae
{
	class Lights
	{
	public:
		Lights() = default;
		~Lights() = default;

		virtual Vector3 GetDirection() const = 0;
		virtual float GetlightIntensity() const = 0;

	};
}
