#pragma once
#include "Engine/Renderer/d3d11/Vertex.hpp"
#include "Engine/Renderer/d3d11/Buffer.hpp"
#include "Engine/Renderer/d3d11/InputLayout.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Core/type.hpp"
#include <vector>

struct Joint_t {
	Joint_t() {}
	Joint_t(const std::string& name, const Vector3& position, const Vector3& eulerRotation, const Vector3& scaling)
		: m_name(name) {
		m_transform.SetLocalPositioin(position);
		m_transform.SetLocalEulerAngles(eulerRotation);
		m_transform.SetLocalScale(scaling);
	}

	void AddChild(Uptr<Joint_t> sNode) {
		sNode->m_parent = this;
		sNode->m_transform.SetParent(&m_transform);
		m_children.emplace_back(std::move(sNode));
	}

	Joint_t* GetChild(int index) {
		return m_children[index].get();
	}

	std::string							m_name = "empty";
	Transform							m_transform;
	std::vector<Uptr<Joint_t>>			m_children;
	Joint_t*							m_parent = nullptr;
};

class SkeletalMesh {
public:
	SkeletalMesh();
	~SkeletalMesh();

public:
	bool							m_isFinalized = false;

	Uptr<Joint_t>					m_rootJoint;
	std::vector<Joint_t*>			m_joints;
	std::vector<VertexPCU>			m_vertices;
	std::vector<VertexSkeletal>		m_skeletalVertices;
	std::vector<int>				m_indices;
	Uptr<Buffer>					m_vertexBuffer;
	Uptr<Buffer>					m_indexBuffer;
	Uptr<InputLayout>				m_inputLayout;
};