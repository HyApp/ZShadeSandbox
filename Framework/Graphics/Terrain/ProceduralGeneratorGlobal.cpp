#include "ProceduralGeneratorGlobal.h"
#include "ProceduralParameters.h"
#include "ZMath.h"
#include <stdlib.h>
using ZShadeSandboxTerrain::ProceduralGeneratorGlobal;
//===============================================================================================================================
//===============================================================================================================================
vector<ZShadeSandboxTerrain::HeightData> ProceduralGeneratorGlobal::mVerticalOverflow;
vector<ZShadeSandboxTerrain::HeightData> ProceduralGeneratorGlobal::mHorizontalOverflow;
//===============================================================================================================================
ProceduralGeneratorGlobal::ProceduralGeneratorGlobal()
{
}
//===============================================================================================================================
ProceduralGeneratorGlobal::~ProceduralGeneratorGlobal()
{
}
//===============================================================================================================================
void ProceduralGeneratorGlobal::BuildRandomHeightmap
(	int size
,	int minHeight
,	int maxHeight
,	ZShadeSandboxTerrain::HeightmapContainer& outContainer
)
{
	outContainer.Clear();
	
	XMFLOAT2 point(minHeight, maxHeight);
	
	ZShadeSandboxMath::ZMath::RandomSeed();
	
	for (int z = 0; z < size; z++)
	{
		for (int x = 0; x < size; x++)
		{
			float height = ZShadeSandboxMath::ZMath::RandF<float>(point.x, point.y) * 0.01f;
			
			outContainer.UpdateHeight(x, z, height);
		}
	}
}
//===============================================================================================================================
void ProceduralGeneratorGlobal::BuildPerlinNoiseHeightmap(int size, ZShadeSandboxTerrain::HeightmapContainer& outContainer)
{
	outContainer.Clear();
	
	ZShadeSandboxMath::ZMath::RandomSeed();
	
	for (int z = 0; z < size; z++)
	{
		for (int x = 0; x < size; x++)
		{
			float height = PerlinValue(x, z, 1000) * 5.0f;
			
			outContainer.UpdateHeight(x, z, height);
		}
	}
}
//===============================================================================================================================
void ProceduralGeneratorGlobal::BuildFieldNoiseHeightmap(int size, ZShadeSandboxTerrain::HeightmapContainer& outContainer)
{
	outContainer.Clear();
	
	for (int z = 0; z < size; z++)
	{
		for (int x = 0; x < size; x++)
		{
			outContainer.UpdateHeight(x, z, (float)sin(x));
		}
	}
}
//===============================================================================================================================
void ProceduralGeneratorGlobal::BuildDiamondSquare
(	int size
,	int minHeight
,	int maxHeight
,	int waveLength
,	float scale
,	ZShadeSandboxTerrain::EDiamondSquareType::Type diamondSqrType
,	ZShadeSandboxTerrain::HeightmapContainer& outContainer
)
{
	// The diamond-square algorithm by definition works on square maps only with side length equal to power  
	//     * of two plus one. But only two rows/columns of points are enough to interpolate the points near the right
	//     * and bottom borders (top and bottom borders are aligned with the grid, so they can be interpolated in much
	//     * simpler fashion) - thanks to the fact that this is mid-point displacement algorithm. The vertical buffer
	//     * stands for any points to outside the right border, the horizontal buffer then analogously serves as all
	//     * points below the bottom border. The points outside both right and bottom border are represented by last
	//     * point in the vertical buffer.
	
	outContainer.Clear();
	
	int featureSize = (waveLength / 2);
	
	ZShadeSandboxMath::ZMath::RandomSeed();
	
	switch (diamondSqrType)
	{
		case ZShadeSandboxTerrain::EDiamondSquareType::Type::eNormal:
		{
			for (int z = 0; z < size; z += featureSize)
			{
				for (int x = 0; x < size; x += featureSize)
				{
					outContainer.UpdateHeight(x, z, ZShadeSandboxMath::ZMath::RandF<float>(-maxHeight, maxHeight));
				}
			}
			
			while (featureSize > 1)
			{
				int halfStep = featureSize / 2;

				//
				// Perform the Square step
				//

				for (int z = halfStep; z < size + halfStep; z += featureSize)
				{
					for (int x = halfStep; x < size + halfStep; x += featureSize)
					{
						int hs = featureSize / 2;

						// a   b
						//   x
						// c   d

						float a = outContainer.ReadHeight(x - hs, z - hs);
						float b = outContainer.ReadHeight(x + hs, z - hs);
						float c = outContainer.ReadHeight(x - hs, z + hs);
						float d = outContainer.ReadHeight(x + hs, z + hs);

						float height = ((a + b + c + d) / 4) + (ZShadeSandboxMath::ZMath::RandF<float>(-maxHeight, maxHeight) * scale);

						outContainer.UpdateHeight(x, z, height);
					}
				}

				//
				// Perform the Diamond step
				//

				for (int z = 0; z < size; z += featureSize)
				{
					for (int x = 0; x < size; x += featureSize)
					{
						int hs = featureSize / 2;

						//   c
						// a x b
						//   d

						float a = outContainer.ReadHeight((x + halfStep) - hs, z);
						float b = outContainer.ReadHeight((x + halfStep) + hs, z);
						float c = outContainer.ReadHeight((x + halfStep), z - hs);
						float d = outContainer.ReadHeight((x + halfStep), z + hs);

						float height = ((a + b + c + d) / 4) + (ZShadeSandboxMath::ZMath::RandF<float>(-maxHeight, maxHeight) * scale);

						outContainer.UpdateHeight((z * size) + (x + halfStep), height);

						//   c
						// a x b
						//   d

						a = outContainer.ReadHeight(x - hs, (z + halfStep));
						b = outContainer.ReadHeight(x + hs, (z + halfStep));
						c = outContainer.ReadHeight(x, (z + halfStep) - hs);
						d = outContainer.ReadHeight(x, (z + halfStep) + hs);

						height = ((a + b + c + d) / 4) + (ZShadeSandboxMath::ZMath::RandF<float>(-maxHeight, maxHeight) * scale);

						outContainer.UpdateHeight(((z + halfStep) * size) + x, height);
					}
				}

				featureSize /= 2;
				scale /= 2.0f;
			}
		}
		break;
		case ZShadeSandboxTerrain::EDiamondSquareType::Type::eBicubic:
		{
			mVerticalOverflow.resize(size + 1);
			mHorizontalOverflow.resize(size);

			for (int z = 0;; z += featureSize)
			{
				if (z < size)
				{
					for (int x = 0;; x += featureSize)
					{
						float height = ZShadeSandboxMath::ZMath::RandF<float>(-maxHeight, maxHeight);

						if (x < size)
						{
							outContainer.UpdateHeight(x, z, height);
						}
						else
						{
							mVerticalOverflow[z].x = x;
							mVerticalOverflow[z].y = height;
							mVerticalOverflow[z].z = z;
							break;
						}
					}
				}
				else
				{
					for (int x = 0;; x += featureSize)
					{
						float height = ZShadeSandboxMath::ZMath::RandF<float>(-maxHeight, maxHeight);

						if (x < size)
						{
							mHorizontalOverflow[x].x = x;
							mHorizontalOverflow[x].y = height;
							mHorizontalOverflow[x].z = z;
						}
						else
						{
							mVerticalOverflow[size].x = x;
							mVerticalOverflow[size].y = height;
							mVerticalOverflow[size].z = z;
							break;
						}
					}

					break;
				}
			}
			
			// Keep interpolating
			while (featureSize > 1)
			{
				int halfStep = featureSize / 2;

				bool breakZ = false;

				//
				// Perform the Square step
				// Put a randomly generated point into center of each square
				//
				// a   b
				//   x
				// c   d
				//

				for (int z = halfStep; breakZ == false; z += featureSize)
				{
					for (int x = halfStep;; x += featureSize)
					{
						//
						// Prepare 4x4 value matrix for bicubic interpolation
						//

						int x0 = x - halfStep - featureSize;
						int x1 = x - halfStep;
						int x2 = x + halfStep;
						int x3 = x + halfStep + featureSize;

						int z0 = z - halfStep - featureSize;
						int z1 = z - halfStep;
						int z2 = z + halfStep;
						int z3 = z + halfStep + featureSize;

						float data[4][4] =
						{
							{
								ReadOverflowProceduralHeight(x0, z0, size, outContainer),
								ReadOverflowProceduralHeight(x1, z0, size, outContainer),
								ReadOverflowProceduralHeight(x2, z0, size, outContainer),
								ReadOverflowProceduralHeight(x3, z0, size, outContainer),
							},
							{
								ReadOverflowProceduralHeight(x0, z1, size, outContainer),
								ReadOverflowProceduralHeight(x1, z1, size, outContainer),
								ReadOverflowProceduralHeight(x2, z1, size, outContainer),
								ReadOverflowProceduralHeight(x3, z1, size, outContainer),
							},
							{
								ReadOverflowProceduralHeight(x0, z2, size, outContainer),
								ReadOverflowProceduralHeight(x1, z2, size, outContainer),
								ReadOverflowProceduralHeight(x2, z2, size, outContainer),
								ReadOverflowProceduralHeight(x3, z2, size, outContainer),
							},
							{
								ReadOverflowProceduralHeight(x0, z3, size, outContainer),
								ReadOverflowProceduralHeight(x1, z3, size, outContainer),
								ReadOverflowProceduralHeight(x2, z3, size, outContainer),
								ReadOverflowProceduralHeight(x3, z3, size, outContainer),
							},
						};

						//
						// Bicubic Interpolation
						//

						float randHeight = ZShadeSandboxMath::ZMath::RandF<float>(-maxHeight, maxHeight);
						float interpolateHeight = ProceduralGeneratorGlobal::BicubicInterpolation(data) + (featureSize < minHeight ? 0 : randHeight) * scale;

						if (x >= size)
						{
							mVerticalOverflow[min(z, size)].y = interpolateHeight;
							break;
						}
						else if (z >= size)
						{
							mHorizontalOverflow[x].y = interpolateHeight;
							breakZ = true;
						}
						else
						{
							outContainer.UpdateHeight(x, z, interpolateHeight);
						}
					}
				}

				//
				// Perform the Diamond step
				// Add point into middle of each diamond so each square from the square step is composed of 4 smaller squares
				//
				//   c
				// a x b
				//   d
				//

				bool evenRow = true;
				breakZ = false;

				for (int z = 0; breakZ == false; z += halfStep)
				{
					for (int x = evenRow ? halfStep : 0;; x += featureSize)
					{
						//
						// Prepare 4x4 value matrix for bicubic interpolation (this time rotated by 45 degrees)
						//

						int x0 = x - halfStep - featureSize;
						int x1 = x - featureSize;
						int x2 = x - halfStep;
						int x3 = x;
						int x4 = x + halfStep;
						int x5 = x + featureSize;
						int x6 = x + halfStep + featureSize;

						int z0 = z - halfStep - featureSize;
						int z1 = z - featureSize;
						int z2 = z - halfStep;
						int z3 = z;
						int z4 = z + halfStep;
						int z5 = z + featureSize;
						int z6 = z + halfStep + featureSize;

						float data[4][4] =
						{
							{
								ReadOverflowProceduralHeight(x0, z3, size, outContainer),
								ReadOverflowProceduralHeight(x1, z4, size, outContainer),
								ReadOverflowProceduralHeight(x2, z5, size, outContainer),
								ReadOverflowProceduralHeight(x3, z6, size, outContainer),
							},
							{
								ReadOverflowProceduralHeight(x1, z2, size, outContainer),
								ReadOverflowProceduralHeight(x2, z3, size, outContainer),
								ReadOverflowProceduralHeight(x3, z4, size, outContainer),
								ReadOverflowProceduralHeight(x4, z5, size, outContainer),
							},
							{
								ReadOverflowProceduralHeight(x2, z1, size, outContainer),
								ReadOverflowProceduralHeight(x3, z2, size, outContainer),
								ReadOverflowProceduralHeight(x4, z3, size, outContainer),
								ReadOverflowProceduralHeight(x5, z4, size, outContainer),
							},
							{
								ReadOverflowProceduralHeight(x3, z0, size, outContainer),
								ReadOverflowProceduralHeight(x4, z1, size, outContainer),
								ReadOverflowProceduralHeight(x5, z2, size, outContainer),
								ReadOverflowProceduralHeight(x6, z3, size, outContainer),
							},
						};

						//
						// Bicubic Interpolation
						//

						float randHeight = ZShadeSandboxMath::ZMath::RandF<float>(-maxHeight, maxHeight);
						float interpolateHeight = ProceduralGeneratorGlobal::BicubicInterpolation(data) + (featureSize < minHeight ? 0 : randHeight) * scale;

						if (x >= size)
						{
							mVerticalOverflow[min(z, size)].y = interpolateHeight;
							break;
						}
						else if (z >= size)
						{
							mHorizontalOverflow[x].y = interpolateHeight;
							breakZ = true;
						}
						else
						{
							outContainer.UpdateHeight(x, z, interpolateHeight);
						}
					}

					// The X coordinate are shifted by featureSize / 2 in even rows/columns
					evenRow = !evenRow;
				}

				featureSize /= 2;
				scale /= 2.0f;
			}

			if (mVerticalOverflow.size() > 0)
				mVerticalOverflow.clear();

			if (mHorizontalOverflow.size() > 0)
				mHorizontalOverflow.clear();
		}
		break;
	}
}
//===============================================================================================================================
void ProceduralGeneratorGlobal::BuildDiamondSquare
(	int size
,	int minHeight
,	int maxHeight
,	float scale
,	float* amplitudes
,	ZShadeSandboxTerrain::EDiamondSquareType::Type diamondSqrType
,	ZShadeSandboxTerrain::HeightmapContainer& outContainer
)
{
	// The diamond-square algorithm by definition works on square maps only with side length equal to power  
	//     * of two plus one. But only two rows/columns of points are enough to interpolate the points near the right
	//     * and bottom borders (top and bottom borders are aligned with the grid, so they can be interpolated in much
	//     * simpler fashion) - thanks to the fact that this is mid-point displacement algorithm. The vertical buffer
	//     * stands for any points to outside the right border, the horizontal buffer then analogously serves as all
	//     * points below the bottom border. The points outside both right and bottom border are represented by last
	//     * point in the vertical buffer.
	
	ZShadeSandboxMath::ZMath::RandomSeed();
	
	int waveLengthLog2 = (int)ProceduralGeneratorGlobal::log2(maxHeight);
	int featureSize = 1 << waveLengthLog2;
	float amplitude = amplitudes[waveLengthLog2];
	
	switch (diamondSqrType)
	{
		case ZShadeSandboxTerrain::EDiamondSquareType::Type::eNormal:
		{
			for (int z = 0; z < size; z += featureSize)
			{
				for (int x = 0; x < size; x += featureSize)
				{
					outContainer.UpdateHeight(x, z, ZShadeSandboxMath::ZMath::RandF<float>(-amplitude, amplitude));
				}
			}
			
			amplitude = amplitudes[waveLengthLog2 - 1];

			while (featureSize > 1)
			{
				int halfStep = featureSize / 2;

				//
				// Perform the Square step
				//

				for (int z = halfStep; z < size + halfStep; z += featureSize)
				{
					for (int x = halfStep; x < size + halfStep; x += featureSize)
					{
						int hs = featureSize / 2;

						// a   b
						//   x
						// c   d

						float a = outContainer.ReadHeight(x - hs, z - hs);
						float b = outContainer.ReadHeight(x + hs, z - hs);
						float c = outContainer.ReadHeight(x - hs, z + hs);
						float d = outContainer.ReadHeight(x + hs, z + hs);

						float height = ((a + b + c + d) / 4) + (ZShadeSandboxMath::ZMath::RandF<float>(-amplitude, amplitude) * scale);

						outContainer.UpdateHeight(x, z, height);
					}
				}

				//
				// Perform the Diamond step
				//

				for (int z = 0; z < size; z += featureSize)
				{
					for (int x = 0; x < size; x += featureSize)
					{
						int hs = featureSize / 2;

						//   c
						// a x b
						//   d

						float a = outContainer.ReadHeight((x + halfStep) - hs, z);
						float b = outContainer.ReadHeight((x + halfStep) + hs, z);
						float c = outContainer.ReadHeight((x + halfStep), z - hs);
						float d = outContainer.ReadHeight((x + halfStep), z + hs);

						float height = ((a + b + c + d) / 4) + (ZShadeSandboxMath::ZMath::RandF<float>(-amplitude, amplitude) * scale);

						outContainer.UpdateHeight((z * size) + (x + halfStep), height);

						//   c
						// a x b
						//   d

						a = outContainer.ReadHeight(x - hs, (z + halfStep));
						b = outContainer.ReadHeight(x + hs, (z + halfStep));
						c = outContainer.ReadHeight(x, (z + halfStep) - hs);
						d = outContainer.ReadHeight(x, (z + halfStep) + hs);

						height = ((a + b + c + d) / 4) + (ZShadeSandboxMath::ZMath::RandF<float>(-amplitude, amplitude) * scale);

						outContainer.UpdateHeight(((z + halfStep) * size) + x, height);
					}
				}

				featureSize /= 2;
				scale /= 2.0f;
				
				if (featureSize > 1)
				{
					amplitude = amplitudes[ProceduralGeneratorGlobal::log2(featureSize / 2)];
				}
			}
		}
		break;
		case ZShadeSandboxTerrain::EDiamondSquareType::Type::eBicubic:
		{
			mVerticalOverflow.resize(size + 1);
			mHorizontalOverflow.resize(size);

			for (int z = 0;; z += featureSize)
			{
				if (z < size)
				{
					for (int x = 0;; x += featureSize)
					{
						float height = ZShadeSandboxMath::ZMath::RandF<float>(-amplitude, amplitude);

						if (x < size)
						{
							outContainer.UpdateHeight(x, z, height);
						}
						else
						{
							mVerticalOverflow[z].x = x;
							mVerticalOverflow[z].y = height;
							mVerticalOverflow[z].z = z;
							break;
						}
					}
				}
				else
				{
					for (int x = 0;; x += featureSize)
					{
						float height = ZShadeSandboxMath::ZMath::RandF<float>(-amplitude, amplitude);

						if (x < size)
						{
							mHorizontalOverflow[x].x = x;
							mHorizontalOverflow[x].y = height;
							mHorizontalOverflow[x].z = z;
						}
						else
						{
							mVerticalOverflow[size].x = x;
							mVerticalOverflow[size].y = height;
							mVerticalOverflow[size].z = z;
							break;
						}
					}

					break;
				}
			}
			
			amplitude = amplitudes[waveLengthLog2 - 1];

			// Keep interpolating
			while (featureSize > 1)
			{
				int halfStep = featureSize / 2;

				bool breakZ = false;

				//
				// Perform the Square step
				// Put a randomly generated point into center of each square
				//
				// a   b
				//   x
				// c   d
				//

				for (int z = halfStep; breakZ == false; z += featureSize)
				{
					for (int x = halfStep;; x += featureSize)
					{
						//
						// Prepare 4x4 value matrix for bicubic interpolation
						//

						int x0 = x - halfStep - featureSize;
						int x1 = x - halfStep;
						int x2 = x + halfStep;
						int x3 = x + halfStep + featureSize;

						int z0 = z - halfStep - featureSize;
						int z1 = z - halfStep;
						int z2 = z + halfStep;
						int z3 = z + halfStep + featureSize;

						float data[4][4] =
						{
							{
								ReadOverflowProceduralHeight(x0, z0, size, outContainer),
								ReadOverflowProceduralHeight(x1, z0, size, outContainer),
								ReadOverflowProceduralHeight(x2, z0, size, outContainer),
								ReadOverflowProceduralHeight(x3, z0, size, outContainer),
							},
							{
								ReadOverflowProceduralHeight(x0, z1, size, outContainer),
								ReadOverflowProceduralHeight(x1, z1, size, outContainer),
								ReadOverflowProceduralHeight(x2, z1, size, outContainer),
								ReadOverflowProceduralHeight(x3, z1, size, outContainer),
							},
							{
								ReadOverflowProceduralHeight(x0, z2, size, outContainer),
								ReadOverflowProceduralHeight(x1, z2, size, outContainer),
								ReadOverflowProceduralHeight(x2, z2, size, outContainer),
								ReadOverflowProceduralHeight(x3, z2, size, outContainer),
							},
							{
								ReadOverflowProceduralHeight(x0, z3, size, outContainer),
								ReadOverflowProceduralHeight(x1, z3, size, outContainer),
								ReadOverflowProceduralHeight(x2, z3, size, outContainer),
								ReadOverflowProceduralHeight(x3, z3, size, outContainer),
							},
						};

						//
						// Bicubic Interpolation
						//

						float randHeight = ZShadeSandboxMath::ZMath::RandF<float>(-amplitude, amplitude);
						float interpolateHeight = ProceduralGeneratorGlobal::BicubicInterpolation(data) + (featureSize < minHeight ? 0 : randHeight) * scale;

						if (x >= size)
						{
							mVerticalOverflow[min(z, size)].y = interpolateHeight;
							break;
						}
						else if (z >= size)
						{
							mHorizontalOverflow[x].y = interpolateHeight;
							breakZ = true;
						}
						else
						{
							outContainer.UpdateHeight(x, z, interpolateHeight);
						}
					}
				}

				//
				// Perform the Diamond step
				// Add point into middle of each diamond so each square from the square step is composed of 4 smaller squares
				//
				//   c
				// a x b
				//   d
				//

				bool evenRow = true;
				breakZ = false;

				for (int z = 0; breakZ == false; z += halfStep)
				{
					for (int x = evenRow ? halfStep : 0;; x += featureSize)
					{
						//
						// Prepare 4x4 value matrix for bicubic interpolation (this time rotated by 45 degrees)
						//

						int x0 = x - halfStep - featureSize;
						int x1 = x - featureSize;
						int x2 = x - halfStep;
						int x3 = x;
						int x4 = x + halfStep;
						int x5 = x + featureSize;
						int x6 = x + halfStep + featureSize;

						int z0 = z - halfStep - featureSize;
						int z1 = z - featureSize;
						int z2 = z - halfStep;
						int z3 = z;
						int z4 = z + halfStep;
						int z5 = z + featureSize;
						int z6 = z + halfStep + featureSize;

						float data[4][4] =
						{
							{
								ReadOverflowProceduralHeight(x0, z3, size, outContainer),
								ReadOverflowProceduralHeight(x1, z4, size, outContainer),
								ReadOverflowProceduralHeight(x2, z5, size, outContainer),
								ReadOverflowProceduralHeight(x3, z6, size, outContainer),
							},
							{
								ReadOverflowProceduralHeight(x1, z2, size, outContainer),
								ReadOverflowProceduralHeight(x2, z3, size, outContainer),
								ReadOverflowProceduralHeight(x3, z4, size, outContainer),
								ReadOverflowProceduralHeight(x4, z5, size, outContainer),
							},
							{
								ReadOverflowProceduralHeight(x2, z1, size, outContainer),
								ReadOverflowProceduralHeight(x3, z2, size, outContainer),
								ReadOverflowProceduralHeight(x4, z3, size, outContainer),
								ReadOverflowProceduralHeight(x5, z4, size, outContainer),
							},
							{
								ReadOverflowProceduralHeight(x3, z0, size, outContainer),
								ReadOverflowProceduralHeight(x4, z1, size, outContainer),
								ReadOverflowProceduralHeight(x5, z2, size, outContainer),
								ReadOverflowProceduralHeight(x6, z3, size, outContainer),
							},
						};

						//
						// Bicubic Interpolation
						//

						float randHeight = ZShadeSandboxMath::ZMath::RandF<float>(-amplitude, amplitude);
						float interpolateHeight = ProceduralGeneratorGlobal::BicubicInterpolation(data) + (featureSize < minHeight ? 0 : randHeight) * scale;

						if (x >= size)
						{
							mVerticalOverflow[min(z, size)].y = interpolateHeight;
							break;
						}
						else if (z >= size)
						{
							mHorizontalOverflow[x].y = interpolateHeight;
							breakZ = true;
						}
						else
						{
							outContainer.UpdateHeight(x, z, interpolateHeight);
						}
					}

					// The X coordinate are shifted by featureSize / 2 in even rows/columns
					evenRow = !evenRow;
				}

				featureSize /= 2;
				scale /= 2.0f;
				
				if (featureSize > 1)
				{
					amplitude = amplitudes[ProceduralGeneratorGlobal::log2(featureSize / 2)];
				}
			}

			if (mVerticalOverflow.size() > 0)
				mVerticalOverflow.clear();

			if (mHorizontalOverflow.size() > 0)
				mHorizontalOverflow.clear();
		}
		break;
	}
}
//===============================================================================================================================
float ProceduralGeneratorGlobal::ReadOverflowProceduralHeight(int x, int z, int size, ZShadeSandboxTerrain::HeightmapContainer outContainer)
{
	if (x < 0)
	{
		x = -x;
	}
	else if (x >= size)
	{
		return mVerticalOverflow[max(min(z, size), 0)].y;
	}
	
	if (z < 0)
	{
		z = -z;
	}
	else if (z >= size)
	{
		return mHorizontalOverflow[x].y;
	}
	
	return outContainer.ReadHeight(x, z);
}
//===============================================================================================================================
float ProceduralGeneratorGlobal::BicubicInterpolation(float p[4][4])
{
	// Prepare coefficients for the bicubic polynomial.
	float a00 = p[1][1];
	float a01 = -.5*p[1][0] + .5*p[1][2];
	float a02 = p[1][0] - 2.5*p[1][1] + 2 * p[1][2] - .5*p[1][3];
	float a03 = -.5*p[1][0] + 1.5*p[1][1] - 1.5*p[1][2] + .5*p[1][3];
	float a10 = -.5*p[0][1] + .5*p[2][1];
	float a11 = .25*p[0][0] - .25*p[0][2] - .25*p[2][0] + .25*p[2][2];
	float a12 = -.5*p[0][0] + 1.25*p[0][1] - p[0][2] + .25*p[0][3] + .5*p[2][0] - 1.25*p[2][1] + p[2][2] - .25*p[2][3];
	float a13 = .25*p[0][0] - .75*p[0][1] + .75*p[0][2] - .25*p[0][3] - .25*p[2][0] + .75*p[2][1] - .75*p[2][2] + .25*p[2][3];
	float a20 = p[0][1] - 2.5*p[1][1] + 2 * p[2][1] - .5*p[3][1];
	float a21 = -.5*p[0][0] + .5*p[0][2] + 1.25*p[1][0] - 1.25*p[1][2] - p[2][0] + p[2][2] + .25*p[3][0] - .25*p[3][2];
	float a22 = p[0][0] - 2.5*p[0][1] + 2 * p[0][2] - .5*p[0][3] - 2.5*p[1][0] + 6.25*p[1][1] - 5 * p[1][2] + 1.25*p[1][3] + 2 * p[2][0] - 5 * p[2][1] + 4 * p[2][2] - p[2][3] - .5*p[3][0] + 1.25*p[3][1] - p[3][2] + .25*p[3][3];
	float a23 = -.5*p[0][0] + 1.5*p[0][1] - 1.5*p[0][2] + .5*p[0][3] + 1.25*p[1][0] - 3.75*p[1][1] + 3.75*p[1][2] - 1.25*p[1][3] - p[2][0] + 3 * p[2][1] - 3 * p[2][2] + p[2][3] + .25*p[3][0] - .75*p[3][1] + .75*p[3][2] - .25*p[3][3];
	float a30 = -.5*p[0][1] + 1.5*p[1][1] - 1.5*p[2][1] + .5*p[3][1];
	float a31 = .25*p[0][0] - .25*p[0][2] - .75*p[1][0] + .75*p[1][2] + .75*p[2][0] - .75*p[2][2] - .25*p[3][0] + .25*p[3][2];
	float a32 = -.5*p[0][0] + 1.25*p[0][1] - p[0][2] + .25*p[0][3] + 1.5*p[1][0] - 3.75*p[1][1] + 3 * p[1][2] - .75*p[1][3] - 1.5*p[2][0] + 3.75*p[2][1] - 3 * p[2][2] + .75*p[2][3] + .5*p[3][0] - 1.25*p[3][1] + p[3][2] - .25*p[3][3];
	float a33 = .25*p[0][0] - .75*p[0][1] + .75*p[0][2] - .25*p[0][3] - .75*p[1][0] + 2.25*p[1][1] - 2.25*p[1][2] + .75*p[1][3] + .75*p[2][0] - 2.25*p[2][1] + 2.25*p[2][2] - .75*p[2][3] - .25*p[3][0] + .75*p[3][1] - .75*p[3][2] + .25*p[3][3];
	
	// Calculate value of the bicubic polynomial.
	float value = (a00 + a01 * 0.5 + a02 * 0.5 * 0.5 + a03 * 0.5 * 0.5 * 0.5) +
		(a10 + a11 * 0.5 + a12 * 0.5 * 0.5 + a13 * 0.5 * 0.5 * 0.5) * 0.5 +
		(a20 + a21 * 0.5 + a22 * 0.5 * 0.5 + a23 * 0.5 * 0.5 * 0.5) * 0.5 * 0.5 +
		(a30 + a31 * 0.5 + a32 * 0.5 * 0.5 + a33 * 0.5 * 0.5 * 0.5) * 0.5 * 0.5 * 0.5;
	
	return value;
}
//===============================================================================================================================
signed int ProceduralGeneratorGlobal::log2(signed int x)
{
	static double base = log10((double) 2);
	return (signed short) (log10((double) x)/ base);
}
//===============================================================================================================================
float ProceduralGeneratorGlobal::PerlinValue(int x, int y, int random)
{
	int n = x + y * 57 + (rand() % random) * 131;
	n = (n << 13) ^ n;
	return (1.0f - ((n * (SQR(n) * 15731 + 789221) + 1376312589) & 0x7fffffff) * 0.000000000931322574615478515625f);
}
//===============================================================================================================================
