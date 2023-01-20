#pragma once
#include <string>
#include "Texture.h"

namespace dae
{
	class Effect
	{
	public:

		Effect(ID3D11Device* pDevice, const std::wstring& assetFile);
		virtual ~Effect();

		Effect(const Effect&) = delete;
		Effect(Effect&&) noexcept = delete;
		Effect& operator=(const Effect&) = delete;
		Effect& operator=(Effect&&) noexcept = delete;

		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);


		ID3DX11Effect* GetEffect();
		ID3DX11EffectTechnique* GetTechnique();
		void SetMatWorldViewProjVariable(const Matrix& worldViewProjectionMatrix);
		void SetMatWorldVariable(const Matrix& worldMatrix);
		void SetMatInvViewVariable(const Matrix& invViewMatrix);

		void SetEffectSamplerVariable(ID3D11SamplerState* samplerState);

		virtual void SetDiffuseMap(Texture* pDiffuseTexture) = 0;
		virtual void SetNormalMap(Texture* pNormalTexture) = 0;
		virtual void SetSpecularMap(Texture* pSpecularTexture) = 0;
		virtual void SetGlossMap(Texture* pGlossTexture) = 0;

		void CycleFilteringMode();

	protected:

		enum class Filtering
		{
			Point, Linear, Anisotropic
		};

		Filtering m_CurrentFilteringMode{ Filtering::Point };

		ID3DX11Effect* m_pEffect;
		ID3DX11EffectTechnique* m_pTechnique;

		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable;
		ID3DX11EffectMatrixVariable* m_pMatMatWorldVariable;
		ID3DX11EffectMatrixVariable* m_pMatInvViewVariable;

		ID3DX11EffectSamplerVariable* m_pEffectSamplerVariable;

		ID3D11SamplerState* m_pPointSamplerState;
		ID3D11SamplerState* m_pLinearSamplerState;
		ID3D11SamplerState* m_pAnisotropicSamplerState;

	};
}