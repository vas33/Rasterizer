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
	float4x4 gProj;
	float gWidth;
	float gHeight;
	//float gTime;
	//float gZoom;
	//float2 gOffset;
	//int gMaxIterationsCount;
	//float4 colors[280];
	//TODO pass colors as texture
};

StructuredBuffer<VertexData> vertexBuffer : register(t0);
StructuredBuffer<uint> indexBuffer : register(t1);
Texture2D        g_txDiffuse : register(t2);

RWTexture2D<float4> gOutput : register(u0);
RWTexture2D<uint> gOutputDepth : register(u1);
SamplerState    g_sampler : register(s0);

#define N 256

float4 convert(float4 projected)
{
	float4 result = float4(0.f, 0.f, 0.f, 0.f);

	//TODO explain why do this
	result.x = projected.x / projected.w;
	result.y= projected.y / projected.w;
	
	//convert to raster space
	result.x = (result.x + 1) * gWidth / 2;
	result.y = (1 - result.y) * gHeight / 2;

	return result;
}

float4 project(float3 vertex)
{
	//float4 projected = float4(vertex.xyz, 1.f);
	float4 projected = mul(float4(vertex, 1.0f), gWorldViewProj);
	projected = convert(projected);

	return projected;
}

float distance(float3 vertex1, float3 vertex2)
{
	return sqrt(pow(2, vertex2.x - vertex2.x) + pow(2, vertex2.y - vertex1.y) + pow(2, vertex2.z - vertex1.z));
}

void drawLine(float3 vertex1, float3 vertex2, float4 color)
{
	float multiplier = 10.f;
	float dist = distance(vertex1, vertex2);
		
	int distInt = (int)(dist * multiplier);
	
	for (int i = 0; i < distInt; ++i)
	{
		float increment = (float)i / multiplier;

		float x = (vertex1.x + (float)(vertex2.x - vertex1.x) * (increment / dist));
		float y = (vertex1.y + (float)(vertex2.y - vertex1.y) * (increment / dist));
		float z = (vertex1.z + (float)(vertex2.z - vertex1.z) * (increment / dist));

		float3 result = float3(x, y, z);
		float4 projected = project(result);


		gOutput[projected.xy] = color;
	}
}

float4 depthToColor(uint value)
{
	if (value < -10000)
	{
		//below -10 white
		return float4(1.f, 1.f, 1.f, 1.f);
	}
	if (value > -10000 && value <=0)
	{
		//below 0 yelow
		return float4(1.f, 1.f, 0.f, 1.f);
	}
	else if (value > 0 && value < 100)
	{
		//between zero and 5 green
		return float4(0.29f, 1.f, 0.f, 1.f);
	}	
	else if(value > 100 && value < 1000)
	{
		return float4(1.f, 0.f, 0.f, 1.f);
	}
	else 
	{
		return float4(0.f, 1.f, 0.f, 1.f);
	}
}

void rasterizeAkaTellusim(float3 p0Unproj, float3 p1Unproj, float3 p2Unproj)
{	
	float3 p10U = p1Unproj - p0Unproj;
	float3 p20U = p2Unproj - p0Unproj;

	float3 p0 = project(p0Unproj);
	float3 p1 = project(p1Unproj);
	float3 p2 = project(p2Unproj);

	float3 p10 = p1 - p0;
	float3 p20 = p2 - p0;

	//Get determinant of matrix
	float det = p20.x * p10.y - p20.y * p10.x;

	//some check for matrix determinant
	if (det >= 0.0f)
	{
		return;
	}

	//find triangle AABB dimensions
	//Get min point 
	float2 min_p = floor(min(min(p0.xy, p1.xy), p2.xy));

	//Get max point
	float2 max_p = ceil(max(max(p0.xy, p1.xy), p2.xy));

	//some check for bounds
	/*[[branch]] if (max_p.x < 0.0f || max_p.y < 0.0f || min_p.x >= surface_size.x || min_p.y >= surface_size.x)
	{
		return;
	}*/

	//Clamp minp need to have surface size 
	//min_p = clamp(min_p, vec2(0.0f), surface_size - 1.0f);
	//max_p = clamp(max_p, vec2(0.0f), surface_size - 1.0f);

	//inverse determinant
	float idet = 1.0f / det;	

	//find bayocentric coordinates by dividing small triangle areas by big triangle area
	//also there is some optimization done here
	float2 texcoord_dx = float2(-p20.y, p10.y) * idet;
	float2 texcoord_dy = float2(p20.x, -p10.x) * idet;

	//start value for textcoord
	float2 texcoord_x = texcoord_dx * (min_p.x - p0.x);
	float2 texcoord_y = texcoord_dy * (min_p.y - p0.y);

	//Iterate pixel by pixel in zone of triangles
	for (float y = min_p.y; y <= max_p.y; y += 1.0f) {

		float2 texcoord = texcoord_x + texcoord_y;

		//Iterate pixel by pixel			
		for (float x = min_p.x; x <= max_p.x; x += 1.0f) {

			if (texcoord.x > -1e-5f && texcoord.y > -1e-5f && texcoord.x + texcoord.y < 1.0f + 1e-5f) {

				//use bayocentric interpolation to find Z for point P
				float z = p10U.z * texcoord.x + p20U.z * texcoord.y + p0Unproj.z;
				
				//convert values to int
				int2 coords = int2(x, y);

				InterlockedMax(gOutputDepth[coords.xy], z, gOutputDepth[coords.xy]);

				gOutput[coords.xy] = depthToColor(gOutputDepth[coords.xy]);
			}

			texcoord += texcoord_dx;
		}

		texcoord_y += texcoord_dy;
	}
}

void colorTriangle(float3 vertex1, float3 vertex2, float3 vertex3)
{
	float4 colorGreen = float4(0.f, 1.f, 0.f, 1.f);
	float4 colorRed = float4(1.f, 0.f, 0.f, 1.f);

	float4 result1 = project(vertex1);
	gOutput[result1.xy] = colorGreen;

	float4 result2 = project(vertex2);
	gOutput[result2.xy] = colorGreen;

	float4 result3 = project(vertex3);
	gOutput[result3.xy] = colorGreen;

	drawLine(vertex1, vertex2, colorRed);
	drawLine(vertex2, vertex3, colorRed);
	drawLine(vertex3, vertex1, colorRed);
}

void colorTriangleTellusim(float3 vertex1, float3 vertex2, float3 vertex3)
{
	float4 colorGreen = float4(0.f, 1.f, 0.f, 1.f);
	float4 colorRed = float4(1.f, 0.f, 0.f, 1.f);

	float4 result1 = project(vertex1);
	//gOutput[result1.xy] = colorGreen;

	float4 result2 = project(vertex2);
	//gOutput[result2.xy] = colorGreen;

	float4 result3 = project(vertex3);
	//gOutput[result3.xy] = colorGreen;

	rasterizeAkaTellusim(
		vertex1,
		vertex2,
		vertex3);

	//drawLine(vertex1, vertex2, colorRed);
	//drawLine(vertex2, vertex3, colorRed);
	//drawLine(vertex3, vertex1, colorRed);
}

//my implementation of rasterization
float areaFunction(float3 a, float3 b, float3 c)
{
	return (c[0] - a[0]) * (b[1] - a[1]) - (c[1] - a[1]) * (b[0] - a[0]);
}

void rasterize(float3 vertex1, float3 vertex2, float3 vertex3, 
	float2 uv1, float2 uv2, float2 uv3)
{
	//project vertices
	float3 p0 = project(vertex1);
	float3 p1 = project(vertex2);
	float3 p2 = project(vertex3);

	//perspective coorection
	vertex1.z = 1 / vertex1.z;
	vertex2.z = 1 / vertex2.z;
	vertex3.z = 1 / vertex3.z;


	//get AABB of triangle
	float2 min_p = floor(min(min(p0.xy, p1.xy), p2.xy));
	//Get max point
	float2 max_p = ceil(max(max(p0.xy, p1.xy), p2.xy));

	float area = areaFunction(p0, p1, p2);

	//iterate over triangle's AABB
	for (float y = min_p.y; y <= max_p.y; y += 1.0f) {

		//float2 texcoord = texcoord_x + texcoord_y;

		//Iterate pixel by pixel			
		for (float x = min_p.x; x <= max_p.x; x += 1.0f)
		{
			float3 p = float3(x + 0.5, y + 0.5, 0.f);

			//get areas of small triangles inside triangle
			//also this is edge function performing edge test
			float w0 = areaFunction(p1, p2, p);
			float w1 = areaFunction(p2, p0, p);
			float w2 = areaFunction(p0, p1, p);
			
			//point is between edges
			if (w0 >= 0 && w1 >= 0 && w2 >= 0)
			{
				//divide areas to get baryctentric coordinates
				w0 /= area;
				w1 /= area;
				w2 /= area;

				//calculate depth
				float z = 1 / (w0 * vertex1.z + w1 * vertex2.z + w2 * vertex3.z);

				//interpolate uv 
				float2 uv = 1 / w0 * uv1 + w1 * uv2 + w2 * uv2;

				int2 coords = int2(p.x, p.y);
				InterlockedMax(gOutputDepth[coords.xy], z, gOutputDepth[coords.xy]);
				gOutput[coords.xy] = depthToColor(gOutputDepth[coords.xy]);
				
				//TODO color, we cannot call sample in CS fix it somehow
				//gOutput[coords.xy] = g_txDiffuse.Sample(g_sampler, uv);				
			}
		}
	}
}

[numthreads(N, 1, 1)]
void CS(int3 groupThreadID : SV_GroupThreadID,
	int3 dispatchThreadID : SV_DispatchThreadID)
{
	float4 	resultColor = float4(1.f, 0.f, 0.f, 1.f);

	gOutput[dispatchThreadID.xy] = resultColor;

	uint index = dispatchThreadID.x;

	uint indexStart = 3 * index;

	uint index1 = indexStart;
	uint index2 = indexStart + 1;
	uint index3 = indexStart + 2;

	index1 = indexBuffer[index1];
	index2 = indexBuffer[index2];
	index3 = indexBuffer[index3];
	
	//colorTriangleTellusim(
	//	vertexBuffer[index1].position,
	//	vertexBuffer[index2].position,
	//	vertexBuffer[index3].position
	//);

	rasterize(vertexBuffer[index1].position,
		vertexBuffer[index2].position,
		vertexBuffer[index3].position, 
		vertexBuffer[index1].uv,
		vertexBuffer[index2].uv, 
		vertexBuffer[index3].uv);
}