#pragma once
#include <cassert>
#include "Math.h"
#include <algorithm>


namespace dae
{
	namespace BRDF
	{
		/**
		 * \param kd Diffuse Reflection Coefficient
		 * \param cd Diffuse Color
		 * \return Lambert Diffuse Color
		 */
		static ColorRGB Lambert(float kd, const ColorRGB& cd)
		{
			ColorRGB lambert{ (cd * kd) / float(PI) };
			return lambert;
		}

		static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
		{
			// not sure.
			ColorRGB lambert{ (cd * kd) / float(PI) };
			return lambert;
		}

		/**
		 * \brief todo
		 * \param ks Specular Reflection Coefficient
		 * \param exp Phong Exponent
		 * \param l Incoming (incident) Light Direction
		 * \param v View Direction
		 * \param n Normal of the Surface
		 * \return Phong Specular Color
		 */
		static ColorRGB Phong(const ColorRGB specularColor, float ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n)
		{
			Vector3 reflect{ Vector3::Reflect(l, n) };
			//Vector3 reflect{ l - (2 * (Vector3::Dot(n, l)) * n)};
			float cosAlpha{ std::max(0.f, Vector3::Dot(reflect, v)) };
			float phong{ ks * (pow(cosAlpha, exp)) };
			//return ColorRGB(phong, phong, phong);
			return specularColor * phong;
		}

	}
}