cbuffer SceneConstantBuffer : register(b0)
{
    float4x4 matWVP;
    float4x4 matWorld;
    float4 objectColorID;
    float4 meshColor;
    float4 lightDir;
    float4 lightColor;
    float4 ambientColor;
};

struct VS_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 objectID : COLOR;
};

PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT result;
    result.position = mul(float4(input.position, 1.0f), matWVP);
    result.objectID = objectColorID;
    return result;
}

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    return input.objectID;
}
