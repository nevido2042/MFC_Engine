cbuffer SceneConstantBuffer : register(b0)
{
    float4x4 matWVP;
    float4x4 matWorld;
    float4 objectColorID;
    float4 meshColor;
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

struct PS_OUTPUT
{
    float4 color : SV_Target0; // Main Render
    float4 id    : SV_Target3; // GBuffer ID for Picking
};

PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT result;
    result.position = mul(float4(input.position, 1.0f), matWVP);
    result.normal = normalize(mul(input.normal, (float3x3)matWorld));
    result.color = input.color;
    return result;
}

PS_OUTPUT PSMain(PS_INPUT input)
{
    PS_OUTPUT output;
    
    // Gizmo Color (Priority: meshColor > vertex color)
    if (meshColor.a > 0.0f)
        output.color = meshColor;
    else
        output.color = input.color;

    // Gizmo ID (Always output for unified picking)
    output.id = objectColorID;

    return output;
}
