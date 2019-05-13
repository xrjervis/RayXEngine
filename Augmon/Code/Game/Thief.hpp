#pragma once
#include "Game/GameCommon.hpp"

class Thief {
public:
	Thief() {}
	~Thief() {}

public:
	bool					m_isHidden = true;
	float					m_fractionOfExposion = 0.f;
	bool					m_hasMovedThisTurn = false;
	bool					m_isMoving = false;
	Vector2					m_centerOnThiefWindow;
	Vector2					m_centerOnBoardWindow;
	IntVector2				m_gridCoords;
	IntVector2				m_cursorGridCoords;
	std::vector<IntVector2> m_availableMoveGridCoords;
	int						m_numTreasuresStolen = 0;
	bool					m_isAnyTreasureStolenLastTurn = false;
	IntVector2				m_treasureGridCoordsStolenLastTurn;
	bool					m_isInAnyRoom = false;
	AABB2					m_roomGridBox;

	//--------------------------------------------------------------------
	const static Pattern_t	s_legoPattern;
	const static IntVector2 s_spriteCoords;
	static int				s_maxMoveDistance;
};