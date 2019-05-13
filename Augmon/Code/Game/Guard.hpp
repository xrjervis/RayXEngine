#pragma once
#include "Game/GameCommon.hpp"


class Guard {
public:
	Guard() {}
	~Guard() {}

public:
	const static Pattern_t	s_legoPattern;
	const static IntVector2 s_spriteCoords;
	static IntRange			s_moveDistanceRange;
	static int				s_alertDistanceDefault;
	static int				s_alertDistanceWithFlashlight;
	static int				s_flashlightFullBattery;
	static float			s_possibilityForSingleMonitor;
	static float			s_possibilityForScanAllMonitors;
	static float			s_possibilityForMotionDetector;

	int						m_flashLightRemainedBattery;
	bool					m_isFlashlightOn = false;
	IntVector2				m_gridCoords;
	bool					m_needRemoveActionItems = false;
	bool					m_isMoving = false;
	bool					m_hasMovedThisTurn = false;
	bool					m_usedActionThisTurn = false;
	bool					m_usedEquipmentThisTurn = false;
	eActionType				m_actionUsedThisTurn;
	
	std::vector<eActionType> m_actionsInHand;
	eActionType				m_actionGotThisTurn;
	int						m_maxMoveDistanceThisTurn;
};