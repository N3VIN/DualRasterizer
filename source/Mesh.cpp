#include "pch.h"
#include "Mesh.h"

namespace dae
{
	Mesh::Mesh(ID3D11Device* pDevice, const std::vector<Vertex_In>& vertex, const std::vector<uint32_t>& index
		, Texture* diffuse
		, Texture* normal
		, Texture* gloss
		, Texture* specular
		, Effect* effect)
	{
		m_pEffect = effect;

		// Create Vertex Layout.
		static constexpr uint32_t numElements{ 5 };
		D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

		vertexDesc[0].SemanticName = "POSITION";
		vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[0].AlignedByteOffset = 0;
		vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[1].SemanticName = "COLOR";
		vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[1].AlignedByteOffset = 12;
		vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[2].SemanticName = "TEXCOORD";
		vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[2].AlignedByteOffset = 24;
		vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[3].SemanticName = "NORMAL";
		vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[3].AlignedByteOffset = 32;
		vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[4].SemanticName = "TANGENT";
		vertexDesc[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[4].AlignedByteOffset = 40;
		vertexDesc[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		// Create Input Layout.
		D3DX11_PASS_DESC passDesc{};
		m_pEffect->GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);

		const HRESULT result = pDevice->CreateInputLayout(
			vertexDesc,
			numElements,
			passDesc.pIAInputSignature,
			passDesc.IAInputSignatureSize,
			&m_pInputLayout);

		if (FAILED(result))
		{
			assert(false);
		}

		// Create Vertex Buffer.
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(Vertex_In) * static_cast<uint32_t>(vertex.size());
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = vertex.data();

		HRESULT resultVB = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
		if (FAILED(resultVB))
		{
			return;
		}

		// Create Index Buffer.
		m_NumIndices = static_cast<uint32_t>(index.size());
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		initData.pSysMem = index.data();

		HRESULT resultIB = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
		if (FAILED(resultIB))
		{
			return;
		}

		// Set Textures.
		m_pEffect->SetDiffuseMap(diffuse);
		m_pEffect->SetNormalMap(normal);
		m_pEffect->SetGlossMap(gloss);
		m_pEffect->SetSpecularMap(specular);

	}

	Mesh::~Mesh()
	{
		if (m_pIndexBuffer)
		{
			m_pIndexBuffer->Release();
		}

		if (m_pVertexBuffer)
		{
			m_pVertexBuffer->Release();
		}

		if (m_pInputLayout)
		{
			m_pInputLayout->Release();
		}

		delete m_pEffect;
		m_pEffect = nullptr;

	}

	void Mesh::Render(ID3D11DeviceContext* pDeviceContext) const
	{
		// Set Primitive Topology.
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Set Input Layout.
		pDeviceContext->IASetInputLayout(m_pInputLayout);

		// Set Vertex Buffer.
		constexpr UINT stride = sizeof(Vertex_In);
		constexpr UINT offset = 0;
		pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

		// Set Index Buffer.
		pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Draw.
		D3DX11_TECHNIQUE_DESC techDesc{};
		m_pEffect->GetTechnique()->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; p++)
		{
			m_pEffect->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
			pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
		}
		
	}

	void Mesh::Update(const Camera& camera, const Timer* pTimer)
	{
		RotateY(90.0f * pTimer->GetElapsed());

		const Matrix worldViewProjectionMat = m_WorldMatrix * camera.viewMatrix * camera.projectionMatrix;
		m_pEffect->SetMatWorldViewProjVariable(worldViewProjectionMat);

		m_pEffect->SetMatWorldVariable(m_WorldMatrix);

		m_pEffect->SetMatInvViewVariable(camera.invViewMatrix);
	}

	void Mesh::CycleFilteringMode() const
	{
		m_pEffect->CycleFilteringMode();
	}

	void Mesh::CycleCullMode() const
	{

	}

	void Mesh::RotateY(float angle)
	{
		m_WorldMatrix = Matrix::CreateRotationY(angle * TO_RADIANS) * m_WorldMatrix;
	}

	void Mesh::RotateX(float angle)
	{
		m_WorldMatrix = Matrix::CreateRotationX(angle * TO_RADIANS) * m_WorldMatrix;
	}

	void Mesh::RotateZ(float angle)
	{
		m_WorldMatrix = Matrix::CreateRotationZ(angle * TO_RADIANS) * m_WorldMatrix;
	}

	void Mesh::Translate(float x, float y, float z)
	{
		m_WorldMatrix = Matrix::CreateTranslation(x, y, z) * m_WorldMatrix;
	}

	void Mesh::Translate(const Vector3& v)
	{
		m_WorldMatrix = Matrix::CreateTranslation(v) * m_WorldMatrix;
	}

}
