#include "Engine/Renderer/d3d11/ImmediateRenderer.hpp"
#include "Engine/Renderer/d3d11/RHIDevice.hpp"
#include "Engine/Renderer/d3d11/Mesh.hpp"
#include "Engine/Renderer/d3d11/Material.hpp"
#include "Engine/Renderer/d3d11/RHIOutput.hpp"
#include "Engine/Renderer/d3d11/Texture.hpp"
#include "Engine/Core/Camera.hpp"
#include "Engine/Core/ResourceManager.hpp"
#include "Engine/Core/EngineCommon.hpp"

ImmediateRenderer::ImmediateRenderer(RHIDevice* device)
	:m_device(device) {

	// create default object constant buffer
	m_obj_cBuffer = m_device->CreateConstantBuffer(sizeof(ObjectBuffer_t), false, false, &m_defaultObjectBuffer);

	m_light_cBuffer = m_device->CreateConstantBuffer(sizeof(LightData_t)*MAX_LIGHTS, false, false, nullptr);

	m_camera_cBuffer = m_device->CreateConstantBuffer(sizeof(CameraBuffer_t), false, false, nullptr);

	m_material_cBuffer = m_device->CreateConstantBuffer(sizeof(MaterialBuffer_t), false, false, nullptr);
}

ImmediateRenderer::~ImmediateRenderer() {

}

void ImmediateRenderer::DrawLine2D(const Vector2& startPos, const Vector2& endPos, const Rgba& color /*= Rgba::WHITE*/) {
	Mesh line(PRIMITIVE_TYPE_LINELIST, false);
	line.AddLine(startPos, endPos, color);
	DrawMeshImmediate(&line);
}

void ImmediateRenderer::DrawQuad2D(const Vector2& position, const Vector2& pivot, const Vector2& size, const Rgba& color) {
	Mesh quad(PRIMITIVE_TYPE_TRIANGLELIST, true);
	quad.AddQuad(position, pivot, size.x, size.y, color);
	DrawMeshImmediate(&quad);
}

void ImmediateRenderer::DrawLine3D(const Vector3& startPos, const Vector3& endPos, const Rgba& color /*= Rgba::WHITE*/) {
	Mesh line(PRIMITIVE_TYPE_LINELIST, false);
	line.AddLine(startPos, endPos, color);
	DrawMeshImmediate(&line);
}

void ImmediateRenderer::DrawCube(const Vector3& center, const Vector3& size, const Rgba& color /*= Rgba::WHITE*/) {
	Mesh cube(PRIMITIVE_TYPE_TRIANGLELIST, true);
	cube.AddCube(center, size, color);
	DrawMeshImmediate(&cube);
}

void ImmediateRenderer::DrawQuad3D(const Vector3& position, const Vector2& size, const Vector3& rightVector, const Vector3& upVector, const Rgba& color /*= Rgba::WHITE*/) {
	Mesh quad(PRIMITIVE_TYPE_TRIANGLELIST, true);
	quad.AddQuad3D(position, size.x, size.y, rightVector, upVector, color);
	DrawMeshImmediate(&quad);
}

void ImmediateRenderer::BindOutput(RHIOutput* output) {
	m_output = output;
}

void ImmediateRenderer::BindMaterial(Material* mat /*= nullptr*/) {
	if (mat == nullptr) {
		mat = g_theResourceManager->GetMaterial("default");
	}
	m_material = mat;
	m_device->UpdateBuffer(m_material_cBuffer.get(), &m_material->m_data);

	// Set Vertex Shader
	u32 numCBuffers = 4U;
	ID3D11Buffer* cBuffers[CONSTANT_BUFFER_SLOT_COUNT] = { nullptr };
	cBuffers[CAMERA_BUFFER_SLOT] = m_camera_cBuffer->m_d3d11Buffer.Get();
	cBuffers[OBJECT_BUFFER_SLOT] = m_obj_cBuffer->m_d3d11Buffer.Get();
	cBuffers[LIGHT_BUFFER_SLOT] = m_light_cBuffer->m_d3d11Buffer.Get();
	cBuffers[MATERIAL_BUFFER_SLOT] = m_material_cBuffer->m_d3d11Buffer.Get();

	m_device->m_d3d11Context->VSSetShader(m_material->m_vs->m_d3d11VertexShader.Get(), nullptr, 0);
	m_device->m_d3d11Context->VSSetConstantBuffers(0U, numCBuffers, cBuffers);

	m_device->m_d3d11Context->RSSetState(m_material->m_rasterizerState->m_d3d11RasterizerState.Get());

	m_device->m_d3d11Context->PSSetShader(m_material->m_ps->m_d3d11PixelShader.Get(), nullptr, 0);
	m_device->m_d3d11Context->PSSetConstantBuffers(0U, numCBuffers, cBuffers);

	u32 numShaderResources = 0U;
	ID3D11ShaderResourceView* srvs[SHADER_RESOURCE_SLOT_COUNT] = { nullptr };
	ID3D11SamplerState* samplers[SAMPLER_SLOT_COUNT] = { nullptr };
	if (m_material->m_diffuse) {
		srvs[numShaderResources] = m_material->m_diffuse->m_srv->m_d3d11SRV.Get();
		samplers[numShaderResources] = m_material->m_diffuse->m_sampler->m_d3d11Sampler.Get();
		numShaderResources++;
	}
	if (m_material->m_normal) {
		srvs[numShaderResources] = m_material->m_normal->m_srv->m_d3d11SRV.Get();
		samplers[numShaderResources] = m_material->m_normal->m_sampler->m_d3d11Sampler.Get();
		numShaderResources++;
	}

	m_device->m_d3d11Context->PSSetShaderResources(0U, numShaderResources, srvs);
	m_device->m_d3d11Context->PSSetSamplers(0U, numShaderResources, samplers);

	m_device->m_d3d11Context->OMSetDepthStencilState(m_material->m_depthStencilState->m_d3d11DepthStencilState.Get(), 0);
	m_device->m_d3d11Context->OMSetBlendState(m_material->m_blendState->m_d3d11BlendState.Get(), nullptr, 0xFFFFFFFF);

}

void ImmediateRenderer::BindCamera(Camera* cam) {
	m_camera = cam;
	m_camera->UpdateIfDirty();
	m_device->UpdateBuffer(m_camera_cBuffer.get(), &m_camera->m_data);

	u32 numViewports = 0U;
	D3D11_VIEWPORT viewports[VIEWPORT_SLOT_COUNT];
	for (int i = 0; i < VIEWPORT_SLOT_COUNT; ++i) {
		if (m_camera->m_viewports[i]) {
			numViewports++;
			viewports[i] = (m_camera->m_viewports[i]->m_d3d11Viewport);
		}
	}

	m_device->m_d3d11Context->RSSetViewports(numViewports, viewports);

	m_device->m_d3d11Context->OMSetRenderTargets(1U, m_camera->m_renderTarget->m_d3d11RTV.GetAddressOf(), m_camera->m_depthTarget->m_d3d11DSV.Get());
}

void ImmediateRenderer::BindLighting(u32 idx, Light* light) {
	light->UpdateIfDirty();
	m_lights[idx] = light->m_data;
	m_device->UpdateBuffer(m_light_cBuffer.get(), m_lights);
}

void ImmediateRenderer::DrawMeshImmediate(Mesh* mesh) {
	// Input Assembler
	if (mesh->m_isFinalized == false) {
		mesh->m_isFinalized = true;
		mesh->m_vertexBuffer = m_device->CreateVertexBuffer(mesh->m_vertices.size() * sizeof(VertexMaster), false, false, mesh->m_vertices.data());
		if(mesh->m_useIndices){
			mesh->m_indexBuffer = m_device->CreateIndexBuffer(mesh->m_triangles.size() * 3 * sizeof(u32), false, mesh->m_triangles.data());
		}
		else{
			mesh->m_indexBuffer = nullptr;
		}

		GUARANTEE_OR_DIE(m_material != nullptr, "Failed to bind material for immediate renderer");
		mesh->m_inputLayout = m_device->CreateInputLayout(VertexMaster::s_layoutDescs, m_material->m_vs);
	}

	u32 strides = sizeof(VertexMaster);
	u32 offsets = 0U;
	m_device->m_d3d11Context->IASetInputLayout(mesh->m_inputLayout->m_d3d11InputLayout.Get());
	m_device->m_d3d11Context->IASetPrimitiveTopology(ToDXPrimitiveType(mesh->m_primitiveType));
	m_device->m_d3d11Context->IASetVertexBuffers(0U, 1U, mesh->m_vertexBuffer->m_d3d11Buffer.GetAddressOf(), &strides, &offsets);
	if (mesh->m_useIndices) {
		m_device->m_d3d11Context->IASetIndexBuffer(mesh->m_indexBuffer->m_d3d11Buffer.Get(), DXGI_FORMAT_R32_UINT, 0U);
	}

	if(mesh->m_useIndices){
		m_device->DrawIndexed(mesh->m_triangles.size() * 3);
	}
	else {
		m_device->Draw(mesh->m_vertices.size());
	}
}