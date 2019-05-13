#pragma once
#include "Engine/InputSystem/KeyButtonState.hpp"
#include "Engine/Math/Vector2.hpp"

enum XboxControllerButtonID {
	XBOXCONTROLLER_BUTTON_A,
	XBOXCONTROLLER_BUTTON_B,
	XBOXCONTROLLER_BUTTON_X,
	XBOXCONTROLLER_BUTTON_Y,
	XBOXCONTROLLER_BUTTON_LB,
	XBOXCONTROLLER_BUTTON_RB,
	XBOXCONTROLLER_BUTTON_UP,
	XBOXCONTROLLER_BUTTON_RIGHT,
	XBOXCONTROLLER_BUTTON_DOWN,
	XBOXCONTROLLER_BUTTON_LEFT,
	XBOXCONTROLLER_BUTTON_START,
	XBOXCONTROLLER_BUTTON_BACK,
	XBOXCONTROLLER_BUTTON_LS,
	XBOXCONTROLLER_BUTTON_RS,
	NUM_XBOXCONTROLLER_BUTTONS
};

enum XboxControllerStickID{
	XBOXCONTROLLER_STICK_LEFT,
	XBOXCONTROLLER_STICK_RIGHT,
	NUM_XBOXCONTROLLER_STICKS
};

enum XboxControllerTriggerID{
	XBOXCONTROLLER_TRIGGER_LEFT,
	XBOXCONTROLLER_TRIGGER_RIGHT,
	NUM_XBOXCONTROLLER_TRIGGERS
};

struct XboxControllerStickState {
	Vector2 m_correctedNormalizedPosiotion;
	float m_magnitude = 0.f;
	float m_orientationDegree = 0.f;
};

class XboxController{
public:
	~XboxController(){}
	XboxController(){}

	
	bool IsConnected() const;
	//return (-180.f, 180.f)
	float GetStickOrienDegree(const XboxControllerStickID& stickID) const;
	float GetStickMagnitude(const XboxControllerStickID& stickID) const;
	float GetTriggerMagnitude(const XboxControllerTriggerID& triggerID) const;
	bool IsButtonPressed(unsigned short buttonCode) const;
	bool WasButtonJustPressed(unsigned short buttonCode) const;
	bool WasButtonJustReleased(unsigned short buttonCode) const;

private:
	void Reset();
	void ClearButtonState();
	void UpdateButtonState(	const XboxControllerButtonID&	buttonID,	unsigned short flags, unsigned short mask);
	void UpdateStickState(	const XboxControllerStickID&	stickID,	short thumbX, short thumbY);
	void UpdateTriggerState(const XboxControllerTriggerID&	triggerID,	unsigned char triggerValue);
	void UpdateXboxControllerState(int controllerNumber);

public:
	friend class InputSystem;

private:
	bool m_isConnected = false;
	KeyButtonState m_buttonStates[NUM_XBOXCONTROLLER_BUTTONS];
	XboxControllerStickState m_stickStates[NUM_XBOXCONTROLLER_STICKS];
	float m_triggerMagnitudes[NUM_XBOXCONTROLLER_TRIGGERS];
};
