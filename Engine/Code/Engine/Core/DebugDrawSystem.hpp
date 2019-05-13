#pragma once
#include "Engine/Core/type.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include <array>
#include <deque>


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

public:
	float				m_age = 0.f;
	Rgba				m_color;
	DebugDrawOptions_t	m_options;
};

inline bool DebugObject::IsDead() const {
	return m_age > m_options.m_lifetime;
}

inline float DebugObject::GetNormalizedAge() const {
	return Clamp01(m_age / m_options.m_lifetime);
}

class DebugLine3D : public DebugObject {
public:
	DebugLine3D(const Vector3& startPos, const Vector3& endPos, const DebugDrawOptions_t& options);
	~DebugLine3D();

	void Render() const override;

public:
	Vector3 m_startPos;
	Vector3 m_endPos;
};

class DebugLine : public DebugObject {
public:
	DebugLine(const Vector2& startPos, const Vector2& endPos, const DebugDrawOptions_t& options);
	~DebugLine();

	void Render() const override;

public:
	Vector2 m_startPos;
	Vector2 m_endPos;
};

class DebugCube3D : public DebugObject {
public:
	DebugCube3D(const Vector3& center, const Vector3& size, const DebugDrawOptions_t& options);
	~DebugCube3D();
	
	void Render() const override;

public:
	Vector3 m_center;
	Vector3 m_size;
};


class DebugText2D : public DebugObject {
public:
	DebugText2D(const std::string& text, const DebugDrawOptions_t& options);
	~DebugText2D();
	
	void Render() const override {}

public:
	std::string m_text;
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
	void AddDebugText2D(Uptr<DebugText2D>&& text2D);

private:
	std::array<Uptr<DebugObject>, MAX_DEBUG_OBJECTS>	m_debugObjects2D;
	std::array<Uptr<DebugObject>, MAX_DEBUG_OBJECTS>	m_debugObjects3D;
	std::deque<Uptr<DebugText2D>>						m_debugText2DQueue;

	Camera*		m_camera2D = nullptr;
	Camera*		m_camera3D = nullptr;

	float m_debugStringSize = 15.f;
	int	m_numDebugStringInQueue = 100;
	AABB2 m_debugStringBounds;
};
extern Uptr<DebugDrawSystem>	g_theDebugDrawSystem;


//----------------------------------------------------------------------
// Debug draw helper functions
//----------------------------------------------------------------------
void DebugDrawBase(float lifeTime, const Vector3& originPos, const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis, float axisLength = 1.f);
void DebugString(float lifeTime, const std::string& text, const Rgba& startColor, const Rgba& endColor);
void DebugDrawLine(float lifeTime, const Vector2& startPos, const Vector2& endPos, const Rgba& startColor, const Rgba& endColor);
void DebugDrawLine3D(float lifeTime, const Vector3& startPos, const Vector3& endPos, const Rgba& startColor, const Rgba& endColor);
void DebugDrawCube(float lifeTime, const Vector3& center, const Vector3& size, const Rgba& startColor, const Rgba& endColor);