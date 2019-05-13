#pragma once
#include "Engine/Audio/MIDIPlayer.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Board.hpp"

class ComposerGame {
public:
	ComposerGame();
	~ComposerGame();

	void LoadDefaultResources();
	Vector2 GetGridPositionFromGridCoordsOnBoardWindow(const IntVector2& gridCoords) const;
	Vector2 GetGridCenterFromGridCoordsOnBoardWindow(const IntVector2& gridCoords) const;


	void Update();
	void Render() const;

	void ClearBoardWindow() const;
	void RenderGridLinesOnBoardWindow() const;
	void RenderInitialQuadsAtCorner() const;
	void RenderTempoIndicator() const;
	void RenderNotesName() const;

public:
	Uptr<Clock> m_gameClock;
	Uptr<Camera> m_boardCamera;
	Uptr<MIDIPlayer> m_midiPlayer;
	Uptr<Board> m_board;

	float								m_currentIndicatorPos = 2.f;
	int									m_tempo = 200; //beats per minute
	std::array<std::vector<u8>, 36>		m_pianoTrack;
	std::array<bool, 36>				m_drumTrack;

	std::vector<std::vector<u8>>		m_track;

	std::string							m_noteNames[23] = {"C", "D", "E", "F", "G", "A", "B", "C", "D", "E", "F", "G", "A", "B", "C", "D", "E", "F", "G", "A", "B", "C", "D"};
	std::array<u8, 23>					m_notes = {48, 50, 52, 53, 55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79, 81, 83, 84, 86};
};