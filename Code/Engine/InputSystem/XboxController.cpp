#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>
#pragma comment( lib, "xinput9_1_0" )
#include <cstdio>

#include "Engine/InputSystem/XboxController.hpp"
#include "Engine/Math/MathUtils.hpp"

void XboxController::UpdateXboxControllerState(int controllerNumber) {
	ClearButtonState();

	XINPUT_STATE rawState;
	memset(&rawState, 0, sizeof(rawState));
	DWORD errorStatus = XInputGetState(controllerNumber, &rawState);

	if (errorStatus != ERROR_SUCCESS){
		Reset();
		return;
	}

	m_isConnected = true;

	UpdateButtonState(XBOXCONTROLLER_BUTTON_A,		rawState.Gamepad.wButtons, XINPUT_GAMEPAD_A);
	UpdateButtonState(XBOXCONTROLLER_BUTTON_B,		rawState.Gamepad.wButtons, XINPUT_GAMEPAD_B);
	UpdateButtonState(XBOXCONTROLLER_BUTTON_X,		rawState.Gamepad.wButtons, XINPUT_GAMEPAD_X);
	UpdateButtonState(XBOXCONTROLLER_BUTTON_Y,		rawState.Gamepad.wButtons, XINPUT_GAMEPAD_Y);
	UpdateButtonState(XBOXCONTROLLER_BUTTON_UP,		rawState.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_UP);
	UpdateButtonState(XBOXCONTROLLER_BUTTON_RIGHT,	rawState.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_RIGHT);
	UpdateButtonState(XBOXCONTROLLER_BUTTON_DOWN,	rawState.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_DOWN);
	UpdateButtonState(XBOXCONTROLLER_BUTTON_LEFT,	rawState.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_LEFT);
	UpdateButtonState(XBOXCONTROLLER_BUTTON_LB,		rawState.Gamepad.wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER);
	UpdateButtonState(XBOXCONTROLLER_BUTTON_RB,		rawState.Gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER);
	UpdateButtonState(XBOXCONTROLLER_BUTTON_START,	rawState.Gamepad.wButtons, XINPUT_GAMEPAD_START);
	UpdateButtonState(XBOXCONTROLLER_BUTTON_BACK,	rawState.Gamepad.wButtons, XINPUT_GAMEPAD_BACK);



	UpdateStickState(XBOXCONTROLLER_STICK_LEFT, rawState.Gamepad.sThumbLX, rawState.Gamepad.sThumbLY);
	UpdateStickState(XBOXCONTROLLER_STICK_RIGHT, rawState.Gamepad.sThumbRX, rawState.Gamepad.sThumbRY);

	UpdateTriggerState(XBOXCONTROLLER_TRIGGER_LEFT, rawState.Gamepad.bLeftTrigger);
	UpdateTriggerState(XBOXCONTROLLER_TRIGGER_RIGHT, rawState.Gamepad.bRightTrigger);
	
}

void XboxController::Reset() {
	m_isConnected = false;
	for (int i = 0; i < NUM_XBOXCONTROLLER_BUTTONS; ++i) {
		m_buttonStates[i] = KeyButtonState();
	}
}

void XboxController::ClearButtonState() {
	for(int i = 0; i < NUM_XBOXCONTROLLER_BUTTONS; ++i){
		m_buttonStates[i].m_wasKeyJustPressed = false;
		m_buttonStates[i].m_wasKeyJustReleased = false;
	}
}

void XboxController::UpdateButtonState(const XboxControllerButtonID& buttonID, unsigned short flags, unsigned short mask) {
	if(AreBitsSet(flags, mask)){
		if(m_buttonStates[buttonID].m_isKeyPressed == false){
			m_buttonStates[buttonID].m_isKeyPressed = true;
			m_buttonStates[buttonID].m_wasKeyJustPressed = true;
		}
	}
	else{
		if(m_buttonStates[buttonID].m_isKeyPressed == true){
			m_buttonStates[buttonID].m_isKeyPressed = false;
			m_buttonStates[buttonID].m_wasKeyJustReleased = true;
		}
	}
}

void XboxController::UpdateStickState(const XboxControllerStickID& stickID, short thumbX, short thumbY) {
	//Convert x, y to [-1.f, 1.f]
	float t_x = RangeMapFloat(thumbX, -32768.f, 32767.f, -1.f, 1.f);
	float t_y = RangeMapFloat(thumbY, -32768.f, 32767.f, -1.f, 1.f);

	//Convert to polar R[0, 1]
	float rawRadius = sqrtf((t_x * t_x) + (t_y * t_y));
	float t_theta = Atan2Degrees(t_y, t_x);

	//Remap Live Zone to R[0, 1]
	float correctedRaidus = RangeMapFloat(rawRadius, .3f, .9f, 0.f, 1.f);
	correctedRaidus = ClampFloat(correctedRaidus, 0.f, 1.f);

	//Convert back to (corrected) Cartesian x, y
	t_x = correctedRaidus * CosDegrees(t_theta);
	t_y = correctedRaidus * SinDegrees(t_theta);


	m_stickStates[stickID].m_orientationDegree = t_theta;
	m_stickStates[stickID].m_correctedNormalizedPosiotion = Vector2(t_x, t_y);
	m_stickStates[stickID].m_magnitude = correctedRaidus;
}


void XboxController::UpdateTriggerState(const XboxControllerTriggerID& triggerID, unsigned char triggerValue) {
	m_triggerMagnitudes[triggerID] = RangeMapFloat(static_cast<float>(triggerValue), 0.f, 255.f, 0.f, 1.f);
}


bool XboxController::IsConnected() const {
	return m_isConnected;
}

float XboxController::GetStickOrienDegree(const XboxControllerStickID& stickID) const {
	return m_stickStates[stickID].m_orientationDegree;
}

float XboxController::GetStickMagnitude(const XboxControllerStickID & stickID) const {
	return m_stickStates[stickID].m_magnitude;
}

float XboxController::GetTriggerMagnitude(const XboxControllerTriggerID& triggerID) const {
	return m_triggerMagnitudes[triggerID];
}

bool XboxController::IsButtonPressed(unsigned short buttonCode) const {
	return m_buttonStates[buttonCode].m_isKeyPressed;
}

bool XboxController::WasButtonJustPressed(unsigned short buttonCode) const {
	return m_buttonStates[buttonCode].m_wasKeyJustPressed;
}

bool XboxController::WasButtonJustReleased(unsigned short buttonCode) const {
	return m_buttonStates[buttonCode].m_wasKeyJustReleased;
}