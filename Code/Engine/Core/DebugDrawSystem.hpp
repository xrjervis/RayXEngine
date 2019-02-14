#pragma once
#include "Engine/Core/type.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include <array>


enum eDebugDrawMode {
	DEBUG_RENDER_IGNORE_DEPTH, // will always draw and be visible 
	DEBUG_RENDER_USE_DEPTH,    // draw using normal depth rules
	DEBUG_RENDER_HIDDEN,       // only draws if it would be hidden by depth
	DEBUG_RENDER_XRAY,         // always draws, but hidden area will be drawn differently
};

struct DebugDrawOptions_t {
public:
	DebugDrawOptions_t();
	DebugDrawOptions_t(float liftTime, const Rgba& startColor, const Rgba&endColor, eDebugDrawMode mode = DEBUG_RENDER_USE_DEPTH);

	Rgba m_startColor;
	Rgba m_endColor;
	float m_lifetime;
	eDebugDrawMode m_mode;
};

//----------------------------------------------------------------------
// Debug objects definitions
//----------------------------------------------------------------------
class DebugObject {
public:
	DebugObject(const DebugDrawOptions_t& options);
	virtual	~DebugObject();
	void				Update(float deltaSeconds);
	virtual void		Render() const = 0;
	bool				IsDead() const;
	float				GetNormalizedAge() const;
protected:
	float				m_age = 0.f;
	Rgba				m_color;
	DebugDrawOptions_t	m_options;
};

class DebugLine3D : public DebugObject {
public:
	DebugLine3D(const Vector3& startPos, const Vector3& endPos, const DebugDrawOptions_t& options);
	~DebugLine3D();

	void Render() const override;
protected:
	Vector3 m_startPos;
	Vector3 m_endPos;
};


//----------------------------------------------------------------------
// Debug draw system
//----------------------------------------------------------------------
constexpr size_t MAX_DEBUG_OBJECTS = 128;

class Camera;

class DebugDrawSystem {
public:
	DebugDrawSystem();
	~DebugDrawSystem();

	void SetCamera2D(Camera* cam);
	void SetCamera3D(Camera* cam);

	void Update(float deltaSeconds);
	void Render2D() const;
	void Render3D() const;

	void AddDebugObject2D(Uptr<DebugObject>&& obj2d);
	void AddDebugObject3D(Uptr<DebugObject>&& obj3d);

private:
	std::array<Uptr<DebugObject>, MAX_DEBUG_OBJECTS> m_debugObjects2D;
	std::array<Uptr<DebugObject>, MAX_DEBUG_OBJECTS> m_debugObjects3D;

	Camera*		m_camera2D = nullptr;
	Camera*		m_camera3D = nullptr;
};
extern Uptr<DebugDrawSystem>	g_theDebugDraw;


//----------------------------------------------------------------------
// Debug draw helper functions
//----------------------------------------------------------------------
void DebugDrawBase(float lifeTime, const Vector3& originPos, const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis, float axisLength = 1.f);