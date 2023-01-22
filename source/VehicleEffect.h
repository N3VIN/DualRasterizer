#pragma once
#include "Effect.h"

namespace dae
{
	class VehicleEffect : public Effect
	{
	public:

		VehicleEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
		~VehicleEffect() override;

		VehicleEffect(const VehicleEffect&) = delete;
		VehicleEffect(VehicleEffect&&) noexcept = delete;
		VehicleEffect& operator=(const VehicleEffect&) = delete;
		VehicleEffect& operator=(VehicleEffect&&) noexcept = delete;

		void SetDiffuseMap(Texture* pDiffuseTexture) override;
		void SetNormalMap(Texture* pNormalTexture) override;
		void SetSpecularMap(Texture* pSpecularTexture) override;
		void SetGlossMap(Texture* pGlossTexture) override;

		void SetLight(Lights* pLight) override;

	private:

		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
		ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable;
		ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable;
		ID3DX11EffectShaderResourceVariable* m_pGlossMapVariable;

		ID3DX11EffectVectorVariable* m_pLightDirection;
		ID3DX11EffectScalarVariable* m_pLightIntensity;

	};
}