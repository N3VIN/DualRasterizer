#pragma once
#include "Effect.h"

namespace dae
{
	class FireEffect : public Effect
	{
	public:

		FireEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
		~FireEffect() override;

		FireEffect(const FireEffect&) = delete;
		FireEffect(FireEffect&&) noexcept = delete;
		FireEffect& operator=(const FireEffect&) = delete;
		FireEffect& operator=(FireEffect&&) noexcept = delete;

		void SetDiffuseMap(Texture* pDiffuseTexture) override;
		void SetNormalMap(Texture* pNormalTexture) override {}
		void SetSpecularMap(Texture* pSpecularTexture) override {}
		void SetGlossMap(Texture* pGlossTexture) override {}

		void SetLight(Lights* pLight) override {}

	private:

		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;

	};
}