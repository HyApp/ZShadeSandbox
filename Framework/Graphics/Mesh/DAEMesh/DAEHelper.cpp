#include "DAEHelper.h"
//===============================================================================================================================
//===============================================================================================================================
int ZShadeSandboxMesh::DAELoaderMeshData::flipIndexOrder(int oldIndex)
{
	int polygonIndex = oldIndex % 3;
	
	if (polygonIndex == 0)
	{
		return oldIndex;
	}
	else if (polygonIndex == 1)
	{
		return (uint32)(oldIndex + 1);
	}
	else
	{
		return (uint32)(oldIndex - 1);
	}
}
//===============================================================================================================================
std::vector<uint32> ZShadeSandboxMesh::DAELoaderMeshData::optimizeVertexBuffer()
{
	std::vector<DAESkinnedVertex> newVertices;
	std::vector<uint32> newIndices;
	
	// Helper to only search already added newVertices and for checking the
	// old position indices by transforming them into newVertices indices.
	std::vector<int> newVerticesPositions;
	
	// Go over all vertices (indices are currently 1:1 with the vertices)
	for (int num = 0; num < vVertices.size(); num++)
	{
		// Get current vertex
		DAESkinnedVertex currentVertex = vVertices[num];
		bool reusedExistingVertex = false;
		
		// Find out which position index was used, then we can compare
		// all other vertices that share this position. They will not
		// all be equal, but some of them can be merged.
		int sharedPos = vReuseVertexPositions[num];
		
		for (auto otherVertexIndex : vReverseReuseVertexPositions[sharedPos])
		{
			// Only check the indices that have already been added!
			if (otherVertexIndex != num &&
				// Make sure we already are that far in our new index list
				otherVertexIndex < newIndices.size() &&
				// And make sure this index has been added to newVertices yet!
				newIndices[otherVertexIndex] < newVertices.size() &&
				// Then finally compare vertices (this call is slow, but thanks to
				// all the other optimizations we don't have to call it that often)
				currentVertex == newVertices[newIndices[otherVertexIndex]])
			{
				// Reuse the existing vertex, don't add it again, just add another index for it!
				newIndices.push_back((uint32)newIndices[otherVertexIndex]);
				reusedExistingVertex = true;
				break;
			}
		}
		
		if (reusedExistingVertex == false)
		{
			// Add the currentVertex and set it as the current index
			newIndices.push_back((uint32)newVertices.size());
			newVertices.push_back(currentVertex);
		}
	}
	
	// Finally flip order of all triangles to allow us rendering
	// with CullCounterClockwiseFace. because all the data
	// is in CullClockwiseFace format right now!
	for (int num = 0; num < newIndices.size() / 3; num++)
	{
		uint32 swap = newIndices[num * 3 + 1];
		newIndices[num * 3 + 1] = newIndices[num * 3 + 2];
		newIndices[num * 3 + 2] = swap;
	}
	
	//reassign the vertex list with the optimized list
	vVertices = newVertices;
	
	return newIndices;
}
//===============================================================================================================================
void ZShadeSandboxMesh::DAELoaderMeshData::Optimize()
{
	// Optimize vertices first and build index buffer from that
	vIndices = optimizeVertexBuffer();
	iVertexCount = vVertices.size();
	iIndexCount = vIndices.size();
}
//===============================================================================================================================