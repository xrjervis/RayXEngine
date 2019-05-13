#pragma once
class Chunk;

class BlockLocator {
public:
	BlockLocator() = default;
	explicit BlockLocator(Chunk* chunk, int blockIndex);
	~BlockLocator() = default;

	BlockLocator GetBlockLocatorToEast();
	BlockLocator GetBlockLocatorToWest();
	BlockLocator GetBlockLocatorToNorth();
	BlockLocator GetBlockLocatorToSouth();
	BlockLocator GetBlockLocatorAbove();
	BlockLocator GetBlockLocatorBelow();

	void		StepEast();
	void		StepWest();
	void		StepNorth();
	void		StepSouth();
	void		StepAbove();
	void		StepBelow();

	eBLockType	GetType() const;
	void		SetType(eBLockType type);
	bool		IsSolid() const;

public:
	Chunk* m_chunk = nullptr;
	int m_blockIndex = -1;
};


inline eBLockType BlockLocator::GetType() const {
	return m_chunk->GetBlockType(m_blockIndex);
}

inline void BlockLocator::SetType(eBLockType type) {
	m_chunk->SetBlockType(m_blockIndex, type);
}

inline bool BlockLocator::IsSolid() const {
	Block* b = &m_chunk->m_blocks[m_blockIndex];
	return b->IsSolid();
}