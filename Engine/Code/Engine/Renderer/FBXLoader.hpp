#pragma once
#include "Engine/Core/type.hpp"
#include "Engine/Renderer/SkeletalMesh.hpp"
#include <string>
#include <fstream>
#include <fbxsdk.h>

class FBXLoader {
public:
	FBXLoader();
	~FBXLoader();

	void LoadFromFile(const std::string& filePath);
	void Print();
	Uptr<SkeletalMesh> CreateSkeletalMesh();

private:
	void ProcessNode(FbxNode* pNode, Joint_t* sNode);
	void ProcessMeshNode(FbxNode* pNode);
	void ProcessSkeletonNode(FbxNode* pNode, Joint_t** sNode);
	
	// Print a node, its attributes, and all its children recursively.
	void PrintNode(FbxNode* pNode, std::fstream& fs);

private:
	FbxManager*		m_sdkManager = nullptr;
	FbxIOSettings*	m_ioSettrings = nullptr;
	FbxImporter*	m_importer = nullptr;
	FbxScene*		m_scene = nullptr;
	FbxMesh*		m_fbxMesh = nullptr;
	std::string		m_filePath;

	Uptr<Joint_t> m_rootJoint;
};