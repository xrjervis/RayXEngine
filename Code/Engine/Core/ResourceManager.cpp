#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ResourceManager.hpp"
#include "Engine/Renderer/d3d11/RHIInstance.hpp"
#include "ThirdParty/pugixml/pugixml.hpp"

void ResourceManager::LoadTexture2D(const std::string& name, const std::string& filePath) {
	if (m_texture2Ds.find(name) != m_texture2Ds.end()) {
		return;
	}
	RHIDevice* device = g_theRHI->GetDevice();
	std::unique_ptr<Texture2D> texture = device->CreateTexture2DFromFile(filePath);
	texture->m_sampler = GetSampler("nearest");
	m_texture2Ds.insert({ name, std::move(texture) });
}

Texture2D* ResourceManager::GetTexture2D(const std::string& name) {
	return m_texture2Ds.at(name).get();
}

void ResourceManager::LoadShaderProgram(const std::string& name, const std::string& filePath, eShaderType type) {
	if (m_shaderPrograms.find(name) != m_shaderPrograms.end()) {
		return;
	}

	RHIDevice* device = g_theRHI->GetDevice();
	std::unique_ptr<ShaderProgram> shaderProgram;
	switch(type){
	case SHADER_TYPE_VERTEX_SHADER:	shaderProgram = device->CreateVertexShader(filePath);	break;
	case SHADER_TYPE_PIXEL_SHADER:	shaderProgram = device->CreatePixelShader(filePath);	break;
	default: DebuggerPrintf("Unsupported shader type.\n"); break;
	}
	m_shaderPrograms.insert({ name, std::move(shaderProgram) });
}

ShaderProgram* ResourceManager::GetShaderProgram(const std::string& name) {
	return m_shaderPrograms.at(name).get();
}

void ResourceManager::LoadMaterial(const std::string& name, const std::string& filePath) {
	if (m_materials.find(name) != m_materials.end()) {
		return;
	}
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(filePath.c_str());
	pugi::xml_node matNode = doc.child("Material");

	if(result){
		std::unique_ptr<Material> newMaterial = std::make_unique<Material>();
		RHIDevice* device = g_theRHI->GetDevice();

		for(pugi::xml_node p : matNode.children()){
			std::string propertyName = p.name();
			if (propertyName == "int") {
				std::string attrName = p.attribute("name").as_string();
				if (attrName == "useDiffuse") {
					int value = p.attribute("value").as_int();
					newMaterial->m_data.useDiffuse = value;
				}
				else if (attrName == "useNormal") {
					int value = p.attribute("value").as_int();
					newMaterial->m_data.useNormal = value;
				}
				else if (attrName == "useSpecular") {
					int value = p.attribute("value").as_int();
					newMaterial->m_data.useSpecular = value;
				}
				else if (attrName == "useLighting") {
					int value = p.attribute("value").as_int();
					newMaterial->m_data.useLighting = value;
				}
			}
			else if (propertyName == "Texture") {
				std::string attrName = p.attribute("name").as_string();
				if (attrName == "diffuse") {
					std::string value = p.attribute("value").as_string();
					Texture2D* d = GetTexture2D(value);
					d->CreateShaderResourceView();
					newMaterial->m_diffuse = d;
					std::string samplerName = p.attribute("sampler").as_string();
					LoadSampler(samplerName);
					d->m_sampler = GetSampler(samplerName);
				}
				else if (name == "normal") {
					std::string value = p.attribute("value").as_string();
					Texture2D* n = GetTexture2D(value);
					n->CreateShaderResourceView();
					newMaterial->m_normal = n;
					std::string samplerName = p.attribute("sampler").as_string();
					LoadSampler(samplerName);
					n->m_sampler = GetSampler(samplerName);
				}
			}
			else if (propertyName == "ShaderProgram") {
				std::string attrName = p.attribute("name").as_string();
				if (attrName == "vertexShader") {
					std::string value = p.attribute("value").as_string();
					ShaderProgram* vs = GetShaderProgram(value);
					newMaterial->m_vs = vs;
				}
				else if (attrName == "pixelShader") {
					std::string value = p.attribute("value").as_string();
					ShaderProgram* ps = GetShaderProgram(value);
					newMaterial->m_ps = ps;
				}
			}
			else if (propertyName == "RasterizerState") {
				std::string fill = p.attribute("fill").as_string();
				std::string cull = p.attribute("cull").as_string();
				bool frontCC = p.attribute("frontCounterClockwise").as_bool();
				newMaterial->m_rasterizerState = device->CreateRasterizerState(ToFillMode(fill), ToCullMode(cull), frontCC);
			}
			else if (propertyName == "BlendState") {
				std::string blendSrc = p.attribute("blendSrc").as_string();
				std::string blendDst = p.attribute("blendDst").as_string();
				std::string blendOp = p.attribute("blendOp").as_string();
				newMaterial->m_blendState = device->CreateBlendState(ToBlendFactor(blendSrc), ToBlendFactor(blendDst), ToBlendOp(blendOp));
			}
			else if (propertyName == "DepthStencilState") {
				bool depthTestEnable = p.attribute("depthTestEnable").as_bool();
				std::string depthTestComapre = p.attribute("depthTestComapre").as_string("");
				newMaterial->m_depthStencilState = device->CreateDepthStencilState(depthTestEnable, ToCompareFunc(depthTestComapre), STENCIL_OP_KEEP);
			}
		}
		m_materials.insert({ name, std::move(newMaterial) });
	}	
}

Material* ResourceManager::GetMaterial(const std::string& name) {
	return m_materials.at(name).get();
}


void ResourceManager::LoadSampler(const std::string& name) {
	if (m_samplers.find(name) != m_samplers.end()) {
		return;
	}
	std::unique_ptr<Sampler> newSampler = std::make_unique<Sampler>();
	if (name == "linear") {
		RHIDevice* device = g_theRHI->GetDevice();
		newSampler = device->CreateSampler(FILTER_TYPE_MIN_MAG_MIP_LINEAR, WRAP_MODE_LOOP, WRAP_MODE_LOOP, WRAP_MODE_LOOP);
		m_samplers.insert({ name, std::move(newSampler) });
	}
	else if (name == "nearest") {
		RHIDevice* device = g_theRHI->GetDevice();
		newSampler = device->CreateSampler(FILTER_TYPE_MIN_MAG_MIP_POINT, WRAP_MODE_LOOP, WRAP_MODE_LOOP, WRAP_MODE_LOOP);
		m_samplers.insert({ name, std::move(newSampler) });
	}
}

Sampler* ResourceManager::GetSampler(const std::string& name) {
	return m_samplers.at(name).get();
}

void ResourceManager::LoadSpriteSheet(const std::string& name, const std::string& filePath, const IntVector2& layout) {
	if (m_spriteSheets.find(name) != m_spriteSheets.end()) {
		return;
	}

	LoadTexture2D(name, filePath);
	Texture2D* texture = GetTexture2D(name);
	Uptr<SpriteSheet> newSpriteSheet = std::make_unique<SpriteSheet>(texture, layout);
	m_spriteSheets.insert({ name, std::move(newSpriteSheet) });
}

SpriteSheet* ResourceManager::GetSpriteSheet(const std::string& name) {
	return m_spriteSheets.at(name).get();
}

// void ResourceManager::LoadSpriteSheet(const std::string& name, const std::string& filePath, const IntVector2& layout) {
// // 	if(m_spriteSheets.find(name) != m_spriteSheets.end()){
// // 		return;
// // 	}
// // 	LoadTexture(name, filePath);
// // 	Texture* texture = GetTexture(name);
// // 	std::unique_ptr<SpriteSheet> spriteSheet = std::make_unique<SpriteSheet>(texture, layout);
// // 	m_spriteSheets.insert({name, std::move(spriteSheet)});
// }
// 
// SpriteSheet* ResourceManager::GetSpriteSheet(const std::string& name) {
// 	return m_spriteSheets.at(name).get();
// }

// void ResourceManager::LoadSpriteAnimation(const std::string& filePath) {
// 	pugi::xml_document doc;
// 	pugi::xml_parse_result result = doc.load_file(filePath.c_str());
// 	if(result){
// 		for(pugi::xml_node anim : doc.children("SpriteAnimation")){
// 			std::string animName = anim.attribute("name").as_string();
// 			// if the animation already exists
// 			if(m_spriteAnims.find(animName) != m_spriteAnims.end()){
// 				return;
// 			}
// 			std::string spriteSheetName = anim.attribute("spriteSheet").as_string();
// 			float animDuration = anim.attribute("duration").as_float(1.0f);
// 			std::string animModeName = anim.attribute("mode").as_string("playToEnd");
// 			eSpriteAnimMode animMode = ToSpriteAnimationMode(animModeName);
// 			int animStartIndex = anim.attribute("startIndex").as_int(-1);
// 			int animEndIndex = anim.attribute("endIndex").as_int(-1);
// 			std::unique_ptr<SpriteAnimation> spriteAnim = std::make_unique<SpriteAnimation>(spriteSheetName, animDuration, animMode, animStartIndex, animEndIndex);
// 			m_spriteAnims.insert({animName, std::move(spriteAnim)});
// 		}
// 	}
// 	else{
// 		ERROR_AND_DIE(Stringf("Cannot load sprite animation: %s", filePath.c_str()));
// 	}
// }

// SpriteAnimation* ResourceManager::GetSpriteAnimation(const std::string& name) {
// 	return m_spriteAnims.at(name).get();
// }
