cbuffer SceneConstantBuffer : register(b0)
{
    float4x4 matRotation;
};

struct VS_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT result;

    result.position = mul(float4(input.position, 1.0f), matRotation);
    result.color = input.color;

    return result;
}

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    return input.color;
}
