Texture2D gPosition : register(t0);
Texture2D gNormal   : register(t1);
Texture2D gAlbedo   : register(t2);

SamplerState gSampler : register(s0);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

PS_INPUT VSMain(uint vertexID : SV_VertexID)
{
    PS_INPUT result;
    result.texcoord = float2((vertexID << 1) & 2, vertexID & 2);
    result.position = float4(result.texcoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    return result;
}

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    float2 uv = input.texcoord;
    
    // 2x2 Grid Layout
    // Top-left: Albedo
    // Top-right: Normal
    // Bottom-left: Position
    // Bottom-right: empty/black
    
    if (uv.y < 0.5f)
    {
        if (uv.x < 0.5f)
        {
            // Albedo
            return gAlbedo.Sample(gSampler, uv * 2.0f);
        }
        else
        {
            // Normal
            float2 normalUV = float2((uv.x - 0.5f) * 2.0f, uv.y * 2.0f);
            float3 n = gNormal.Sample(gSampler, normalUV).xyz;
            // Map normal from [-1, 1] to [0, 1] for visualization
            if (length(n) < 0.1f) return float4(0,0,0,1);
            return float4(n * 0.5f + 0.5f, 1.0f);
        }
    }
    else
    {
        if (uv.x < 0.5f)
        {
            // Position
            float2 posUV = float2(uv.x * 2.0f, (uv.y - 0.5f) * 2.0f);
            float3 p = gPosition.Sample(gSampler, posUV).xyz;
            // Visualize fractional part of position to see changes clearly
            return float4(abs(frac(p)), 1.0f);
        }
        else
        {
            // Empty
            return float4(0.2f, 0.2f, 0.2f, 1.0f);
        }
    }
}
