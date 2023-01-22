#pragma once
#include "Lights.h"
#include <vector>

namespace dae
{
	class LightManager
	{
	public:
		static LightManager& GetInstance()
		{
			static LightManager instance{};
			return instance;
		}
		~LightManager();
		LightManager(const LightManager&) = default;
		LightManager(LightManager&&) noexcept = default;
		LightManager& operator=(const LightManager&) = default;
		LightManager& operator=(LightManager&&) noexcept = default;

		void add(Lights* pObject);
		std::vector<Lights*> GetLights() const;

	private:
		LightManager() = default;
		std::vector<Lights*> m_pLightContainer;

	};
}
