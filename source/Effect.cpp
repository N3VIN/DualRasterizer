#include "pch.h"
#include "Effect.h"

namespace dae
{
	Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile)
	{
		m_pEffect = LoadEffect(pDevice, assetFile);

		m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
		if (!m_pTechnique->IsValid())
		{
			std::wcout << L"Technique not valid\n";
		}

		m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
		if (!m_pMatWorldViewProjVariable->IsValid())
		{
			std::wcout << L"m_pMatWorldViewProjVariable not valid\n";
		}

		m_pMatMatWorldVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
		if (!m_pMatMatWorldVariable->IsValid())
		{
			std::wcout << L"m_pMatMatWorldVariable not valid\n";
		}

		m_pMatInvViewVariable = m_pEffect->GetVariableByName("gViewInvMatrix")->AsMatrix();
		if (!m_pMatInvViewVariable->IsValid())
		{
			std::wcout << L"m_pMatInvViewVariable not valid\n";
		}

		m_pEffectSamplerVariable = m_pEffect->GetVariableByName("gSampleState")->AsSampler();
		if (!m_pEffectSamplerVariable->IsValid())
		{
			std::wcout << L"m_pEffectSamplerVariable not valid" << '\n';
		}

		// Sampler States.
		D3D11_SAMPLER_DESC descPoint{};
		descPoint.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		descPoint.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		descPoint.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		descPoint.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		descPoint.ComparisonFunc = D3D11_COMPARISON_NEVER;
		descPoint.MipLODBias = 0;
		descPoint.MinLOD = 0;
		descPoint.MaxLOD = D3D11_FLOAT32_MAX;
		descPoint.MaxAnisotropy = 16;

		HRESULT hr = pDevice->CreateSamplerState(&descPoint, &m_pPointSamplerState);

		D3D11_SAMPLER_DESC descLinear{};
		descLinear.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		descLinear.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		descLinear.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		descLinear.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		descLinear.ComparisonFunc = D3D11_COMPARISON_NEVER;
		descLinear.MipLODBias = 0;
		descLinear.MinLOD = 0;
		descLinear.MaxLOD = D3D11_FLOAT32_MAX;
		descLinear.MaxAnisotropy = 16;

		hr = pDevice->CreateSamplerState(&descLinear, &m_pLinearSamplerState);

		D3D11_SAMPLER_DESC descAnisotropic{};
		descAnisotropic.Filter = D3D11_FILTER_ANISOTROPIC;
		descAnisotropic.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		descAnisotropic.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		descAnisotropic.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		descAnisotropic.ComparisonFunc = D3D11_COMPARISON_NEVER;
		descAnisotropic.MipLODBias = 0;
		descAnisotropic.MinLOD = 0;
		descAnisotropic.MaxLOD = D3D11_FLOAT32_MAX;
		descAnisotropic.MaxAnisotropy = 16;

		hr = pDevice->CreateSamplerState(&descAnisotropic, &m_pAnisotropicSamplerState);

	}
	

	Effect::~Effect()
	{
		if (m_pTechnique)
		{
			m_pTechnique->Release();
		}

		if (m_pEffect)
		{
			m_pEffect->Release();
		}

		if (m_pPointSamplerState)
		{
			m_pPointSamplerState->Release();
		}

		if (m_pLinearSamplerState)
		{
			m_pLinearSamplerState->Release();
		}

		if (m_pAnisotropicSamplerState)
		{
			m_pAnisotropicSamplerState->Release();
		}
	}


	ID3DX11Effect* Effect::GetEffect() const
	{
		return m_pEffect;
	}

	ID3DX11EffectTechnique* Effect::GetTechnique() const
	{
		return m_pTechnique;
	}

	void Effect::SetMatWorldViewProjVariable(const Matrix& worldViewProjectionMatrix) const
	{
		m_pMatWorldViewProjVariable->SetMatrix(reinterpret_cast<const float*>(&worldViewProjectionMatrix));
	}

	void Effect::SetMatWorldVariable(const Matrix& worldMatrix) const
	{
		m_pMatMatWorldVariable->SetMatrix(reinterpret_cast<const float*>(&worldMatrix));
	}

	void Effect::SetMatInvViewVariable(const Matrix& invViewMatrix) const
	{
		m_pMatInvViewVariable->SetMatrix(reinterpret_cast<const float*>(&invViewMatrix));
	}

	void Effect::SetEffectSamplerVariable(ID3D11SamplerState* samplerState) const
	{
		HRESULT hr = m_pEffectSamplerVariable->SetSampler(0, samplerState);
	}

	ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	{
		HRESULT result;
		ID3D10Blob* pErrorBlob{ nullptr };
		ID3DX11Effect* pEffect;

		DWORD shaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		shaderFlags |= D3DCOMPILE_DEBUG;
		shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;

#endif 
		result = D3DX11CompileEffectFromFile(assetFile.c_str(),
			nullptr,
			nullptr,
			shaderFlags,
			0,
			pDevice,
			&pEffect,
			&pErrorBlob);

		if (FAILED(result))
		{
			if (pErrorBlob != nullptr)
			{
				const char* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

				std::wstringstream ss;
				for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); i++)
				{
					ss << pErrors[i];
				}

				OutputDebugStringW(ss.str().c_str());
				pErrorBlob->Release();
				pErrorBlob = nullptr;

				std::wcout << ss.str() << std::endl;
			}
			else
			{
				std::wstringstream ss;
				ss << "Effectloader: Failed to CreateEffectFromFile!\nPath: " << assetFile;
				std::wcout << ss.str() << std::endl;

				return nullptr;
			}
		}

		return pEffect;
	}

	void Effect::CycleFilteringMode()
	{
		int count{ static_cast<int>(m_CurrentFilteringMode) };
		count++;
		if (count > 2)
		{
			count = 0;
		}
		const auto castEnum = static_cast<Filtering>(count);
		m_CurrentFilteringMode = castEnum;

		switch (m_CurrentFilteringMode)
		{
		case Filtering::Point:

			SetEffectSamplerVariable(m_pPointSamplerState);

			std::cout << "Filtering Mode: Point\n";
			break;
		case Filtering::Linear:

			SetEffectSamplerVariable(m_pLinearSamplerState);

			std::cout << "Filtering Mode: Linear\n";
			break;
		case Filtering::Anisotropic:

			SetEffectSamplerVariable(m_pAnisotropicSamplerState);

			std::cout << "Filtering Mode: Anisotropic\n";
			break;
		}
	}
}
