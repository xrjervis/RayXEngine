#pragma once
#include "Engine/Core/type.hpp"
#include "Engine/Renderer/d3d11/Texture.hpp"
#include "Engine/Renderer/d3d11/ShaderProgram.hpp"
#include "Engine/Renderer/d3d11/Material.hpp"
#include "Engine/Renderer/d3d11/Sampler.hpp"
#include "Engine/Renderer/d3d11/Shader.hpp"
#include "Engine/Renderer/d3d11/SpriteSheet.hpp"
#include "Engine/Renderer/SkeletalMesh.hpp"
#include "Engine/Renderer/Animation.hpp"
#include "Engine/Math/IntVector2.hpp"
#include <unordered_map>
#include <string>
#include <vector>

class ResourceManager{
public:
	ResourceManager();
	~ResourceManager();

	void				LoadTexture2D(const std::string& name, const std::string& filePath);
	Texture2D*			GetTexture2D(const std::string& name) const;

	void				LoadShaderProgram(const std::string& name, const std::string& filePath, eShaderType type);
	ShaderProgram*		GetShaderProgram(const std::string& name);

	void				LoadMaterial(const std::string& name, const std::string& filePath);
	Material*			GetMaterial(const std::string& name);

	void				LoadSampler(const std::string& name);
	Sampler*			GetSampler(const std::string& name);

	void				LoadSpriteSheet(const std::string& name, const std::string& filePath, const IntVector2& layout);
	SpriteSheet*		GetSpriteSheet(const std::string& name);

	void				LoadSkeletalMesh(const std::string& name, const std::string& filePath);
	SkeletalMesh*		GetSkeletalMesh(const std::string& name);

	void				LoadAnimation(const std::string& name, const std::string& filePath);
	Animation*			GetAnimation(const std::string& name);
	// 
// 	void				LoadSpriteAnimation(const std::string& filePath);
// 	SpriteAnimation*	GetSpriteAnimation(const std::string& name);

private:
	std::unordered_map<std::string, Uptr<Texture2D>>			m_texture2Ds;
	std::unordered_map<std::string, Uptr<ShaderProgram>>		m_shaderPrograms;
	std::unordered_map<std::string, Uptr<Material>>				m_materials;
	std::unordered_map<std::string, Uptr<Sampler>>				m_samplers;
	std::unordered_map<std::string, Uptr<SpriteSheet>>			m_spriteSheets;
	std::unordered_map<std::string, Uptr<SkeletalMesh>>			m_skeletalMeshes;
	std::unordered_map<std::string, Uptr<Animation>>			m_animations;
};

inline Texture2D* ResourceManager::GetTexture2D(const std::string& name) const{
	if (m_texture2Ds.find(name) == m_texture2Ds.end()) {
		return nullptr;
	}
	return m_texture2Ds.at(name).get();
}

inline ShaderProgram* ResourceManager::GetShaderProgram(const std::string& name) {
	if (m_shaderPrograms.find(name) == m_shaderPrograms.end()) {
		return nullptr;
	}
	return m_shaderPrograms.at(name).get();
}

inline Material* ResourceManager::GetMaterial(const std::string& name) {
	if (m_materials.find(name) == m_materials.end()) {
		return nullptr;
	}
	return m_materials.at(name).get();
}

inline Sampler* ResourceManager::GetSampler(const std::string& name) {
	if (m_samplers.find(name) == m_samplers.end()) {
		return nullptr;
	}
	return m_samplers.at(name).get();
}

inline SpriteSheet* ResourceManager::GetSpriteSheet(const std::string& name) {
	if (m_spriteSheets.find(name) == m_spriteSheets.end()) {
		return nullptr;
	}
	return m_spriteSheets.at(name).get();
}

inline SkeletalMesh* ResourceManager::GetSkeletalMesh(const std::string& name) {
	if (m_skeletalMeshes.find(name) == m_skeletalMeshes.end()) {
		return nullptr;
	}
	return m_skeletalMeshes.at(name).get();
}

inline Animation* ResourceManager::GetAnimation(const std::string& name) {
	if (m_animations.find(name) == m_animations.end()) {
		return nullptr;
	}
	return m_animations.at(name).get();
}