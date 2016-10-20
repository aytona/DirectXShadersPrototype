// GLOBALS
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer CameraBuffer
{
    float3 cameraPosition;
    float padding;
};

// TYPEDEFS
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 viewDirection : TEXCOORD1;
};

// Vertex Shader
PixelInputType LightVertexShader(VertexInputType input)
{
    PixelInputType output;
    float4 worldPosition;

    // Change vector to be 4 units for proper matrix calculations
	input.position.w = 1.0f;

    // Calculate position of vertex
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Store texture coord for pixel shader
	output.tex = input.tex;

	output.normal = mul(input.normal, (float3x3)worldMatrix);

	output.normal = normalize(output.normal);

    worldPosition = mul(input.position, worldMatrix);

    // Determine the viewing direction based on the position of the camera and the position of the vertex in the world
    output.viewDirection = cameraPosition.xyz - worldPosition.xyz;

    output.viewDirection = normalize(output.viewDirection);

	return output;
}