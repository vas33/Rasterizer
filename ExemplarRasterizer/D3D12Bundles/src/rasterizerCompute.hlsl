//=============================================================================
// Performs a separable Guassian blur with a blur radius up to 5 pixels.
//=============================================================================

struct VertexData
{
	float3 position;
	float3 normal;
	float2 uv;
	float3 tangent;
};

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
	float gTime;
	float gZoom;
	float2 gOffset;
	int gMaxIterationsCount;
	float4 colors[280];
	//TODO pass colors as texture
};

StructuredBuffer<VertexData> vertexBuffer : register(t0);
StructuredBuffer<uint> indexBuffer : register(t1);

RWTexture2D<float4> gOutput : register(u0);

#define N 256

uint mandlebrod(double realIn, double imaginaryIn)
{
	uint maxIterations = gMaxIterationsCount;
	double real = realIn;
	double imaginary = imaginaryIn;

	uint numIterations = 0;
	while (real * real + imaginary * imaginary <= 4.0 && numIterations < maxIterations)
	{
		double  newReal = real * real - imaginary * imaginary;
		double  newImaginary = 2.0 * real * imaginary;

		real = realIn + newReal;
		imaginary = imaginaryIn + newImaginary;

		++numIterations;
	}

	return numIterations;
}

uint getNumIterationsForCoordinate(double x, double y)
{
	float initialRealPartStart = -2.0;
	float initialRealPartEnd = 1.0;
	float initialImPartStart = -1.0;
	float initialImPartEnd = 1.0;

	float realPartStart = initialRealPartStart;
	float realPartEnd = initialRealPartEnd;
	float imPartStart = initialImPartStart;
	float imPartEnd = initialImPartEnd;

	float oldCenter = (initialRealPartStart + initialRealPartEnd) / 2;
	float oldCenterY = (initialImPartStart + initialImPartEnd) / 2;

	realPartStart = realPartStart / gZoom;
	realPartEnd = realPartEnd / gZoom;
	imPartStart = imPartStart / gZoom;
	imPartEnd = imPartEnd / gZoom;

	float newCenter = (realPartStart + realPartEnd) / 2;
	float newCenterY = (imPartStart + imPartEnd) / 2;

	float coompensateX = (oldCenter - newCenter) / 2;
	float coompensateY = (oldCenterY - newCenterY) / 2;

	realPartStart += (gOffset.x - coompensateX);
	realPartEnd += (gOffset.x - coompensateX);
	imPartStart += (gOffset.y - coompensateY);
	imPartEnd += (gOffset.y - coompensateY);


	float widthDifference = realPartEnd - realPartStart;
	float heighDifference = realPartEnd - realPartStart;

	float xCoord = x;
	float yCoord = y;
	//Todo pass image width as constant

	//TODO PASS width height
	float first = realPartStart + (xCoord / 1920.0) * (widthDifference);
	float second = imPartStart + (yCoord / 1080.0) * (heighDifference);

	uint numIterations = mandlebrod(first, second);

	return numIterations;
}


[numthreads(N, 1, 1)]
void CS(int3 groupThreadID : SV_GroupThreadID,
	int3 dispatchThreadID : SV_DispatchThreadID)
{

	float4 	resultColor = float4(0.f, 0.f, 0.f, 1.f);

	gOutput[dispatchThreadID.xy] = resultColor;
}