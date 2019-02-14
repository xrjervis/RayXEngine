#pragma once
#include "Engine/Math/Matrix44.hpp"
#include <stack>

class MatrixStack {
public:
	MatrixStack(); // set m_top to IDENTITY

	void Load(Matrix44& mat); // set m_top to mat; 
	void LoadIdentity(); // set m_top to the IDENTITY matrix
	void Transform(Matrix44& mat); // set top to the top transformed by mat. 
	// any other helpers you need/want
	void Push();   // push m_top to m_stack
	void Pop();    // Set m_top to the top of m_stack, and pop m_stack

public:
	Matrix44 m_top; // I prefer to keep top seperate since I use it a lot.
	std::stack<Matrix44> m_stack; // really, a history
};