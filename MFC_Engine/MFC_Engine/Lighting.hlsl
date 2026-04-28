Texture2D gPosition : register(t0);
Texture2D gNormal   : register(t1);
Texture2D gAlbedo   : register(t2);

SamplerState gSampler : register(s0);

cbuffer LightConstantBuffer : register(b0)
{
    float4x4 matWVP;
    float4x4 matWorld;
    float4 objectColorID;
    float4 meshColor;
    float4 lightDir;
    float4 lightColor;
    float4 ambientColor;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

PS_INPUT VSMain(uint vertexID : SV_VertexID)
{
    PS_INPUT result;
    // Generate full-screen triangle
    result.texcoord = float2((vertexID << 1) & 2, vertexID & 2);
    result.position = float4(result.texcoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    return result;
}

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    float4 albedo = gAlbedo.Sample(gSampler, input.texcoord);
    float3 normal = gNormal.Sample(gSampler, input.texcoord).xyz;
    float3 position = gPosition.Sample(gSampler, input.texcoord).xyz;

    // Check if background (normal is 0)
    if (length(normal) < 0.1f) 
    {
        return albedo;
    }

    float3 N = normalize(normal);
    float3 L = normalize(-lightDir.xyz);

    float NdotL = max(0.0f, dot(N, L));

    float3 finalColor = albedo.rgb * (ambientColor.rgb + lightColor.rgb * NdotL);

    return float4(finalColor, albedo.a);
}
