struct GSOutput
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 color : COLOR;
	uint primID : SV_PrimitiveID;
};

struct VertexOut
{
	float2 position : POSITION;
	float2 size : SIZE;
	float4 color : COLOR;
	float4 source : SOURCE;
};

[maxvertexcount(4)]
void main(
	point VertexOut gin[1],
	uint primID : SV_PrimitiveID,
	inout TriangleStream< GSOutput > output
)
{
	float2 TexC[4] =
	{
		float2(gin[0].source.x, gin[0].source.y),
		float2(gin[0].source.x, gin[0].source.w),
		float2(gin[0].source.z, gin[0].source.y),
		float2(gin[0].source.z, gin[0].source.w)
	};

	float4 v[4];
	v[0] = float4(gin[0].position.x, gin[0].position.y-gin[0].size.y, 0.0f, 1.0f);
	v[1] = float4(gin[0].position.x, gin[0].position.y, 0.0f, 1.0f);
	v[2] = float4(gin[0].position.x+gin[0].size.x, gin[0].position.y-gin[0].size.y, 0.0f, 1.0f);
	v[3] = float4(gin[0].position.x+gin[0].size.x, gin[0].position.y, 0.0f, 1.0f);

	GSOutput gout;
	[unroll]
	for (int i = 0; i < 4; i++)
	{
		gout.pos = v[i];
		gout.tex = TexC[i];
		gout.color = gin[0].color;
		gout.primID = primID;

		output.Append(gout);
	}
}