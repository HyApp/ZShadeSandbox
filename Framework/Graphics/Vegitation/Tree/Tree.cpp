#include "Tree.h"
#include "TextureManager.h"
#include "Convert.h"
//==================================================================================================================================
//==================================================================================================================================
Tree::Tree(D3D* d3d, TreeParameters tp)
:	mD3DSystem(d3d)
{
	mTreeParameters = new TreeParameters();
	mTreeParameters->treeCount = tp.treeCount;
	mTreeParameters->gameDirectory3D = tp.gameDirectory3D;
	
	// Set the full path to the leaf texture and model
	mTreeParameters->leafTextureName = mTreeParameters->gameDirectory3D->m_textures_path + "\\" + tp.leafTextureName;
	mTreeParameters->treeModelName = mTreeParameters->gameDirectory3D->m_models_path + "\\" + tp.treeModelName;
	
	mShader = new TreeShader(mD3DSystem);
}
//==================================================================================================================================
void Tree::Init()
{
	CreateLeaves();
	CreateMesh();
	CreateBillboards();
	CalculateAABB();
}
//==================================================================================================================================
void Tree::SetWireframe(bool wire)
{
	mShader->Wireframe() = wire;
}
//==================================================================================================================================
void Tree::CreateLeaves()
{
	mLeafAttributes = new ZShadeSandboxMesh::MeshAttributes();
	mLeafAttributes->mD3D = mD3DSystem;
	mLeafAttributes->mVertexCount = 4;
	mLeafAttributes->mIndexCount = 6;
	mLeafAttributes->mTriangleCount = 2;
	mLeafAttributes->mVerticesNormalTexTan.resize(mLeafAttributes->mVertexCount);
	mLeafAttributes->mVerticesNormalTexTan[0] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTexTan(-1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f);
	mLeafAttributes->mVerticesNormalTexTan[1] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTexTan(-1.0f,  1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f);
	mLeafAttributes->mVerticesNormalTexTan[2] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTexTan( 1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f);
	mLeafAttributes->mVerticesNormalTexTan[3] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTexTan( 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f);
	mLeafAttributes->mIndices.resize(mLeafAttributes->mIndexCount);
	mLeafAttributes->mIndices[0] = 0;
	mLeafAttributes->mIndices[1] = 1;
	mLeafAttributes->mIndices[2] = 2;
	mLeafAttributes->mIndices[3] = 0;
	mLeafAttributes->mIndices[4] = 2;
	mLeafAttributes->mIndices[5] = 3;
	mLeafAttributes->mVertexByteWidth = sizeof(ZShadeSandboxMesh::VertexNormalTexTan);
	mLeafAttributes->BuildVertexBuffer(ZShadeSandboxMesh::EVertexType::Type::VT_NormalTexTan);
	mLeafAttributes->BuildIndexBuffer();
	
	mLeafTexture = TextureManager::Instance()->GetTexture(mTreeParameters->leafTextureName);
	
	// Here we create the leaf world matrices, that will be the leafs
	// position and orientation on the tree each individual tree. We will create an array of matrices
	// for the leaves that we will send to the shaders in the cbPerInstance constant buffer
	// This matrix array is used "per tree", so that each tree gets the exact same number of leaves,
	// with the same orientation, position, and scale as all of the other trees
	// Start by initializing the matrix array
	ZShadeSandboxMath::ZMath::RandomSeed();
	XMFLOAT3 fTPos;
	XMMATRIX rotationMatrix;
	XMMATRIX tempMatrix, scale, translation;
	XMVECTOR tempPos;
	InternalTreeRenderParameters itrp;
	
	for (int i = 0; i < NUM_LEAVES_PER_TREE; i++)
	{
		float rotX = (rand() % 2000) / 500.0f;
		float rotY = (rand() % 2000) / 500.0f;
		float rotZ = (rand() % 2000) / 500.0f;
		
		// the rand() function is slightly more biased towards lower numbers, which would make the center of
		// the leaf "mass" be more dense with leaves than the outside of the "sphere" of leaves we are making.
		// We want the outside of the "sphere" of leaves to be more dense than the inside, so the way we do this
		// is getting a distance value between 0 and 4, we then subtract that value from 6, so that the very center
		// does not have any leaves. then below you can see we are checking to see if the distance is greater than 4
		// (because the tree branches are approximately 4 units radius from the center of the tree). If the distance
		// is greater than 4, then we set it at 4, which will make the edge of the "sphere" of leaves more densly
		// populated than the center of the leaf mass
		float distFromCenter = 6.0f - ((rand() % 1000) / 250.0f);	
		
		if (distFromCenter > 4.0f)
		{
			distFromCenter = 4.0f;
		}
		
		// Now we create a vector with the length of distFromCenter, by simply setting it's x component as distFromCenter.
		// We will now rotate the vector, which will give us the "sphere" of leaves after we have rotated all the leaves.
		// We do not want a perfect sphere, more like a half sphere to cover the branches, so we check to see if the y
		// value is less than -1.0f (giving us slightly more than half a sphere), and if it is, negate it so it is reflected
		// across the xz plane
		tempPos = XMVectorSet(distFromCenter, 0.0f, 0.0f, 0.0f);
		rotationMatrix = XMMatrixRotationRollPitchYaw(rotX, rotY, rotZ);
		tempPos = XMVector3TransformCoord(tempPos, rotationMatrix );
		
		if (XMVectorGetY(tempPos) < -1.0f)
		{
			tempPos = XMVectorSetY(tempPos, -XMVectorGetY(tempPos));
		}
		
		// Now we create our leaves "tree" matrix (this is not the leaves "world matrix", because we are not
		// defining the leaves position, orientation, and scale in world space, but instead in "tree" space
		XMStoreFloat3(&fTPos, tempPos);
		
		scale = XMMatrixScaling( 0.25f, 0.25f, 0.25f );
		translation = XMMatrixTranslation(fTPos.x, fTPos.y + 8.0f, fTPos.z );
		tempMatrix = scale * rotationMatrix * translation;
		
		// To make things simple, we just store the matrix directly into our cbPerInst structure
		itrp.g_LeafOnTree[i] = tempMatrix;
	}
	
	// Finally create the leaf space matrix array, if animating the leaves then this will need to be updated every frame.
	mShader->InitializeLeafBuffer(itrp);
}
//==================================================================================================================================
void Tree::CreateMesh()
{
	mTree = new ZShadeSandboxMesh::OBJMesh(mD3DSystem, mTreeParameters->gameDirectory3D);
	mTree->Load(mTreeParameters->treeModelName, false, false);
	
	// Load a custom shader so that the tree obj mesh will use the tree shader
	mTree->HasCustomShader() = true;
	mTree->CustomShader() = mShader;
	
	// Setup the tree instance positions
	std::vector<XMFLOAT3> instPositions(mTreeParameters->treeCount);
	XMVECTOR tempPos;
	ZShadeSandboxMath::ZMath::RandomSeed();
	
	std::vector<XMFLOAT3> instRotations(mTreeParameters->treeCount);
	std::vector<XMFLOAT3> instScales(mTreeParameters->treeCount);
	
	// We are just creating random positions for the trees, between the positions of (-100, 0, -100) to (100, 0, 100)
	// then storing the position in our instanceData array
	for(int i = 0; i < mTreeParameters->treeCount; i++)
	{
		float randX = ((float)(rand() % 2000) / 10) - 100;
		float randZ = ((float)(rand() % 2000) / 10) - 100;
		
		tempPos = XMVectorSet(randX, 0.0f, randZ, 0.0f);
		
		XMStoreFloat3(&instPositions[i], tempPos);
		
		instRotations[i] = XMFLOAT3(0, 0, 0);
		instScales[i] = XMFLOAT3(1, 1, 1);
	}
	
	// Set these instance positions into the obj model for the tree
	mTree->AddInstanceMatrix(instPositions, instRotations, instScales);
	
	// The leaf cloud has the same instance positions as the tree so leaves can be rendered with the tree
	mLeafAttributes->AddInstanceMatrix(instPositions, instRotations, instScales);
}
//==================================================================================================================================
void Tree::CreateBillboards()
{
	// Billboards only need one point and do not need an index buffer
	
	mBillboardAttributes = new ZShadeSandboxMesh::MeshAttributes();
	mBillboardAttributes->mD3D = mD3DSystem;
	mBillboardAttributes->mVertexCount = 1;
	mBillboardAttributes->mTriangleCount = 2;
	mBillboardAttributes->mVerticesNormalTexTan.resize(mBillboardAttributes->mVertexCount);
	mBillboardAttributes->mVerticesNormalTexTan[0] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTexTan(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	mBillboardAttributes->mVertexByteWidth = sizeof(ZShadeSandboxMesh::VertexNormalTexTan);
	mBillboardAttributes->BuildVertexBuffer(ZShadeSandboxMesh::EVertexType::Type::VT_NormalTexTan);
	
	// Create the billboard instance buffer which will be filled during rendering
	mBillboardAttributes->mInstanceCount = mTreeParameters->treeCount;
	mBillboardAttributes->BuildInstanceBuffer();
}
//==================================================================================================================================
int Tree::TriangleCount() const
{
	int leafTriangleCount = mLeafAttributes->mTriangleCount * NUM_LEAVES_PER_TREE * mTreeParameters->treeCount;
	int billboardTriangleCount = mBillboardAttributes->mTriangleCount * iNumBillboardTreesToDraw;
	int treeTriangleCount = mTree->Attributes()->mTriangleCount * mTreeParameters->treeCount;
	
	return leafTriangleCount + billboardTriangleCount + treeTriangleCount;
}
//==================================================================================================================================
int Tree::RenderedTriangleCount() const
{
	int leafTriangleCount = mLeafAttributes->mTriangleCount * NUM_LEAVES_PER_TREE * iNumTreesToDraw;
	int billboardTriangleCount = mBillboardAttributes->mTriangleCount * iNumBillboardTreesToDraw;
	int treeTriangleCount = mTree->Attributes()->mTriangleCount * iNumTreesToDraw;
	
	return leafTriangleCount + billboardTriangleCount + treeTriangleCount;
}
//==================================================================================================================================
string Tree::TriangleCountString() const
{
	return ZShadeSandboxGlobal::Convert::ConvertToString<int>(TriangleCount());
}
//==================================================================================================================================
string Tree::RenderedTriangleCountString() const
{
	return ZShadeSandboxGlobal::Convert::ConvertToString<int>(RenderedTriangleCount());
}
//==================================================================================================================================
string Tree::TreeCountString() const
{
	return ZShadeSandboxGlobal::Convert::ConvertToString<int>(mTreeParameters->treeCount);
}
//==================================================================================================================================
string Tree::TreesRenderedString() const
{
	return ZShadeSandboxGlobal::Convert::ConvertToString<int>(iNumTreesToDraw);
}
//==================================================================================================================================
string Tree::BillboardsRenderedString() const
{
	return ZShadeSandboxGlobal::Convert::ConvertToString<int>(iNumBillboardTreesToDraw);
}
//==================================================================================================================================
string Tree::TreeStats()
{
	string stats = "";
	
	stats += "Tree Count: ";
	stats += TreeCountString();
	stats += "\n";
	stats += "Trees Rendered: ";
	stats += TreesRenderedString();
	stats += "\n";
	stats += "Tree Billboards Rendered: ";
	stats += BillboardsRenderedString();
	stats += "\n";
	stats += "Total Tree Triangles: ";
	stats += TriangleCountString();
	stats += "\n";
	stats += "Total Tree Triangles Rendered: ";
	stats += RenderedTriangleCountString();
	stats += "\n";
	
	return stats;
}
//==================================================================================================================================
void Tree::CalculateAABB()
{
	// Since the AABB was already calculated for the tree mesh then get it
	mTreeAABB = *mTree->GetAABB();
	
	// Since the leaves go out further than the branches of the tree, we need to modify the bounding box to take
	// into account the leaves radius of 4 units from the center of the tree. We add the extra .25 since we scale
	// the leaves down to 25% of their original size. The leaves "sphere" mass is moved up 8 units so the leaves 
	// are at the top of the tree, so we will set the max y to 12 (8 + 4)
	mTreeAABB.vMin.x = -4.25f;
	mTreeAABB.vMin.z = -4.25f;
	mTreeAABB.vMax.x = 4.25f;
	mTreeAABB.vMax.z = 4.25f;
	mTreeAABB.vMax.y = 12.25f;
}
//==================================================================================================================================
void Tree::UpdateBillboardInstances(XMFLOAT3 cameraPos, vector<ZShadeSandboxMesh::InstanceMatrix>& instanceMatrix)
{
	int numFullyDrawnTrees = 0;
	iNumBillboardTreesToDraw = 0;
	
	ZShadeSandboxMath::XMMath3 camPos(cameraPos.x, cameraPos.y, cameraPos.z);
	
	for (int i = 0; i < mTreeParameters->treeCount; ++i)
	{
		// We only want the distance on the xz plane between the tree and camera,
		// in case that the camera happens to be above the trees, so that the trees
		// would not look flat while we are above them
		
		ZShadeSandboxMath::XMMath3 instPos(instanceMatrix[i].position.x, instanceMatrix[i].position.y, instanceMatrix[i].position.z);
		
		ZShadeSandboxMath::XMMath3 treeToCam = camPos - instPos;
		
		treeToCam.y = 0.0f;
		
		float treeToCamDist = treeToCam.Length();
		
		// If tree is further than 200 units from camera, make it a billboard
		if (treeToCamDist < 200.0f)
		{
			instanceMatrix[numFullyDrawnTrees].position = instanceMatrix[i].position;
			
			numFullyDrawnTrees++;
		}
		else
		{
			mBillboardAttributes->mInstanceMatrix[iNumBillboardTreesToDraw].position = instanceMatrix[i].position;
			
			iNumBillboardTreesToDraw++;
		}
	}
	
	iNumTreesToDraw = numFullyDrawnTrees;
	
	mBillboardAttributes->AddInstanceMatrix(mBillboardAttributes->mInstanceMatrix);
}
//==================================================================================================================================
void Tree::SaveBillboardTexture(ZShadeSandboxMesh::MeshRenderParameters mrp)
{
	std::string billboardTextureName = mTreeParameters->gameDirectory3D->m_textures_path + "\\treeBillTexture.png";
	
	// This does not use the main camera, it will setup a ortho projection and view matrix
	// based on the tree's AABB and size
	
	mrp.useInstancing = true;
	
	// We will store the billboards width and height
	float treeBillWidth = mTreeAABB.vMax.x - mTreeAABB.vMin.x; 
	float treeBillHeight = mTreeAABB.vMax.y - mTreeAABB.vMin.y;
	
	mShader->BillTreeWidth(treeBillWidth);
	mShader->BillTreeHeight(treeBillHeight);
	
	if (mrp.useInstancing)
	{
		mLeafAttributes->SetBuffersInstanced(mrp.renderType);
		mrp.world = XMMatrixIdentity();
	}
	else
	{
		mLeafAttributes->SetBuffers(mrp.renderType);
		mrp.world = mLeafAttributes->WorldXM();
	}
	
	// Only draw a single tree's worth of leaves
	mrp.instanceCount = NUM_LEAVES_PER_TREE;
	
	mShader->RenderAndSaveBillboard(
		mLeafAttributes->mIndexCount,
		mrp,
		mLeafTexture,
		mTreeAABB,
		mTree,
		billboardTextureName
	);
	
	// Need to reset the main viewport for rendering since it was changed
	mD3DSystem->ResetViewport();
	mD3DSystem->SetBackBufferRenderTarget();
	
	// Now load the billboard texture for the tree
	mBillboardTexture = TextureManager::Instance()->GetTexture(billboardTextureName);
}
//==================================================================================================================================
void Tree::CullAABB(XMFLOAT4 frustumPlanes[6], vector<ZShadeSandboxMesh::InstanceMatrix> instanceMatrix)
{
	// Assuming that nothing is seen
	iNumTreesToDraw = 0;
	
	vector<ZShadeSandboxMesh::InstanceMatrix> tempInstData(mTreeParameters->treeCount);
	
	bool cull = false;
	
	for (int i = 0; i < mTreeParameters->treeCount; ++i)
	{
		cull = false;
		
		for (int planeID = 0; planeID < 6; planeID++)
		{
			XMFLOAT4 plane = frustumPlanes[planeID];
			XMFLOAT4 planeNormal = XMFLOAT4(plane.x, plane.y, plane.z, 0.0f);
			float planeConst = plane.w;
			
			// Check each axis to get the AABB vertex furthest away from the direction the plane is facing
			XMFLOAT3 axisVertex;
			
			// Check X-Axis
			if (plane.x < 0.0f)
			{
				axisVertex.x = mTreeAABB.vMin.x + instanceMatrix[i].position.x;
			}
			else
			{
				axisVertex.x = mTreeAABB.vMax.x + instanceMatrix[i].position.x;
			}
			
			// Check Y-Axis
			if (plane.y < 0.0f)
			{
				axisVertex.y = mTreeAABB.vMin.y + instanceMatrix[i].position.y;
			}
			else
			{
				axisVertex.y = mTreeAABB.vMax.y + instanceMatrix[i].position.y;
			}
			
			// Check Z-Axis
			if (plane.z < 0.0f)
			{
				axisVertex.z = mTreeAABB.vMin.z + instanceMatrix[i].position.z;
			}
			else
			{
				axisVertex.z = mTreeAABB.vMax.z + instanceMatrix[i].position.z;
			}
			
			// Now we get the signed distance from the AABB vertex that's furthest down the frustum planes normal,
			// and if the signed distance is negative, then the entire bounding box is behind the frustum plane,
			// which means that it should be culled.
			
			ZShadeSandboxMath::XMMath3 normal(planeNormal.x, planeNormal.y, planeNormal.z);
			ZShadeSandboxMath::XMMath3 axis(axisVertex.x, axisVertex.y, axisVertex.z);
			
			if ((normal.Dot(axis) + planeConst) < 0.0f)
			{
				cull = true;
				
				// Skip remaining planes to check and move on to next tree
				break;
			}
		}
		
		if (!cull)
		{
			// Set the treesToDrawIndex in the constant buffer. We are rearranging the tree instance positions, so that the trees
			// that will be drawn have their positions first. This way, when the GPU loops through the instances, it will first
			// get all the tree positions that we want to draw. We are not going to have the GPU draw all 4000 trees, only the couple
			// that are in the view frustum, so we want those tree positions to be the first ones in the instance buffer array
			tempInstData[iNumTreesToDraw].position = instanceMatrix[i].position;
			
			// Add one to the number of trees to draw
			iNumTreesToDraw++;
		}
	}
	
	// Update our instance buffer with our new (newly ordered) array of tree instance data (positions)
	mTree->AddInstanceMatrix(tempInstData);
}
//==================================================================================================================================
void Tree::Render(ZShadeSandboxMesh::MeshRenderParameters mrp, XMFLOAT4 frustumPlanes[6])
{
	if (mrp.camera == NULL) return;
	
	mrp.useInstancing = true;
	
	// We will store the billboards width and height
	float treeBillWidth = mTreeAABB.vMax.x - mTreeAABB.vMin.x; 
	float treeBillHeight = mTreeAABB.vMax.y - mTreeAABB.vMin.y;
	
	mShader->BillTreeWidth(treeBillWidth);
	mShader->BillTreeHeight(treeBillHeight);
	
	// Clip the trees that are not seen
	CullAABB(frustumPlanes, mTree->InstanceMatrices());
	
	//
	// Update the billboards so that any tree that is out of distance from the camera will become
	// a billboard image and will not be as expensive in rendering
	//
	
	UpdateBillboardInstances(mrp.camera->Position(), mTree->InstanceMatrices());
	
	// Update the obj mesh tree instance positions since they might have changed from the camera moving around
	// since a full size tree can become a billboard
	mTree->AddInstanceMatrix(mTree->InstanceMatrices());
	
	//
	// Draw the tree billboards
	//
	
	if (mrp.useInstancing)
	{
		mBillboardAttributes->SetBuffersInstanced(ZShadeSandboxMesh::ERenderType::Type::ePointList);
		mrp.world = XMMatrixIdentity();
	}
	else
	{
		mBillboardAttributes->SetBuffers(ZShadeSandboxMesh::ERenderType::Type::ePointList);
		mrp.world = mBillboardAttributes->WorldXM();
	}
	
	mrp.instanceCount = iNumBillboardTreesToDraw;
	
	iNumBillboardTrianglesToDraw = 2 * mrp.instanceCount;
	
	mShader->RenderBillboards(
		mrp,
		mBillboardTexture
	);
	
	//
	// Draw the leaves
	//
	
	if (mrp.useInstancing)
	{
		mLeafAttributes->SetBuffersInstanced(mrp.renderType);
		mrp.world = XMMatrixIdentity();
	}
	else
	{
		mLeafAttributes->SetBuffers(mrp.renderType);
		mrp.world = mLeafAttributes->WorldXM();
	}
	
	mrp.instanceCount = NUM_LEAVES_PER_TREE * iNumTreesToDraw;
	
	iNumLeafTrianglesToDraw = 2 * mrp.instanceCount;
	
	mShader->RenderLeaves(
		mLeafAttributes->mIndexCount,
		mrp,
		mLeafTexture
	);
	
	//
	// Draw the tree model which will use the same tree shader since it has a custom shader
	// This will be the full size tree
	//
	
	mrp.instanceCount = iNumTreesToDraw;
	
	iNumTreeTrianglesToDraw = mTree->Attributes()->mTriangleCount * mrp.instanceCount;
	
	mTree->Render(mrp);
}
//==================================================================================================================================