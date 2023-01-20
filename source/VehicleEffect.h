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

	private:

		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
		ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable;
		ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable;
		ID3DX11EffectShaderResourceVariable* m_pGlossMapVariable;

	};
}