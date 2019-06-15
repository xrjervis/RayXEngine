#pragma once
#include "Engine/Renderer/d3d11/Light.hpp"
#include "Engine/Renderer/d3d11/Mesh.hpp"
#include "Engine/Renderer/d3d11/Buffer.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "ENgine/Core/Rgba.hpp"

class RHIDevice;
class RHIOutput;
class Camera;
class Material;
class Texture2D;
class SkeletalMesh;

constexpr u32 CAMERA_BUFFER_SLOT = 0U;
constexpr u32 OBJECT_BUFFER_SLOT = 1U;
constexpr u32 LIGHT_BUFFER_SLOT = 2U;
constexpr u32 MATERIAL_BUFFER_SLOT = 3U;
constexpr u32 GLOBAL_BUFFER_SLOT = 4U;


struct ObjectBuffer_t{
	Matrix44 model;
	Matrix44 inverseTransposeModel;
};

struct GlobalBuffer_t {
	Vector3 skyColor;
	float fogStart;
	Vector3 indoorLightColor;
	float fogEnd;
	Vector3 outdoorLightColor;
	// CreateConstantBuffer can be wrong if don't add this padding
	float padding;
};

// Traditional Rendering Pipeline
class ImmediateRenderer{
public:
	ImmediateRenderer(RHIDevice* device);
	~ImmediateRenderer();

	//--------------------------------------------------------------------
	// Helper functions for draw calls
	//--------------------------------------------------------------------
	void DrawLine2D(const Vector2& startPos, const Vector2& endPos, const Rgba& color = Rgba::WHITE);
	void DrawLineBox2D(const AABB2& boxBounds, const Rgba& color = Rgba::WHITE);
	void DrawQuad2D(const Vector2& position, const Vector2& pivot, const Vector2& size, const AABB2& uv, const Rgba& color = Rgba::WHITE);
	void DrawQuad2D(const Vector2& position, const Vector2& pivot, const Vector2& size, const Rgba& color = Rgba::WHITE);
	void DrawOBB2D(const Vector2& position, const Vector2& rightVector, const Vector2& upVector, const Vector2& halfExtensions, const Rgba& color = Rgba::WHITE);
	void DrawDisc2D(const Vector2& center, float radius, const Rgba& color = Rgba::WHITE);
	void DrawDashedCircle2D(const Vector2& center, float radius, const Rgba& color = Rgba::WHITE);
	void DrawPoint3D(const Vector3& center, const Rgba& color = Rgba::WHITE);
	void DrawLine3D(const Vector3& startPos, const Vector3& endPos, const Rgba& color = Rgba::WHITE);
	void DrawCube(const Vector3& center, const Vector3& size, const Rgba& color = Rgba::WHITE);
	void DrawQuad3D(const Vector3& position, const Vector2& size, const Vector3& rightVector, const Vector3& upVector, const Rgba& color = Rgba::WHITE);
	void DrawAABB3Box(const Vector3& center, const Vector3&size, const Rgba& color = Rgba::WHITE);
	void DrawUVSphere(const Vector3& center, float radius, int longitude = 16, int latitude = 16, const Rgba& color = Rgba::WHITE);

	//--------------------------------------------------------------------
	// Set lighting data to global lights array slot
	//--------------------------------------------------------------------
	void BindLighting(u32 idx, Light* light);

	//--------------------------------------------------------------------
	// Set output window/swapchain/backbuffer
	//--------------------------------------------------------------------
	void BindOutput(RHIOutput* output);

	//--------------------------------------------------------------------
	// If mat == nullptr, load default one	
	// Setup VS, PS, RS stages
	//--------------------------------------------------------------------
	void BindMaterial(Material* mat = nullptr);

	//--------------------------------------------------------------------
	// Setup RS, OM stage
	//--------------------------------------------------------------------
	void BindCamera(Camera* cam);	

	//--------------------------------------------------------------------
	// Setup IA stage
	//--------------------------------------------------------------------
	template<typename VertType>
	void DrawMeshImmediate(Mesh<VertType>* mesh);

	void DrawMeshImmediate(SkeletalMesh* sMesh);

	//--------------------------------------------------------------------
	// Set global constants
	void SetSkyColor(const Rgba& color);
	void SetIndoorColor(const Rgba& color);
	void SetOutdoorColor(const Rgba& color);
	void SetFogStart(float value);
	void SetFogEnd(float value);

public:
	RHIDevice*				m_device = nullptr;
	RHIOutput*				m_output = nullptr;

	Camera*					m_camera = nullptr;
	std::unique_ptr<Buffer> m_camera_cBuffer;

	ObjectBuffer_t			m_defaultObjectBuffer;
	std::unique_ptr<Buffer> m_obj_cBuffer;
	
	LightData_t				m_lights[MAX_LIGHTS];
	std::unique_ptr<Buffer> m_light_cBuffer;

	Material*				m_material = nullptr;
	std::unique_ptr<Buffer> m_material_cBuffer;

	GlobalBuffer_t			m_globals;
	std::unique_ptr<Buffer> m_global_cBuffer;
};

template<typename VertType>
void ImmediateRenderer::DrawMeshImmediate(Mesh<VertType>* mesh) {
	// Input Assembler
	if (mesh->m_isFinalized == false) {
		mesh->m_isFinalized = true;
		mesh->m_vertexBuffer.reset();
		mesh->m_vertexBuffer = m_device->CreateVertexBuffer(mesh->m_vertices.size() * sizeof(VertType), true, false, mesh->m_vertices.data());
		if (mesh->m_useIndices) {
			mesh->m_indexBuffer.reset();
			mesh->m_indexBuffer = m_device->CreateIndexBuffer(mesh->m_triangles.size() * 3 * sizeof(u32), false, mesh->m_triangles.data());
		}
		else {
			mesh->m_indexBuffer = nullptr;
		}

		GUARANTEE_OR_DIE(m_material != nullptr, "Failed to bind material for immediate renderer");
		mesh->m_inputLayout.reset();
		mesh->m_inputLayout = m_device->CreateInputLayout(VertType::s_layout, m_material->m_vs);
	}

	u32 strides = sizeof(VertType);
	u32 offsets = 0U;
	m_device->m_d3d11Context->IASetInputLayout(mesh->m_inputLayout->m_d3d11InputLayout.Get());
	m_device->m_d3d11Context->IASetPrimitiveTopology(ToDXPrimitiveType(mesh->m_primitiveType));
	m_device->m_d3d11Context->IASetVertexBuffers(0U, 1U, mesh->m_vertexBuffer->m_d3d11Buffer.GetAddressOf(), &strides, &offsets);
	if (mesh->m_useIndices) {
		m_device->m_d3d11Context->IASetIndexBuffer(mesh->m_indexBuffer->m_d3d11Buffer.Get(), DXGI_FORMAT_R32_UINT, 0U);
	}

	if (mesh->m_useIndices) {
		m_device->DrawIndexed(mesh->m_triangles.size() * 3);
	}
	else {
		m_device->Draw(mesh->m_vertices.size());
	}
}
