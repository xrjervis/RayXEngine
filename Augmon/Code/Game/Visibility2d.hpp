#pragma once
#include "Engine/Core/type.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/d3d11/Mesh.hpp"
#include <set>
#include <vector>
#include <algorithm>

struct LineSegment_t {
	LineSegment_t(const Vector2& start, const Vector2& end)
		: m_startPoint(start)
		, m_endPoint(end) {}

	Vector2 m_startPoint;
	Vector2 m_endPoint;
};

class Visibility2d {
public:
	Visibility2d() = default;
	~Visibility2d() = default;

	struct OrientCompare {
		OrientCompare(const Vector2& pos) : m_pos(pos) {}
		bool operator() (const Vector2& lhs, const Vector2& rhs) const {
			float a = (lhs - m_pos).GetOrientationDegrees();
			float b = (rhs - m_pos).GetOrientationDegrees();
			if (ApproxEqual(a, b, 0.001f)) {
				return (lhs - m_pos).GetLength() < (rhs - m_pos).GetLength();
			}
			return a < b;
		}

		const Vector2& m_pos;
	};

	struct LengthGreaterCompare {
		LengthGreaterCompare(const Vector2& pos) : m_pos(pos) {}
		bool operator() (const Vector2& lhs, const Vector2& rhs) const {
			return (lhs - m_pos).GetLengthSquared() > (rhs - m_pos).GetLengthSquared();
		}
		const Vector2& m_pos;
	};

	void AddWall(const AABB2& bounds) {
		m_wallsBounds.emplace_back(bounds);
		AddLine(LineSegment_t{ bounds.mins, Vector2(bounds.maxs.x, bounds.mins.y) });
		AddLine(LineSegment_t{ Vector2(bounds.maxs.x, bounds.mins.y), bounds.maxs });
		AddLine(LineSegment_t{ Vector2(bounds.mins.x, bounds.maxs.y), bounds.maxs });
		AddLine(LineSegment_t{ bounds.mins, Vector2(bounds.mins.x, bounds.maxs.y) });
	}

	void AddLine(const LineSegment_t& line) {
		m_lines.push_back(line);
		if (std::find(m_endPoints.begin(), m_endPoints.end(), line.m_startPoint) == m_endPoints.end()) {
			m_endPoints.push_back(line.m_startPoint);
		}
		if (std::find(m_endPoints.begin(), m_endPoints.end(), line.m_endPoint) == m_endPoints.end()) {
			m_endPoints.push_back(line.m_endPoint);
		}
	}

	void SetPlayerPosition(const Vector2& pos) {
		m_playerPosition = pos;
		std::sort(m_endPoints.begin(), m_endPoints.end(), OrientCompare(m_playerPosition));
	}

	void RebuildMesh() {
		if (m_mesh) {
			m_mesh->Reset(PRIMITIVE_TYPE_TRIANGLELIST, false);
		}
		else {
			m_mesh = std::make_unique<Mesh<VertexPCU>>(PRIMITIVE_TYPE_TRIANGLELIST, false);
		}

		std::vector<Vector2> vertices;
		for (auto& endPoint : m_endPoints) {
			Vector2 rayStart = m_playerPosition;
			Vector2 rayDir = (endPoint - rayStart).GetNormalized();

			std::vector<Vector2> hitPositions;

			for (auto& line : m_lines) {
				Vector2 hitPosition;
				bool hit = DoRayAndLineSegmentsIntersect(rayStart, rayDir, line.m_startPoint, line.m_endPoint, hitPosition);
				if (hit) {
					// only add unique points
					if (std::find(hitPositions.begin(), hitPositions.end(), hitPosition) == hitPositions.end()) {
						hitPositions.emplace_back(hitPosition);
					}
				}
			}
			
			if (!hitPositions.empty()) {
				Vector2 dir = (endPoint - rayStart).GetNormalized();
				if (dir.x != 0.f && dir.y != 0.f) {
					Vector2 point = endPoint + 0.1f * dir;
					for (auto& bounds : m_wallsBounds) {
						bool result = bounds.IsPointInside(point);
						if (result) {
							hitPositions.clear();
							hitPositions.emplace_back(endPoint);
							break;
						}
					}
				}

				std::sort(hitPositions.begin(), hitPositions.end(), LengthGreaterCompare(m_playerPosition));
				while (hitPositions.size() > 2) {
					hitPositions.erase(hitPositions.begin());
				}
				for (auto& hitPosition : hitPositions) {
					vertices.emplace_back(hitPosition);
				}
			}
		}

		Rgba color = Rgba::YELLOW;
		Vector4 c = color.GetAsFloats();

		std::sort(vertices.begin(), vertices.end(), OrientCompare(m_playerPosition));


		for (int i = 0; i < vertices.size() - 1; ++i) {
			Vector2 dir_1 = (vertices[i] - m_playerPosition).GetNormalized();
			Vector2 dir_2 = (vertices[i + 1] - m_playerPosition).GetNormalized();
			if (dir_1 == dir_2) {
				for (auto& bounds : m_wallsBounds) {
					if (bounds.IsPointInside(vertices[i])) {
						Vector2 wallDir = (bounds.GetCenter() - m_playerPosition).GetNormalized();
						Vector2 lineDir = (vertices[i] - m_playerPosition).GetNormalized();
						float wallOrientDegrees = wallDir.GetOrientationDegrees();
						float lineDirDegrees = lineDir.GetOrientationDegrees();
						if (wallOrientDegrees == 180.f) {
							wallOrientDegrees = -180.f;
						}
						if (lineDirDegrees == 180.f) {
							lineDirDegrees = -180.f;
						}

						if (wallOrientDegrees + 180.f > lineDirDegrees + 180.f) {
							if ((vertices[i + 1] - m_playerPosition).GetLengthSquared() > (vertices[i] - m_playerPosition).GetLengthSquared()) {
								std::swap(vertices[i], vertices[i + 1]);
							}
							break;
						}
					}
				}
			}
		}

		for (int i = 0; i < vertices.size() - 1; ++i) {
			Vector2 dir_1 = (vertices[i] - m_playerPosition).GetNormalized();
			Vector2 dir_2 = (vertices[i + 1] - m_playerPosition).GetNormalized();
			if (dir_1 == dir_2) {
				continue;
			}
			m_mesh->m_vertices.emplace_back(m_playerPosition, c, Vector2::ZERO);
			m_mesh->m_vertices.emplace_back(vertices[i], c, Vector2::ZERO);
			m_mesh->m_vertices.emplace_back(vertices[i + 1], c, Vector2::ZERO);
		}
		m_mesh->m_vertices.emplace_back(m_playerPosition, c, Vector2::ZERO);
		m_mesh->m_vertices.emplace_back(vertices[vertices.size() - 1], c, Vector2::ZERO);
		m_mesh->m_vertices.emplace_back(vertices[0], c, Vector2::ZERO);
	}

	Vector2 m_playerPosition;
	std::vector<Vector2> m_endPoints;
	std::vector<LineSegment_t> m_lines;
	std::vector<AABB2> m_wallsBounds;
	Uptr<Mesh<VertexPCU>>	m_mesh;
};