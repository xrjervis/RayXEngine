#pragma once
#include "Engine/Core/type.hpp"
#include "Engine/Renderer/d3d11/Texture.hpp"
#include "Engine/Renderer/d3d11/ShaderProgram.hpp"
#include "Engine/Renderer/d3d11/Material.hpp"
#include "Engine/Renderer/d3d11/Sampler.hpp"
#include "Engine/Renderer/d3d11/Shader.hpp"
#include "Engine/Renderer/d3d11/SpriteSheet.hpp"
#include "Engine/Math/IntVector2.hpp"
#include <unordered_map>
#include <string>
#include <vector>

class ResourceManager{
public:
	ResourceManager() = default;
	~ResourceManager() = default;

	void				LoadTexture2D(const std::string& name, const std::string& filePath);
	Texture2D*			GetTexture2D(const std::string& name);

	void				LoadShaderProgram(const std::string& name, const std::string& filePath, eShaderType type);
	ShaderProgram*		GetShaderProgram(const std::string& name);

	void				LoadMaterial(const std::string& name, const std::string& filePath);
	Material*			GetMaterial(const std::string& name);

	void				LoadSampler(const std::string& name);
	Sampler*			GetSampler(const std::string& name);

	void				LoadSpriteSheet(const std::string& name, const std::string& filePath, const IntVector2& layout);
	SpriteSheet*		GetSpriteSheet(const std::string& name);
// 
// 	void				LoadSpriteAnimation(const std::string& filePath);
// 	SpriteAnimation*	GetSpriteAnimation(const std::string& name);

private:
	std::unordered_map<std::string, Uptr<Texture2D>>			m_texture2Ds;
	std::unordered_map<std::string, Uptr<ShaderProgram>>		m_shaderPrograms;
	std::unordered_map<std::string, Uptr<Material>>				m_materials;
	std::unordered_map<std::string, Uptr<Sampler>>				m_samplers;
	std::unordered_map<std::string, Uptr<SpriteSheet>>			m_spriteSheets;
// 	std::unordered_map<std::string, std::unique_ptr<SpriteAnimation>>	m_spriteAnims;
};

inline Texture2D* ResourceManager::GetTexture2D(const std::string& name) {
	return m_texture2Ds.at(name).get();
}

inline ShaderProgram* ResourceManager::GetShaderProgram(const std::string& name) {
	return m_shaderPrograms.at(name).get();
}

inline Material* ResourceManager::GetMaterial(const std::string& name) {
	return m_materials.at(name).get();
}

inline Sampler* ResourceManager::GetSampler(const std::string& name) {
	return m_samplers.at(name).get();
}

inline SpriteSheet* ResourceManager::GetSpriteSheet(const std::string& name) {
	return m_spriteSheets.at(name).get();
}