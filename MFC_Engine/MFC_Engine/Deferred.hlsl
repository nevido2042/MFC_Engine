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
    float3 normal   : NORMAL;
    float4 color    : COLOR;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 normal   : NORMAL;
    float4 color    : COLOR;
    float3 worldPos : TEXCOORD0;
};

struct PS_OUTPUT
{
    float4 position : SV_Target0;
    float4 normal   : SV_Target1;
    float4 albedo   : SV_Target2;
};

PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT result;
    result.position = mul(float4(input.position, 1.0f), matWVP);
    result.worldPos = mul(float4(input.position, 1.0f), matWorld).xyz;
    
    // Transform normal to world space
    result.normal = mul(input.normal, (float3x3)matWorld);
    result.normal = normalize(result.normal);
    
    result.color = input.color;
    return result;
}

PS_OUTPUT PSMain(PS_INPUT input)
{
    PS_OUTPUT output;
    
    output.position = float4(input.worldPos, 1.0f);
    output.normal   = float4(normalize(input.normal), 1.0f);
    
    if (meshColor.a > 0.0f)
        output.albedo = meshColor;
    else
        output.albedo = input.color;

    return output;
}
