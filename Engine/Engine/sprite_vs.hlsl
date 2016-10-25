struct VertexInputType
{
    float2 position : POSITION;
    float2 size : SIZE;
	float4 color : COLOR;
	float4 source : SOURCE;
};

struct PixelInputType
{
    float2 position : POSITION;
    float2 size : SIZE;
	float4 color : COLOR;
	float4 source : SOURCE;
};

PixelInputType main(VertexInputType input)
{
	PixelInputType output;

	output.position = input.position;
	output.size = input.size;
	output.color = input.color;
	output.source = input.source;

	return output;
}