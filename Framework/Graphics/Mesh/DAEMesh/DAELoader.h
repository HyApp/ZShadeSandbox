//===============================================================================================================================
// DAELoader
//
// Google: collada exporter c++
// https://github.com/rdiankov/collada-dom
// http://www.codeproject.com/Articles/625701/COLLADA-TinyXML-and-OpenGL
// https://github.com/mattscar/gl_tinyxml_collada
// http://www.andythomason.com/lecture_notes/tm/tm_session4_collada_and_xml.html
//===============================================================================================================================
// History
//
// -Created on 6/24/2015 by Dustin Watson
//===============================================================================================================================
#ifndef __DAELOADER_H
#define __DAELOADER_H
//===============================================================================================================================
//===============================================================================================================================

//
// Includes
//

#include "D3D.h"
#include "Vertex.h"
#include "MeshRenderType.h"
#include "DAEHelper.h"
#include "XMLParser.h"
#include "Bone.h"

//===============================================================================================================================
//===============================================================================================================================
namespace ZShadeSandboxMesh {
class DAELoader
{
public:
	
	DAELoader();
	~DAELoader();
	
	void Load(string filename);
	
private:
	
	void LoadScene();
	void LoadMaterial();
	void LoadBones();
	void LoadMesh();
	void LoadAnimation();
	void CalculateAbsoluteBoneMatrices();
	
	void ReadMeshSource(XMLElement* sourceElement);
	
private:
	
	void LoadBones(ZShadeSandboxMesh::Bone* parent, XMLElement* element);
	
	ZShadeSandboxMath::XMMatrix FixMatrix(ZShadeSandboxMath::XMMatrix m);
	ZShadeSandboxMath::XMMatrix LoadColladaMatrix(float* mat, int offset);
	
private:
	
	std::map<string, string> mImages;
	std::vector<DAEMaterial*> mMaterials;
	std::vector<DAEEffect*> mEffects;
	std::vector<DAELoaderMeshData*> mMeshes;
	std::vector<DAEScene*> mScenes;
	std::vector<ZShadeSandboxMesh::Bone*> mBones;
	
	XMLParser* mParser;
};
}
//===============================================================================================================================
//===============================================================================================================================
#endif//__DAELOADER_H