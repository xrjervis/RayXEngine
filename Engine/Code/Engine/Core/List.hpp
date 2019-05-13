#pragma once
#include "Engine/Math/MathUtils.hpp"
#include <vector>
#include <map>
#include <unordered_map>

template <typename ValueType>
struct SLNode_t {
	SLNode_t(int key, size_t level)
		: m_key(key), forward(level, nullptr) {}

	SLNode_t(int key, const ValueType& val, size_t level)
		: m_key(key), m_value(val), forward(level, nullptr) {}

	int			m_key;
	ValueType	m_value;
	// pointers to successor nodes
	std::vector<SLNode_t*> forward;
};

template <typename ValueType>
class SkipList{
public:
	SkipList(size_t maxLevel) 
		: m_maxLevel(maxLevel) {
		m_head = new SLNode_t<ValueType>(-1, maxLevel);
	}

	~SkipList() {
		SLNode_t<ValueType>* current = m_head;
		while(current->forward[0] != nullptr){
			SLNode_t<ValueType>* next = current->forward[0];
			delete current;
			current = next;
		}
	}

	SLNode_t<ValueType>* Find(int key) const {
		auto x = GetLowerBoundNode(key);
		if (x && x->m_key == key) {
			return x;
		}
		else {
			return nullptr;
		}
	}
	
	SLNode_t<ValueType>* Insert(int key, const ValueType& value) {
		SLNode_t<ValueType>* node = Find(key);
		if(node){
			node->m_value = value;
			return node;
		}

		size_t level = GetRandomLevel();
		SLNode_t<ValueType>* newNode = new SLNode_t<ValueType>(key, value, level);
		std::vector<SLNode_t<ValueType>*> update(GetNodeLevelCount(m_head), nullptr);
		SLNode_t<ValueType>* x = m_head;

		for (int i = GetNodeLevelCount(m_head) - 1; i >= 0; i--) {
			while (x->forward[i] && (x->forward[i]->m_key < key)) {
				x = x->forward[i];
			}
			update[i] = x;
		}
		// connect pointers of predecessors and new node to respective successors
		for (size_t i = 0; i < level; ++i) {
			newNode->forward[i] = update[i]->forward[i];
			update[i]->forward[i] = newNode;
		}

		return newNode;
	}
	void Erase(int key){
		SLNode_t<ValueType>* deleteNode = Find(key);
		if(deleteNode == nullptr){
			return;
		}

		std::vector<SLNode_t*> update(GetNodeLevelCount(m_head), nullptr);
		SLNode_t<ValueType>* x = m_head;

		for (int i = GetNodeLevelCount(deleteNode) - 1; i >= 0; i--) {
			while (x->forward[i] && (x->forward[i]->m_key < key)) {
				x = x->forward[i];
			}
			update[i] = x;
		}
		// update pointers and delete node 
		for (size_t i = 0; i < GetNodeLevelCount(deleteNode); ++i) {
			update[i]->forward[i] = node->forward[i];
		}
		delete deleteNode;
	}

private:
	// Generates node levels in the range [1, maxLevel).    
	size_t GetRandomLevel() const {
		return (size_t)GetRandomIntInRange(0, m_maxLevel - 1);
	}

	size_t GetNodeLevelCount(const SLNode_t<ValueType>* node) const {
		return node->forward.size();
	}

	// Returns the first node for which node->key < searchKey is false  
	SLNode_t<ValueType>* GetLowerBoundNode(int key) const{
		SLNode_t<ValueType>* x = m_head;
		for (int i = GetNodeLevelCount(m_head) - 1; i >= 0; i--) {
			while (x->forward[i] && (x->forward[i]->m_key < key)) {
				x = x->forward[i];
			}
		}
		return x->forward[0];
	}

	SLNode_t<ValueType>*	m_head = nullptr; // pointer to first node
	size_t					m_maxLevel;
};

//-----------------------------------------------Test Code--------------------------------------------------
// SkipList: 1.12s  std::map: 0.02s
// 	{
// 		PROFILE_LOG_SCOPE("SkipList");
// 		SkipList<float> testList(14);
// 		for (int i = 0; i < 10000; ++i) {
// 			testList.Insert(i, (float)(i * 10));
// 		}
// 		auto a = testList.Find(50);
// 		auto val = a->m_value;
// 	}
// 
// 	{
// 		PROFILE_LOG_SCOPE("std::map");
// 		std::map<int, float> testMap;
// 		for(int i = 0; i < 10000; ++i){
// 			testMap.insert({i, (float)(i * 10)});
// 		}
// 		auto a = testMap.find(50);
// 		auto val = a->second;
// 	}


class IDObject {
public:
	bool m_isDestroyed = false;
};

class DMap{
public:

	void Add(const std::string& key, const IDObject& value){
		m_isAddDirty = true;
		m_delayedKey.push_back(key);
		m_delayedObj.push_back(value);
	}

	void Remove(const std::string& key){
		m_isRemoveDirty = true;
		m_unorderedMap[key].m_isDestroyed = true;
	}

	void Remove(IDObject& obj){
		m_isRemoveDirty = true;
		obj.m_isDestroyed = true;
	}

	void Finalize() {
		if(m_isAddDirty){
			m_isAddDirty = false;
			for (size_t i = 0; i < m_delayedKey.size(); ++i) {
				m_unorderedMap.insert({std::move(m_delayedKey[i]), std::move(m_delayedObj[i])});
			}
			m_delayedKey.clear();
			m_delayedObj.clear();
		}

		if(m_isRemoveDirty){
			m_isRemoveDirty = false;
			for (auto it = m_unorderedMap.begin(); it != m_unorderedMap.end(); ) {
				if (it->second.m_isDestroyed) { 
					m_unorderedMap.erase(it++); 
				}
				else { 
					++it; 
				}
			}
		}
	}

protected:
	std::unordered_map<std::string, IDObject> m_unorderedMap;
	std::vector<std::string> m_delayedKey;
	std::vector<IDObject> m_delayedObj;
	bool m_isAddDirty = false;
	bool m_isRemoveDirty = false;
};
//--------------------------------------Test DMap----------------------------------------
// 	DMap entityList;
// 	{
// 		Entity e1("e1");
// 		Entity e2("e2");
// 		Entity e3("e3");
// 		Entity e4("e4");
// 
// 		entityList.Add(e1.m_name, e1);
// 		entityList.Add(e2.m_name, e2);
// 		entityList.Add(e3.m_name, e3);
// 		entityList.Add(e3.m_name, e3);
// 		entityList.Add(e4.m_name, e4);
// 	}
// 
// 	entityList.Finalize();
//  	entityList.Remove("e3");
//  	entityList.Finalize();
