#include "Engine/Math/MatrixStack.hpp"

MatrixStack::MatrixStack() {
	m_top.SetIdentity();
}

void MatrixStack::Push() {
	m_stack.push(m_top);
}

void MatrixStack::Pop() {
	m_stack.pop();
	m_top = m_stack.top();
}

void MatrixStack::Load(Matrix44& mat) {
	Pop();
	m_top = mat.Append(m_top);
	Push();
}

void MatrixStack::LoadIdentity() {
	m_top.SetIdentity();
	Push();
}

void MatrixStack::Transform(Matrix44& mat) {
	m_top.Append(mat);
	Pop();
	Push();
}
