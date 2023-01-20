#include "pch.h"
#include "VehicleEffect.h"

namespace dae
{
	VehicleEffect::VehicleEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
		: Effect(pDevice, assetFile)
	{
		m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
		if (!m_pDiffuseMapVariable->IsValid())
		{
			std::wcout << L"m_pDiffuseMapVariable not valid\n";
		}

		m_pGlossMapVariable = m_pEffect->GetVariableByName("gGlossMap")->AsShaderResource();
		if (!m_pGlossMapVariable->IsValid())
		{
			std::wcout << L"m_pGlossMapVariable not valid\n";
		}

		m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
		if (!m_pNormalMapVariable->IsValid())
		{
			std::wcout << L"m_pNormalMapVariable not valid\n";
		}

		m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
		if (!m_pSpecularMapVariable->IsValid())
		{
			std::wcout << L"m_pSpecularMapVariable not valid\n";
		}
	}

	VehicleEffect::~VehicleEffect()
	{
		if (m_pDiffuseMapVariable)
		{
			m_pDiffuseMapVariable->Release();
		}

		if (m_pNormalMapVariable)
		{
			m_pNormalMapVariable->Release();
		}

		if (m_pSpecularMapVariable)
		{
			m_pSpecularMapVariable->Release();
		}

		if (m_pGlossMapVariable)
		{
			m_pGlossMapVariable->Release();
		}
	}

	void VehicleEffect::SetDiffuseMap(Texture* pDiffuseTexture)
	{
		if (m_pDiffuseMapVariable)
		{
			m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetSRV());
		}
	}

	void VehicleEffect::SetNormalMap(Texture* pNormalTexture)
	{
		if (m_pNormalMapVariable)
		{
			m_pNormalMapVariable->SetResource(pNormalTexture->GetSRV());
		}
	}

	void VehicleEffect::SetSpecularMap(Texture* pSpecularTexture)
	{
		if (m_pSpecularMapVariable)
		{
			m_pSpecularMapVariable->SetResource(pSpecularTexture->GetSRV());
		}
	}

	void VehicleEffect::SetGlossMap(Texture* pGlossTexture)
	{
		if (m_pGlossMapVariable)
		{
			m_pGlossMapVariable->SetResource(pGlossTexture->GetSRV());
		}
	}

}
