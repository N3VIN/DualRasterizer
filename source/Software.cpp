#include "pch.h"
#include "Software.h"
#include <ppl.h> // parallel_for

#include <array>

#include "BRDFs.h"
#include "Vertex.h"

namespace dae
{
	Software::Software(SDL_Window* pWindow, int width, int height)
		: m_pWindow(pWindow)
		, m_Width(width)
		, m_Height(height)
	{
		//Create Buffers software.
		m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
		m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
		m_pBackBufferPixels = static_cast<uint32_t*>(m_pBackBuffer->pixels);

		m_pDepthBufferPixels = new float[m_Width * m_Height];

	}

	Software::~Software()
	{
		delete[] m_pDepthBufferPixels;
		m_pDepthBufferPixels = nullptr;
	}

	void Software::Render(const Camera& camera) const
	{
		//@START
		//Lock BackBuffer
		SDL_LockSurface(m_pBackBuffer);


		// Main Mesh vector.
		std::vector<Mesh*> meshes_world{};
		meshes_world.emplace_back(m_pVehicleMesh);

		VertexTransformationFunction(meshes_world, camera);

		std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);

		UINT8 color;
		m_UniformBg ? color = 25 : color = 100;
		SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, color, color, color));

		//RENDER LOGIC
		for (const auto& mesh : meshes_world)
		{
			if (mesh->m_PrimitiveTopology == Mesh::PrimitiveTopology::TriangleList)
			{
				concurrency::parallel_for(static_cast<size_t>(0), mesh->m_Indices.size(), static_cast<size_t>(3), [=](const size_t i)
				{
					Vertex_Out v0{ mesh->m_VerticesOut[mesh->m_Indices[i]] };
					Vertex_Out v1{ mesh->m_VerticesOut[mesh->m_Indices[i + 1]] };
					Vertex_Out v2{ mesh->m_VerticesOut[mesh->m_Indices[i + 2]] };

					// Clipping.
					if (v0.position.x < -1 || v0.position.x > 1 ||
						v0.position.y < -1 || v0.position.y > 1)
					{
						return;
					}

					if (v1.position.x < -1 || v1.position.x > 1 ||
						v1.position.y < -1 || v1.position.y > 1)
					{
						return;
					}

					if (v2.position.x < -1 || v2.position.x > 1 ||
						v2.position.y < -1 || v2.position.y > 1)
					{
						return;
					}

					// NDC to raster.
					v0.position.x = ((v0.position.x + 1) / 2) * static_cast<float>(m_Width);
					v0.position.y = ((1 - v0.position.y) / 2) * static_cast<float>(m_Height);

					v1.position.x = ((v1.position.x + 1) / 2) * static_cast<float>(m_Width);
					v1.position.y = ((1 - v1.position.y) / 2) * static_cast<float>(m_Height);

					v2.position.x = ((v2.position.x + 1) / 2) * static_cast<float>(m_Width);
					v2.position.y = ((1 - v2.position.y) / 2) * static_cast<float>(m_Height);

					PixelRenderLoop(v0, v1, v2);
				});

			}
			else if (mesh->m_PrimitiveTopology == Mesh::PrimitiveTopology::TriangleStrip)
			{
				concurrency::parallel_for(static_cast<size_t>(0), mesh->m_Indices.size() - 2, [=](const size_t i)
				{
					uint32_t index1{ mesh->m_Indices[i] };
					uint32_t index2{ mesh->m_Indices[i + 1] };
					uint32_t index3{ mesh->m_Indices[i + 2] };

					if (index3 % 2 == 1)
					{
						index2 = mesh->m_Indices[i + 2];
						index3 = mesh->m_Indices[i + 1];
					}

					Vertex_Out v0{ mesh->m_VerticesOut[index1] };
					Vertex_Out v1{ mesh->m_VerticesOut[index2] };
					Vertex_Out v2{ mesh->m_VerticesOut[index3] };

					PixelRenderLoop(v0, v1, v2);
				});
			}
		}
		//@END
		//Update SDL Surface
		SDL_UnlockSurface(m_pBackBuffer);
		SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
		SDL_UpdateWindowSurface(m_pWindow);

	}

	void Software::VertexTransformationFunction(const std::vector<Mesh*>& mesh, const Camera& camera) const
	{
		for (auto& m : mesh)
		{
			m->m_VerticesOut.clear();
			const auto worldViewProjectionMatrix{ m->m_WorldMatrix * camera.viewMatrix * camera.projectionMatrix };

			for (const auto& vertices : m->m_VerticesIn)
			{
				// Projection.
				Vector4 projectedVertex{ worldViewProjectionMatrix.TransformPoint(vertices.position.x, vertices.position.y, vertices.position.z, 1) };

				// World space normal calculation.
				Vector3 worldSpaceNormal{ m->m_WorldMatrix.TransformVector(vertices.normal).Normalized() };

				// Tangent calculation.
				Vector3 tangent{ m->m_WorldMatrix.TransformVector(vertices.tangent).Normalized() };

				// View Direction Calculation.
				Vector3 viewDirection{ camera.origin - m->m_WorldMatrix.TransformPoint(vertices.position) };

				// Perspective Divide.
				projectedVertex.x /= projectedVertex.w;
				projectedVertex.y /= projectedVertex.w;
				projectedVertex.z /= projectedVertex.w;

				Vertex_Out temp{ projectedVertex, vertices.color, vertices.uv, worldSpaceNormal, tangent, viewDirection };
				m->m_VerticesOut.emplace_back(temp);
			}
		}
	}

	void Software::PixelRenderLoop(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2) const
	{
		// Bounding Box.
		Vector3 min{}, max{};
		min = Vector3::Min(v0.position, Vector3::Min(v1.position, v2.position));
		max = Vector3::Max(v0.position, Vector3::Max(v1.position, v2.position));

		Vector3 margin{ 1.f, 1.f, 1.f };
		min -= margin;
		max += margin;

		min = Vector3::Max({ 0, 0, 0 }, Vector3::Min({ static_cast<float>(m_Width - 1), static_cast<float>(m_Height - 1), 0.f }, min));
		max = Vector3::Max({ 0, 0, 0 }, Vector3::Min({ static_cast<float>(m_Width - 1), static_cast<float>(m_Height - 1), 0.f }, max));

		for (int px{ static_cast<int>(min.x) }; px < static_cast<int>(max.x); ++px)
		{
			for (int py{ static_cast<int>(min.y) }; py < static_cast<int>(max.y); ++py)
			{
				if (m_ToggleBoundingBox)
				{
					m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
						static_cast<uint8_t>(255),
						static_cast<uint8_t>(255),
						static_cast<uint8_t>(255));
				}
				else
				{
					Vector2 pixel{ static_cast<float>(px), static_cast<float>(py) };

					//edges
					const Vector2 v0v1{ v1.position.GetXY() - v0.position.GetXY() };
					const Vector2 v1v2{ v2.position.GetXY() - v1.position.GetXY() };
					const Vector2 v2v0{ v0.position.GetXY() - v2.position.GetXY() };

					const Vector2 v0v2{ v2.position.GetXY() - v0.position.GetXY() };

					// Vector from vertex to pixel.
					Vector2 vertexToPixel1{ pixel - v0.position.GetXY() };
					Vector2 vertexToPixel2{ pixel - v1.position.GetXY() };
					Vector2 vertexToPixel3{ pixel - v2.position.GetXY() };

					//Cross of vertex to pixel and vertex
					float signedArea1{ Vector2::Cross(v0v1, vertexToPixel1) };
					float signedArea2{ Vector2::Cross(v1v2, vertexToPixel2) };
					float signedArea3{ Vector2::Cross(v2v0, vertexToPixel3) };

					// Culling Check.
					bool isBackCulling{ signedArea1 > 0 && signedArea2 > 0 && signedArea3 > 0 };
					bool isFrontCulling{ signedArea1 < 0 && signedArea2 < 0 && signedArea3 < 0 };

					if ((m_CurrentCullingMode == Culling::Back && isBackCulling) || (m_CurrentCullingMode == Culling::Front
						&& isFrontCulling) || (m_CurrentCullingMode == Culling::None && (isFrontCulling || isBackCulling)))
					{
						// Pixel inside triangle.
						float areaTotalParallelogram{ Vector2::Cross(v0v1, v0v2) };

						float W0{ signedArea2 / areaTotalParallelogram };
						float W1{ signedArea3 / areaTotalParallelogram };
						float W2{ signedArea1 / areaTotalParallelogram };

						//Vector3 pixelPosition{ (v0.position * W0) + (v1.position * W1) + (v2.position * W2) };

						float zBufferValue{ ZBufferValue(v0, v1, v2, W0, W1, W2) };

						float depth = m_pDepthBufferPixels[py * m_Width + px];
						//if (pixelPosition.z < depth)
						if (zBufferValue < depth)
						{
							m_pDepthBufferPixels[py * m_Width + px] = zBufferValue;
							//Vector2 uv = { (v0.uv * W0) + (v1.uv * W1) + (v2.uv * W2) };
							float wInterpolated{ WInterpolated(v0, v1, v2, W0, W1, W2) };
							Vector2 uv{ UVInterpolated(v0, v1, v2, W0, W1, W2, wInterpolated) };
							Vector3 normal{ NormalInterpolated(v0, v1, v2, W0, W1, W2, wInterpolated).Normalized() };
							Vector3 tangent{ TangentInterpolated(v0, v1, v2, W0, W1, W2, wInterpolated).Normalized() };
							Vector3 viewDirection{ ViewDirInterpolated(v0, v1, v2, W0, W1, W2, wInterpolated).Normalized() };

							ColorRGB finalColor{};

							if (!m_DepthBufferVisualized)
							{
								//finalColor = m_pTexture->Sample(uv);

								const Vector4 pixelPos{ static_cast<float>(px), static_cast<float>(py), zBufferValue, wInterpolated };

								Vertex_Out pixelVertex{ pixelPos, finalColor, uv, normal, tangent, viewDirection };

								//finalColor += PixelShading(pixelVertex);
								finalColor = PixelShading(pixelVertex);
							}
							else
							{
								finalColor = ColorRGB{ 1, 1, 1 } *Remap(zBufferValue, 0.985f, 1.f, 0.f, 1.f);
							}

							//Update Color in Buffer
							finalColor.MaxToOne();

							m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
								static_cast<uint8_t>(finalColor.r * 255),
								static_cast<uint8_t>(finalColor.g * 255),
								static_cast<uint8_t>(finalColor.b * 255));
						}
					}
				}
			}
		}
	}

	float Software::ZBufferValue(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const float w0, const float w1, const float w2) const
	{
		float denominator = (1.0f / v0.position.z) * w0 + (1.0f / v1.position.z) * w1 + (1.0f / v2.position.z) * w2;
		return (1 / denominator);
	}

	float Software::WInterpolated(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const float w0, const float w1, const float w2) const
	{
		float denominator = (1.0f / v0.position.w) * w0 + (1.0f / v1.position.w) * w1 + (1.0f / v2.position.w) * w2;
		return (1 / denominator);
	}

	Vector2 Software::UVInterpolated(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const float w0, const float w1, const float w2, const float wInterpolated) const
	{
		return ((v0.uv / v0.position.w) * w0 + (v1.uv / v1.position.w) * w1 + (v2.uv / v2.position.w) * w2) * wInterpolated;
	}

	Vector3 Software::NormalInterpolated(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const float w0, const float w1, const float w2, const float wInterpolated) const
	{
		return ((v0.normal / v0.position.w) * w0 + (v1.normal / v1.position.w) * w1 + (v2.normal / v2.position.w) * w2) * wInterpolated;
	}

	Vector3 Software::TangentInterpolated(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const float w0, const float w1, const float w2, const float wInterpolated) const
	{
		return ((v0.tangent / v0.position.w) * w0 + (v1.tangent / v1.position.w) * w1 + (v2.tangent / v2.position.w) * w2) * wInterpolated;
	}

	Vector3 Software::ViewDirInterpolated(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const float w0, const float w1, const float w2, const float wInterpolated) const
	{
		return ((v0.viewDirection / v0.position.w) * w0 + (v1.viewDirection / v1.position.w) * w1 + (v2.viewDirection / v2.position.w) * w2) * wInterpolated;
	}

	float Software::Remap(float value, float oldRangeL, float oldRangeN, float newRangeL, float newRangeN) const
	{
		const float newVal{ std::clamp(value, oldRangeL, oldRangeN) };
		return newRangeL + (newVal - oldRangeL) * (newRangeN - newRangeL) / (newRangeN - oldRangeL);
	}

	ColorRGB Software::PixelShading(const Vertex_Out& v) const
	{
		Vector3 lightDirection{ 0.577f, -0.577f, 0.577f };
		float lightIntensity{ 7.0f };
		const ColorRGB ambient{ 0.025f, 0.025f, 0.025f };

		Vector3 tangentSpaceVector = v.normal;

		if (m_ToggleNormalMap)
		{
			//// Calculate Binormal.
			Vector3 binormal{ Vector3::Cross(v.normal, v.tangent).Normalized() };

			//// Calculate Tangent space Matrix.
			Matrix tangentSpaceMatrix{ v.tangent, binormal, v.normal, Vector3::Zero };

			//// Calculate Normal according to the Normal Map.
			ColorRGB normalMapCol{ (2 * m_pNormalVehicle->Sample(v.uv)) - colors::White };
			Vector3 normalMapVector{ normalMapCol.r, normalMapCol.g, normalMapCol.b };
			normalMapVector /= 255.f;
			tangentSpaceVector = tangentSpaceMatrix.TransformVector(normalMapVector).Normalized();
		}

		float lambertCosineLaw{ GetLambertCosine(tangentSpaceVector, lightDirection) };

		if (lambertCosineLaw < 0.f)
		{
			return { 0,0,0 };
		}

		float specularShininess{ 25.f };
		float specularExp{ specularShininess * m_pGlossVehicle->Sample(v.uv).r };
		ColorRGB specular{ BRDF::Phong(m_pSpecularVehicle->Sample(v.uv), 1.f, specularExp, lightDirection, v.viewDirection, tangentSpaceVector) };
		ColorRGB lambert{ BRDF::Lambert(1.0f, m_pDiffuseVehicle->Sample(v.uv)) };

		switch (m_ShadingMode)
		{
		case ShadingModes::Combined:
			return ((lambert * lightIntensity + specular) * lambertCosineLaw) + ambient;
			break;
		case ShadingModes::ObservedArea:
			return ColorRGB{ 1, 1, 1 } * lambertCosineLaw;
			break;
		case ShadingModes::Diffuse:
			return (lambert * lambertCosineLaw * lightIntensity);
			break;
		case ShadingModes::Specular:
			return specular;
			break;
		default:
			return ((lambert * lightIntensity + specular) * lambertCosineLaw) + ambient;
		}

		//return (lambert * lambertCosineLaw * lightIntensity); // lambert final.
		//return specular; // specular final.
		//return ((lambert * lightIntensity) + specular) * lambertCosineLaw; // combined.
		//return ColorRGB{1, 1, 1} * lambertCosineLaw;

	}

	float Software::GetLambertCosine(const Vector3& normal, const Vector3& lightDirection) const
	{
		const float lambertCosine = std::max(0.f, Vector3::Dot(normal, -lightDirection.Normalized()));
		return lambertCosine;
	}

	void Software::SetMesh(Mesh* pMesh)
	{
		m_pVehicleMesh = pMesh;
	}

	void Software::SetTextures(Texture* pDiffuse, Texture* pNormal, Texture* pGloss, Texture* pSpecular)
	{
		m_pDiffuseVehicle = pDiffuse;
		m_pNormalVehicle = pNormal;
		m_pGlossVehicle = pGloss;
		m_pSpecularVehicle = pSpecular;
	}

	void Software::CycleCullMode()
	{
		int count{ static_cast<int>(m_CurrentCullingMode) };
		count++;
		if (count > 2)
		{
			count = 0;
		}
		const auto castEnum = static_cast<Culling>(count);
		m_CurrentCullingMode = castEnum;

		switch (m_CurrentCullingMode)
		{
		case Culling::None:
			m_CurrentCullingMode = Culling::None;
			break;
		case Culling::Back:
			m_CurrentCullingMode = Culling::Back;
			break;
		case Culling::Front:
			m_CurrentCullingMode = Culling::Front;
			break;
		}
	}

	void Software::CycleShadingMode()
	{
		int count{ static_cast<int>(m_ShadingMode) };
		count++;
		if (count > 3)
		{
			count = 0;
		}
		const auto castEnum = static_cast<ShadingModes>(count);
		m_ShadingMode = castEnum;

		const std::array<std::string, 4> shadingNames{ "Shading Mode: Combined.", "Shading Mode: Observed Area.", "Shading Mode: Diffuse.", "Shading Mode: Specular." };
		std::cout << shadingNames.at(count) << std::endl;
	}
}
