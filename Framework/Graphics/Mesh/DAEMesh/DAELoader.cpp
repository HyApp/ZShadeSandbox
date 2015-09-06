#include "DAELoader.h"
using ZShadeSandboxMesh::DAELoader;
//===============================================================================================================================
//===============================================================================================================================
char array_types[7][15] = {"float_array", "int_array", "bool_array", "Name_array", 
                           "IDREF_array", "SIDREF_array", "token_array"};
char primitive_types[7][15] = {"lines", "linestrips", "polygons", "polylist", 
                               "triangles", "trifans", "tristrips"};
//===============================================================================================================================
DAELoader::DAELoader()
{
	mParser = new XMLParser();
}
//===============================================================================================================================
DAELoader::~DAELoader()
{
}
//===============================================================================================================================
ZShadeSandboxMath::XMMatrix DAELoader::FixMatrix(ZShadeSandboxMath::XMMatrix m)
{
	ZShadeSandboxMath::XMMatrix finalMatrix;
	XMFLOAT3 translate, rotate, scale, qrotate;
	
	m.Decompose(scale, qrotate, translate);
	
	// Flip the yz components of rotation
	rotate.x = qrotate.x;
	rotate.y = qrotate.z;
	rotate.z = qrotate.y;
	
	// Find the angle of rotation
	XMVECTOR axis, Q;
	float angle;
	Q = XMLoadFloat3(&qrotate);
	XMQuaternionToAxisAngle(&axis, &angle, Q);
	
	finalMatrix.Scale(scale);
	finalMatrix.RotationAxis(rotate, -angle);
	finalMatrix.Translate(translate);
	
	return finalMatrix;
}
//===============================================================================================================================
ZShadeSandboxMath::XMMatrix DAELoader::LoadColladaMatrix(float* mat, int offset)
{
	ZShadeSandboxMath::XMMatrix m;
	
	// Put array of floats into matrix.
	for (int i = 0; i <= 15; i++)
	{
		// m.Update(i / 4, i % 4, mat[i]); //Rowbased vs columnbased
		m.Update(i % 4, i / 4, mat[i]);
		
		//m[i % 4, i / 4] = mat[i];
		
		// Make sure the matrix is fully built
		//m.UpdateMatrixFromArray();
		//m.UpdateFields();
	}
	
	return m;
}
//===============================================================================================================================
void DAELoader::Load(string filename)
{
	mParser->Open(filename);
	
	LoadScene();
	LoadMaterial();
	LoadBones();
	CalculateAbsoluteBoneMatrices();
	LoadMesh();
	LoadAnimation();
}
//===============================================================================================================================
void DAELoader::LoadScene()
{
	ArrayElementXMLParser sceneParser(mParser->Element());
	sceneParser.ReadArrayElement("library_visual_scenes", "visual_scene");
	do
	{
		ArrayElementXMLParser nodeParser(sceneParser.ArrayElementIter());
		nodeParser.ReadArrayElement("node");
		do
		{
			DAEScene* scene = new DAEScene();
			
			scene->mName = nodeParser.ReadArrayElementAttribute("name");
			
			ArrayElementXMLParser instGeomParser(nodeParser.ArrayElementIter());
			instGeomParser.ReadArrayElement("instance_geometry");
			do
			{
				scene->mType = DAEScene::SceneType::eGeometry;
				scene->mGeometryLink = nodeParser.ReadArrayElementAttribute("url");
				scene->mGeometryLink = scene->mGeometryLink.erase(0, 1);
				
				
			} while (instGeomParser.ArrayElementNotNull("instance_geometry"));
			
			ArrayElementXMLParser instCamParser(nodeParser.ArrayElementIter());
			instCamParser.ReadArrayElement("instance_camera");
			do
			{
				scene->mType = DAEScene::SceneType::eCamera;
			} while (instCamParser.ArrayElementNotNull("instance_camera"));
			
			ArrayElementXMLParser instLightParser(nodeParser.ArrayElementIter());
			instLightParser.ReadArrayElement("instance_light");
			do
			{
				scene->mType = DAEScene::SceneType::eLight;
			} while (instLightParser.ArrayElementNotNull("instance_light"));
			
			string matrixArray = nodeParser.ReadArrayChildElement("matrix");
			scene->mWorld = FixMatrix(LoadColladaMatrix(ZShadeSandboxGlobal::Convert::ConvertStringToFloatArray(matrixArray), 0));
			
			mScenes.push_back(scene);
		} while (nodeParser.ArrayElementNotNull("node"));
		
		//string filename = string(inputParser.ReadArrayChildElement("init_from"));
		//string imageName = inputParser.ReadArrayElementAttribute("name");
		
	} while (sceneParser.ArrayElementNotNull("visual_scene"));
}
//===============================================================================================================================
void DAELoader::LoadMaterial()
{
	//
	// Find the library images
	//
	
	ArrayElementXMLParser imagesParser(mParser->Element());
	imagesParser.ReadArrayElement("library_images", "image");
	do
	{
		string filename = string(imagesParser.ReadArrayChildElement("init_from"));
		string imageName = imagesParser.ReadArrayElementAttribute("name");
		
		// Add to images dictionary if it does not exist yet
		auto images_iter = mImages.find(imageName);
		bool found = (images_iter != mImages.end());
		if (!found && (filename.length() != 0))
		{
			mImages.insert( make_pair(imageName, filename) );
		}
	} while (imagesParser.ArrayElementNotNull("image"));
	
	//
	// Find the library effects
	//
	
	ArrayElementXMLParser effectsParser(mParser->Element());
	effectsParser.ReadArrayElement("library_effects", "effect");
	do
	{
		DAEEffect* effect = new DAEEffect();
		
		effect->mInitFrom = string(effectsParser.ReadArrayChildElement("init_from"));
		effect->mName = effectsParser.ReadArrayElementAttribute("name");
		
		// If a texture exists then there's no diffuse color
		XMLElement* diffuseNode = effectsParser.ArrayFirstChildElement("diffuse");
		if (diffuseNode)
		{
			XMLElement* colorNode = diffuseNode->FirstChildElement("color");
			effect->mDiffuseColor = ZShadeSandboxGlobal::Convert::ConvertToFloat4(colorNode->GetText());
		}
		
		XMLElement* ambientNode = effectsParser.ArrayFirstChildElement("ambient");
		if (ambientNode)
		{
			XMLElement* colorNode = ambientNode->FirstChildElement("color");
			effect->mAmbientColor = ZShadeSandboxGlobal::Convert::ConvertToFloat4(colorNode->GetText());
		}
		
		XMLElement* specularNode = effectsParser.ArrayFirstChildElement("specular");
		if (specularNode)
		{
			XMLElement* colorNode = specularNode->FirstChildElement("color");
			effect->mSpecularColor = ZShadeSandboxGlobal::Convert::ConvertToFloat4(colorNode->GetText());
		}
		
		XMLElement* shininessNode = effectsParser.ArrayFirstChildElement("shininess");
		if (shininessNode)
		{
			XMLElement* floatNode = shininessNode->FirstChildElement("float");
			effect->fShininess = ZShadeSandboxGlobal::Convert::ConvertStringToT<float>(floatNode->GetText());
		}
		
		XMLElement* transparencyNode = effectsParser.ArrayFirstChildElement("transparency");
		if (transparencyNode)
		{
			XMLElement* floatNode = transparencyNode->FirstChildElement("float");
			effect->fAlhpa = ZShadeSandboxGlobal::Convert::ConvertStringToT<float>(floatNode->GetText());
		}
		
		mEffects.push_back(effect);
	} while (effectsParser.ArrayElementNotNull("effect"));
	
	//
	// Find the library materials
	//
	
	ArrayElementXMLParser materialsParser(mParser->Element());
	materialsParser.ReadArrayElement("library_materials", "material");
	do
	{
		DAEMaterial* material = new DAEMaterial();
		
		material->mName = materialsParser.ReadArrayElementAttribute("name");
		
		XMLElement* instEffectNode = materialsParser.ArrayFirstChildElement("instance_effect");
		
		material->mEffect = instEffectNode->Attribute("url");
		material->mInitFrom = string(instEffectNode->FirstChildElement("init_from")->GetText());
		
		mMaterials.push_back(material);
	} while (materialsParser.ArrayElementNotNull("material"));
}
//===============================================================================================================================
void DAELoader::LoadBones()
{
	XMLElement* visual_scene_element = mParser->Element()->FirstChildElement("library_visual_scenes")->FirstChildElement("visual_scene");
	
	LoadBones(NULL, visual_scene_element);
}
//===============================================================================================================================
void DAELoader::LoadBones(ZShadeSandboxMesh::Bone* parent, XMLElement* element)
{
	ArrayElementXMLParser bonesParser(element);
	bonesParser.ReadArrayElement("node");
	do
	{
		string boneID = bonesParser.ReadArrayElementAttribute("id");
		string boneType = bonesParser.ReadArrayElementAttribute("type");
		
		std::size_t boneIDFound = boneID.find("Bone");
		std::size_t boneTypeFound = boneType.find("JOINT");
		
		if (boneIDFound != std::string::npos || boneTypeFound != std::string::npos)
		{
			ZShadeSandboxMath::XMMatrix matrix;
			matrix.Identity();
			
			// Get all sub nodes for the matrix, these are the inner nodes
			//ArrayElementXMLParser boneSubNodeParser(bonesParser.ArrayElementIter());
			//boneSubNodeParser.ReadArrayElement("node");
			//do
			//{
			//	string subTypeArray = boneSubNodeParser.ReadArrayChildElement("matrix");
			//	matrix = FixMatrix(LoadColladaMatrix(ZShadeSandboxGlobal::Convert::ConvertStringToFloatArray(subTypeArray), 0));
			//} while (boneSubNodeParser.ArrayElementNotNull("node"));
			
			string matrixArray = bonesParser.ReadArrayChildElement("matrix");
			matrix = FixMatrix(LoadColladaMatrix(ZShadeSandboxGlobal::Convert::ConvertStringToFloatArray(matrixArray), 0));
			
			// Create the node
			ZShadeSandboxMesh::Bone* newBone = new ZShadeSandboxMesh::Bone();
			newBone->initialMatrix = matrix;
			newBone->parent = parent;
			newBone->number = mBones.size();
			newBone->id = bonesParser.ReadArrayElementAttribute("sid");
			mBones.push_back(newBone);
			
			// Add the bone to the parent
			if (parent)
			{
				parent->children.push_back(newBone);
			}
			
			LoadBones(newBone, bonesParser.ArrayFirstChildElement("node"));
		}
	} while (bonesParser.ArrayElementNotNull("node"));
}
//===============================================================================================================================
void DAELoader::LoadMesh()
{
	// Iterate through geometry elements
	ArrayElementXMLParser geometryParser(mParser->Element());
	geometryParser.ReadArrayElement("library_geometries", "geometry");
	do
	{
		string geometry_name = geometryParser.ReadArrayElementAttribute("id");
		
		// Iterate through mesh elements
		ArrayElementXMLParser meshElementParser(geometryParser.ArrayElementIter());
		meshElementParser.ReadArrayElement("mesh");
		do
		{
			XMLElement* vertices = meshElementParser.ArrayFirstChildElement("vertices");
			
			// Iterate through the input elements in the vertices
			ArrayElementXMLParser inputParser(vertices);
			inputParser.ReadArrayElement("input");
			do
			{
				string source_name = string(inputParser.ReadArrayElementAttribute("source"));
				source_name = source_name.erase(0, 1);
				
				// Iterate through the source elements
				ArrayElementXMLParser sourceParser(meshElementParser.ArrayElementIter());
				sourceParser.ReadArrayElement("source");
				do
				{
					if (string(sourceParser.ReadArrayElementAttribute("id")) == source_name)
					{
						string semantic = inputParser.ReadArrayElementAttribute("semantic");
						ReadMeshSource(sourceParser.ArrayElementIter());
						// data.map[std::string(input->Attribute("semantic"))] = ReadMeshSource(sourceParser.ArrayElementIter());
					}
				} while (sourceParser.ArrayElementNotNull("source"));
			} while (inputParser.ArrayElementNotNull("input"));
			
			int prim_count = 0, index_count = 0;
			D3D11_PRIMITIVE_TOPOLOGY primitive;
			
			// Find primitive type
			for (int i = 0; i < 7; i++)
			{
				XMLElement* primitive_type = meshElementParser.ArrayFirstChildElement(primitive_types[i]);
				if (primitive_type)
				{
					// Determine number of primitives
					primitive_type->QueryIntAttribute("count", &prim_count);
					
					// Can also get material name
					
					// Determine primitive type and set count
					switch (i)
					{
						case 0:
						{
							primitive = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
							index_count = prim_count * 2;
						}
						break;
						case 1: 
						{
							primitive = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
							index_count = prim_count + 1;
						}
						break;
						case 4:
						{
							primitive = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
							index_count = prim_count * 3;
						}
						break;
						case 5:
						{
							//primitive = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLEFAN;
							index_count = prim_count + 2;
						}
						break;
						case 6:
						{
							primitive = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
							index_count = prim_count + 2;
						}
						break;
						default: std::cout << "Primitive " << primitive_types[i] << " not supported" << std::endl;
					}
					
					// Read indice values
					vector<uint32> indices(index_count);
					
					const char* indice_values = primitive_type->FirstChildElement("p")->GetText();
					BetterString indice_str(indice_values);
					vector<string> indice_tokens = indice_str.split(' ');
					
					int c = 0;
					
					for (int ind_tok = 0; ind_tok < indice_tokens.size(); ind_tok++)
					{
						indices[c++] = (uint32)ZShadeSandboxGlobal::Convert::ConvertStringToInteger(indice_tokens[ind_tok]);
					}
					
					int dummy = 0;
					dummy = 23;
				}
			}
		} while (meshElementParser.ArrayElementNotNull("mesh"));
	} while (geometryParser.ArrayElementNotNull("geometry"));
}
//===============================================================================================================================
void DAELoader::ReadMeshSource(XMLElement* sourceElement)
{
	unsigned int num_vals, stride;
	int check;
	
	uint32 mesh_size;
	uint32 mesh_stride;
	void* mesh_vertex_data;
	
	for (int i = 0; i < 7; i++)
	{
		XMLElement* array_type = sourceElement->FirstChildElement(array_types[i]);
		
		if (array_type)
		{
			// Find number of values
			array_type->QueryUnsignedAttribute("count", &num_vals);
			
			// Find the stride
			check = sourceElement->FirstChildElement("technique_common")->FirstChildElement("accessor")->QueryUnsignedAttribute("stride", &stride);
			
			/*if (check != TIXML_NO_ATTRIBUTE) 
				mesh_stride = stride;
			else
				mesh_stride = 1;*/
			
			char* array_values = (char *)array_type->GetText();
			
			// Initialize mesh data according to data type
			switch (i)
			{
				// Array of floats
				case 0:
				{
					//source_data.type = GL_FLOAT;
					mesh_size *= sizeof(float);
					// Read the float values
					mesh_vertex_data = malloc(num_vals * sizeof(float));
					((float*)mesh_vertex_data)[0] = atof(strtok(array_values, " "));
					for(uint32 index = 1; index < num_vals; index++)
					{
						((float*)mesh_vertex_data)[index] = atof(strtok(NULL, " "));
					}
				}
				break;
				// Array of integers
				case 1:
				{
					//source_data.type = GL_INT;
					mesh_size *= sizeof(int);
					// Read the int values
					mesh_vertex_data = malloc(num_vals * sizeof(int));
					((int*)mesh_vertex_data)[0] = atof(strtok(array_values, " "));
					for(uint32 index = 1; index < num_vals; index++)
					{
						((int*)mesh_vertex_data)[index] = atof(strtok(NULL, " "));
					}
				}
				break;
				// Other
				default: std::cout << "Collada Reader doesn't support mesh data in this format" << std::endl; break;
			}
		}
	}
}
//===============================================================================================================================
void DAELoader::LoadAnimation()
{
	
}
//===============================================================================================================================
void DAELoader::CalculateAbsoluteBoneMatrices()
{
	
}
//===============================================================================================================================