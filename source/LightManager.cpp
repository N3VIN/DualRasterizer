#include "pch.h"
#include "LightManager.h"

namespace dae
{
	LightManager::~LightManager()
	{
		for (auto objects : m_pLightContainer)
		{
			delete objects;
			objects = nullptr;
		}
		m_pLightContainer.clear();
	}

	void LightManager::add(Lights* pObject)
	{
		m_pLightContainer.push_back(pObject);
	}

	std::vector<Lights*> LightManager::GetLights() const
	{
		return m_pLightContainer;
	}
}