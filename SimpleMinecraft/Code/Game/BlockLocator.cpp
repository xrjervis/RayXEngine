#include "Game/Chunk.hpp"
#include "Game/BlockLocator.hpp"

BlockLocator::BlockLocator(Chunk* chunk, int blockIndex) 
	: m_chunk(chunk)
	, m_blockIndex(blockIndex) {

}

BlockLocator BlockLocator::GetBlockLocatorToEast() {
	GUARANTEE_OR_DIE(m_chunk != nullptr, "BlockLocator: m_chunk is invalid!");
	// NOT on the edge
	if ((m_blockIndex & CHUNK_MASK_X) != CHUNK_MASK_X) {
		return BlockLocator(m_chunk, m_blockIndex + 1);
	}
	else {
		return BlockLocator(m_chunk->GetEastNeighbor(), m_blockIndex & (~CHUNK_MASK_X));
	}
}

BlockLocator BlockLocator::GetBlockLocatorToWest() {
	GUARANTEE_OR_DIE(m_chunk != nullptr, "BlockLocator: m_chunk is invalid!");
	// NOT on the edge
	if ((m_blockIndex & CHUNK_MASK_X) != 0) {
		return BlockLocator(m_chunk, m_blockIndex - 1);
	}
	else {
		return BlockLocator(m_chunk->GetWestNeighbor(), m_blockIndex | CHUNK_MASK_X);
	}
}

BlockLocator BlockLocator::GetBlockLocatorToNorth() {
	GUARANTEE_OR_DIE(m_chunk != nullptr, "BlockLocator: m_chunk is invalid!");
	// NOT on the edge
	if ((m_blockIndex & CHUNK_MASK_Y) != CHUNK_MASK_Y) {
		return BlockLocator(m_chunk, m_blockIndex + CHUNK_SIZE_X);
	}
	else {
		return BlockLocator(m_chunk->GetNorthNeighbor(), m_blockIndex & (~CHUNK_MASK_Y));
	}
}

BlockLocator BlockLocator::GetBlockLocatorToSouth() {
	GUARANTEE_OR_DIE(m_chunk != nullptr, "BlockLocator: m_chunk is invalid!");
	// NOT on the edge
	if ((m_blockIndex & CHUNK_MASK_Y) != 0) {
		return BlockLocator(m_chunk, m_blockIndex - CHUNK_SIZE_X);
	}
	else {
		return BlockLocator(m_chunk->GetSouthNeightbor(), m_blockIndex | CHUNK_MASK_Y);
	}
}

BlockLocator BlockLocator::GetBlockLocatorAbove() {
	GUARANTEE_OR_DIE(m_chunk != nullptr, "BlockLocator: m_chunk is invalid!");
	// NOT on the edge
	if ((m_blockIndex & CHUNK_MASK_Z) != CHUNK_MASK_Z) {
		return BlockLocator(m_chunk, m_blockIndex + BLOCKS_PER_LAYER);
	}
	else {
		return BlockLocator(nullptr, m_blockIndex);
	}
}

BlockLocator BlockLocator::GetBlockLocatorBelow() {
	GUARANTEE_OR_DIE(m_chunk != nullptr, "BlockLocator: m_chunk is invalid!");
	// NOT on the edge
	if ((m_blockIndex & CHUNK_MASK_Z) != 0) {
		return BlockLocator(m_chunk, m_blockIndex - BLOCKS_PER_LAYER);
	}
	else {
		return BlockLocator(nullptr, m_blockIndex);
	}
}

void BlockLocator::StepEast() {
	*this = this->GetBlockLocatorToEast();
}

void BlockLocator::StepWest() {
	*this = this->GetBlockLocatorToWest();
}

void BlockLocator::StepNorth() {
	*this = this->GetBlockLocatorToNorth();
}

void BlockLocator::StepSouth() {
	*this = this->GetBlockLocatorToSouth();
}

void BlockLocator::StepAbove() {
	*this = this->GetBlockLocatorAbove();
}

void BlockLocator::StepBelow() {
	*this = this->GetBlockLocatorBelow();
}
