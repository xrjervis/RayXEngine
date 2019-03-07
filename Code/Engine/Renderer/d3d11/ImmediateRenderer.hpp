#pragma once
#include "Engine/Renderer/d3d11/Light.hpp"
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
class Mesh;
class Material;
class Texture2D;

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
	void DrawPoint3D(const Vector3& center, const Rgba& color = Rgba::WHITE);
	void DrawLine3D(const Vector3& startPos, const Vector3& endPos, const Rgba& color = Rgba::WHITE);
	void DrawCube(const Vector3& center, const Vector3& size, const Rgba& color = Rgba::WHITE);
	void DrawQuad3D(const Vector3& position, const Vector2& size, const Vector3& rightVector, const Vector3& upVector, const Rgba& color = Rgba::WHITE);


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
	void DrawMeshImmediate(Mesh* mesh);

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