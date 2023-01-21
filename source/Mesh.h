#pragma once
#include <vector>
#include "Vertex.h"
#include "Effect.h"
#include "Camera.h"

namespace dae
{
	class Mesh
	{
	public:
		Mesh(ID3D11Device* pDevice, const std::vector<Vertex_In>& vertex, const std::vector<uint32_t>& index
			, Texture* diffuse
			, Texture* normal
			, Texture* gloss
			, Texture* specular
			, Effect* effect);
		~Mesh();

		Mesh(const Mesh&) = delete;
		Mesh(Mesh&&) noexcept = delete;
		Mesh& operator=(const Mesh&) = delete;
		Mesh& operator=(Mesh&&) noexcept = delete;

		void Render(ID3D11DeviceContext* pDeviceContext) const;
		void Update(const Camera& camera, const Timer* pTimer);
		void CycleFilteringMode() const;

		void RotateY(float angle);
		void RotateX(float angle);
		void RotateZ(float angle);
		void Translate(float x, float y, float z);
		void Translate(const Vector3& v);

		void ToggleRotation()
		{
			m_ToggleRotation = !m_ToggleRotation;
			std::cout << (m_ToggleRotation ? "Rotation ON.\n" : "Rotation OFF.\n");
		}

		Matrix m_WorldMatrix;

		// Software.
		enum class PrimitiveTopology
		{
			TriangleList,
			TriangleStrip
		};

		std::vector<Vertex_In> m_VerticesIn{};
		std::vector<Vertex_Out> m_VerticesOut{};
		std::vector<uint32_t> m_Indices{};
		PrimitiveTopology m_PrimitiveTopology{ PrimitiveTopology::TriangleList };

	private:

		


		// DX.
		Effect* m_pEffect;
		ID3D11InputLayout* m_pInputLayout;
		ID3D11Buffer* m_pVertexBuffer;
		ID3D11Buffer* m_pIndexBuffer;
		uint32_t m_NumIndices;


		bool m_ToggleRotation{ true };

	};
}