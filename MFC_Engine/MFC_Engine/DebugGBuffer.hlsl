Texture2D gGBuffers[4] : register(t0);
SamplerState gSampler : register(s0);

cbuffer DebugParams : register(b0)
{
    uint nBufferCount;
    uint nGridCols;
    uint nGridRows;
    uint padding;
    uint4 nBufferTypes; // x: buffer0, y: buffer1, z: buffer2, w: buffer3
                        // 0: Albedo, 1: Normal, 2: Position, 3: ID
};

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
    
    uint colIdx = (uint)(uv.x * nGridCols);
    uint rowIdx = (uint)(uv.y * nGridRows);
    uint totalIdx = rowIdx * nGridCols + colIdx;
    
    if (totalIdx >= nBufferCount)
        return float4(0.1f, 0.1f, 0.1f, 1.0f);
        
    float2 localUV = float2(frac(uv.x * nGridCols), frac(uv.y * nGridRows));
    
    float4 color = gGBuffers[NonUniformResourceIndex(totalIdx)].Sample(gSampler, localUV);
    
    uint type = 0;
    if (totalIdx == 0) type = nBufferTypes.x;
    else if (totalIdx == 1) type = nBufferTypes.y;
    else if (totalIdx == 2) type = nBufferTypes.z;
    else if (totalIdx == 3) type = nBufferTypes.w;

    if (type == 1) // Normal
    {
        float3 n = color.xyz;
        if (length(n) < 0.001f) return float4(0, 0, 0, 1);
        return float4(n * 0.5f + 0.5f, 1.0f);
    }
    else if (type == 2) // Position
    {
        return float4(abs(frac(color.xyz * 0.1f)), 1.0f);
    }
    else if (type == 3) // ID
    {
        // Highlight small IDs and special Gizmo IDs
        if (any(color.rgb > 0))
        {
            // If it looks like a Gizmo ID (R=1.0, G=0, B=small)
            if (color.r > 0.9f && color.g < 0.1f)
                return float4(1.0f, 1.0f, 0.0f, 1.0f); // Bright Yellow for Gizmo

            // Amplified visualization for object IDs
            return float4(frac(color.rgb * 50.0f) + 0.2f, 1.0f);
        }
        return float4(0, 0, 0, 1);
    }
    
    return color;
}
