#include "Engine/Renderer/FBXLoader.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Logger.hpp"

// Tab character ("\t") counter
int numTabs = 0;

void PrintTabs(std::fstream& fs) {
	for (int i = 0; i < numTabs; ++i) {
		fs << "--->";
	}
}

/**
* Return a string-based representation based on the attribute type.
*/
FbxString GetAttributeTypeName(FbxNodeAttribute::EType type) {
	switch (type) {
	case FbxNodeAttribute::eUnknown: return "unidentified";
	case FbxNodeAttribute::eNull: return "null";
	case FbxNodeAttribute::eMarker: return "marker";
	case FbxNodeAttribute::eSkeleton: return "skeleton";
	case FbxNodeAttribute::eMesh: return "mesh";
	case FbxNodeAttribute::eNurbs: return "nurbs";
	case FbxNodeAttribute::ePatch: return "patch";
	case FbxNodeAttribute::eCamera: return "camera";
	case FbxNodeAttribute::eCameraStereo: return "stereo";
	case FbxNodeAttribute::eCameraSwitcher: return "camera switcher";
	case FbxNodeAttribute::eLight: return "light";
	case FbxNodeAttribute::eOpticalReference: return "optical reference";
	case FbxNodeAttribute::eOpticalMarker: return "marker";
	case FbxNodeAttribute::eNurbsCurve: return "nurbs curve";
	case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";
	case FbxNodeAttribute::eBoundary: return "boundary";
	case FbxNodeAttribute::eNurbsSurface: return "nurbs surface";
	case FbxNodeAttribute::eShape: return "shape";
	case FbxNodeAttribute::eLODGroup: return "lodgroup";
	case FbxNodeAttribute::eSubDiv: return "subdiv";
	default: return "unknown";
	}
}

/**
 * Print an attribute.
 */
void PrintAttribute(FbxNodeAttribute* pAttribute, std::fstream& fs) {
	if (!pAttribute) return;
	fs << "\n";
	PrintTabs(fs);
	FbxString typeName = GetAttributeTypeName(pAttribute->GetAttributeType());
	FbxString attrName = pAttribute->GetName();
	// Note: to retrieve the character array of a FbxString, use its Buffer() method.
	fs << typeName.Buffer() << " | " << attrName.Buffer();
}

FBXLoader::FBXLoader() {
	// Initialize the SDK manager. This object handles memory management
	m_sdkManager = FbxManager::Create();

	// Create the IO settings object
	m_ioSettrings = FbxIOSettings::Create(m_sdkManager, IOSROOT);
	m_sdkManager->SetIOSettings(m_ioSettrings);

// 	(*(m_sdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_MATERIAL, true);
// 	(*(m_sdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_TEXTURE, true);
// 	(*(m_sdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_LINK, false);
// 	(*(m_sdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_SHAPE, false);
// 	(*(m_sdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_GOBO, false);
// 	(*(m_sdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_ANIMATION, true);
// 	(*(m_sdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);

	// Create an importer using the SDK manager.
	m_importer = FbxImporter::Create(m_sdkManager, "");
}

FBXLoader::~FBXLoader() {
	m_importer->Destroy();
	m_sdkManager->Destroy();
}


void FBXLoader::LoadFromFile(const std::string& filePath) {
	m_filePath = filePath;
	// Use the first argument as the filename for the importer
	if (!m_importer->Initialize(filePath.c_str(), -1, m_sdkManager->GetIOSettings())) {
		ERROR_AND_DIE(Stringf("Call to FbxImporter::Initialize() failed. Error returned: %s\n", m_importer->GetStatus().GetErrorString()).c_str());
	}

	// Create a new scene so that it can be populated by the imported file
	m_scene = FbxScene::Create(m_sdkManager, "myScene");

	// Import the contents of the file into the scene.
	m_importer->Import(m_scene);

	FbxNode* rootNode = m_scene->GetRootNode();
	m_rootJoint = std::make_unique<Joint_t>();
	if (rootNode) {
		for (int i = 0; i < rootNode->GetChildCount(); i++) {
			ProcessNode(rootNode->GetChild(i), m_rootJoint.get());
		}
	}
}

void FBXLoader::ProcessNode(FbxNode* pNode, Joint_t* sNode) {
	for (int i = 0; i < pNode->GetNodeAttributeCount(); ++i) {
		FbxNodeAttribute* attribute = pNode->GetNodeAttributeByIndex(i);
		FbxNodeAttribute::EType type = attribute->GetAttributeType();
		switch (type) {
		case FbxNodeAttribute::eMesh: ProcessMeshNode(pNode); break;
		case FbxNodeAttribute::eSkeleton: ProcessSkeletonNode(pNode, &sNode); break;
		default: break;
		}
	}
	for (int i = 0; i < pNode->GetChildCount(); ++i) {
		ProcessNode(pNode->GetChild(i), sNode);
	}
}

void FBXLoader::ProcessMeshNode(FbxNode* pNode) {
	m_fbxMesh = pNode->GetMesh();
}

void FBXLoader::ProcessSkeletonNode(FbxNode* pNode, Joint_t** sNode) {
	std::string name = pNode->GetName();
	FbxDouble3 fbxTranslation = pNode->LclTranslation.Get();
	FbxDouble3 fbxRotation = pNode->LclRotation.Get();
	FbxDouble3 fbxScaling = pNode->LclScaling.Get();
	Vector3 translation((float)fbxTranslation[0], (float)fbxTranslation[1], (float)fbxTranslation[2]);
	Vector3 rotation((float)fbxRotation[0], (float)fbxRotation[1], (float)fbxRotation[2]);
	Vector3 scaling((float)fbxScaling[0], (float)fbxScaling[1], (float)fbxScaling[2]);
	
	Uptr<Joint_t> newSNode = std::make_unique<Joint_t>(name, translation, rotation, scaling);
	Joint_t* ptr = newSNode.get();
	(*sNode)->AddChild(std::move(newSNode));
	(*sNode) = ptr;
}

void FBXLoader::PrintNode(FbxNode* pNode, std::fstream& fs) {
	PrintTabs(fs);
	const char* nodeName = pNode->GetName();
	FbxDouble3 translation = pNode->LclTranslation.Get();
	FbxDouble3 rotation = pNode->LclRotation.Get();
	FbxDouble3 scaling = pNode->LclScaling.Get();

	// Print the contents of the node
	fs << 
		nodeName << ":" <<
		"[" << translation[0] << "," << translation[1] << "," << translation[2] << "]" <<
		"[" << rotation[0] << "," << rotation[1] << "," << rotation[2] << "]" <<
		"[" << scaling[0] << "," << scaling[1] << "," << scaling[2] << "]";

	// Print the node's attributes
	for (int i = 0; i < pNode->GetNodeAttributeCount(); ++i) {
		PrintAttribute(pNode->GetNodeAttributeByIndex(i), fs);
	}
	numTabs++;
	fs << "\n";
	// Recursively print the children
	for (int j = 0; j < pNode->GetChildCount(); ++j) {
		PrintNode(pNode->GetChild(j), fs);
	}
	numTabs--;
}

void FBXLoader::Print() {
	std::fstream fs;
	fs.open(m_filePath + "_print.txt", std::ios::out | std::ios::trunc);
	int fileMajor, fileMinor, fileRevision;
	m_importer->GetFileVersion(fileMajor, fileMinor, fileRevision);

	fs << "Version:" << fileMajor << "." << fileMinor << "." << fileRevision << "\n";
	// Print the nodes of the scene and their attributes recursively.
	// Note that we are not printing the root node because it should
	// not contain any attributes
	FbxNode* rootNode = m_scene->GetRootNode();
	if (rootNode) {
		for (int i = 0; i < rootNode->GetChildCount(); i++)
			PrintNode(rootNode->GetChild(i), fs);
	}

	fs.close();
}

Uptr<SkeletalMesh> FBXLoader::CreateSkeletalMesh() {
	Uptr<SkeletalMesh> sMesh = std::make_unique<SkeletalMesh>();

	//--------------------------------------------------------------------
	// Move skeletons
	// 
	sMesh->m_rootJoint = std::move(m_rootJoint);

	//--------------------------------------------------------------------
	// Read vertices
	FbxVector4* positions = m_fbxMesh->GetControlPoints();

	int numIndices = m_fbxMesh->GetPolygonVertexCount();
	sMesh->m_indices.reserve(numIndices);
	int* indices = m_fbxMesh->GetPolygonVertices();
	for (int i = 0; i < numIndices; ++i) {
		sMesh->m_indices.emplace_back(indices[i]);
	}

	int vertexId = 0;
	int numPolygons = m_fbxMesh->GetPolygonCount();
	for (int i = 0; i < numPolygons; ++i) {
		int numPolygonSize = m_fbxMesh->GetPolygonSize(i); // 3-triangle 4-quad
		for (int j = 0; j < numPolygonSize; ++j) {
			int controlPointIndex = m_fbxMesh->GetPolygonVertex(i, j);
			Vector3 position((float)positions[controlPointIndex][0], (float)positions[controlPointIndex][2], -(float)positions[controlPointIndex][1]);

			FbxColor fbxColor;
			FbxGeometryElementVertexColor* vertexColorElement = m_fbxMesh->GetElementVertexColor();
			switch (vertexColorElement->GetMappingMode()) {
			case FbxGeometryElement::eByControlPoint:
				switch (vertexColorElement->GetReferenceMode()) {
				case FbxGeometryElement::eDirect:
					fbxColor = vertexColorElement->GetDirectArray().GetAt(controlPointIndex);
					break;
				case FbxGeometryElement::eIndexToDirect:
				{
					int id = vertexColorElement->GetIndexArray().GetAt(controlPointIndex);
					fbxColor = vertexColorElement->GetDirectArray().GetAt(id);
				}
				break;
				default:
					break;
				}
				break;
			case FbxGeometryElement::eByPolygonVertex:
				switch (vertexColorElement->GetReferenceMode()) {
				case FbxGeometryElement::eDirect:
					fbxColor = vertexColorElement->GetDirectArray().GetAt(vertexId);
					break;
				case FbxGeometryElement::eIndexToDirect:
				{
					int id = vertexColorElement->GetIndexArray().GetAt(vertexId);
					fbxColor = vertexColorElement->GetDirectArray().GetAt(id);
				}
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}
			Vector4 color((float)fbxColor.mRed, (float)fbxColor.mGreen, (float)fbxColor.mBlue, 1.f);


			FbxVector2 fbxUV;
			FbxGeometryElementUV* vertexUVElement = m_fbxMesh->GetElementUV();
			switch (vertexUVElement->GetMappingMode()) {
			case FbxGeometryElement::eByControlPoint:
				switch (vertexUVElement->GetReferenceMode()) {
				case FbxGeometryElement::eDirect:
					fbxUV = vertexUVElement->GetDirectArray().GetAt(controlPointIndex);
					break;
				case FbxGeometryElement::eIndexToDirect:
				{
					int id = vertexUVElement->GetIndexArray().GetAt(controlPointIndex);
					fbxUV = vertexUVElement->GetDirectArray().GetAt(id);
				}
				break;
				default:
					break;
				}
				break;
			case FbxGeometryElement::eByPolygonVertex:
				switch (vertexColorElement->GetReferenceMode()) {
				case FbxGeometryElement::eDirect:
					fbxUV = vertexUVElement->GetDirectArray().GetAt(vertexId);
					break;
				case FbxGeometryElement::eIndexToDirect:
				{
					int id = vertexUVElement->GetIndexArray().GetAt(vertexId);
					fbxUV = vertexUVElement->GetDirectArray().GetAt(id);
				}
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}
			Vector2 uv((float)fbxUV[0], 1.f - (float)fbxUV[1]);
			sMesh->m_vertices.emplace_back(position, color, uv);

			vertexId++;
		}// for polygon size
	}// for each polygon

	return std::move(sMesh);
}

// Uptr<Mesh<VertexPCU>> FBXLoader::CreateMeshFromFbxMesh() {
// 	int triangleCount = m_fbxMesh->GetPolygonCount();
// 
// 	FbxVector4* controlPoints = m_fbxMesh->GetControlPoints();
// 
// 	for (int i = 0; i < triangleCount; ++i) {
// 		int vertCount = m_fbxMesh->GetPolygonSize(i);
// 
// 		for (int j = 0; j < vertCount; ++j) {
// 			int controlPointIndex = m_fbxMesh->GetPolygonVertex(i, j);
// 			Vector3 position((float)controlPoints[i][0], (float)controlPoints[i][1], (float)controlPoints[i][2]);
// 
// 
// 		}
// 	}
// }

