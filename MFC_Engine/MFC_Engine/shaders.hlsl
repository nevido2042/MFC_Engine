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
    float3 normal : NORMAL;
    float4 color : COLOR;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
};

PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT result;

    result.position = mul(float4(input.position, 1.0f), matWVP);
    
    // Transform normal to world space
    result.normal = mul(input.normal, (float3x3)matWorld);
    result.normal = normalize(result.normal);
    
    result.color = input.color;

    return result;
}

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    if (meshColor.a > 0.0f)
        return meshColor;

    // Normalize normal and light direction
    float3 normal = normalize(input.normal);
    float3 L = normalize(-lightDir.xyz);

    // Lambertian lighting
    float NdotL = max(0.0f, dot(normal, L));

    // Calculate final color
    float3 finalColor = input.color.rgb * (ambientColor.rgb + lightColor.rgb * NdotL);

    return float4(finalColor, input.color.a);
}
