struct LightViewProjection
{
    matrix VP;
};

Texture2D shadowMap : register(t0);

SamplerState sampleClamp : register(s0);

ConstantBuffer<LightViewProjection> LightViewProjectionCB : register(b2);

struct PSInput
{
    float4 Position : SV_Position;
    float4 posWorld : POSITION;
    float4 Color : COLOR;
};

float4 shadowFactor(float4 posWorld)
{
    float SHADOW_DEPTH_BIAS = 0.00005f;

    // fragment in light space
    float4 posLightSpace = mul(LightViewProjectionCB.VP, posWorld);
    
    // homo divide
    posLightSpace.xyz /= posLightSpace.w;

    // translate from homogeneous coords to texture coords
    float2 shadowTexCoord = 0.5f * posLightSpace.xy + 0.5f;
    posLightSpace.y = 1.0f - shadowTexCoord.y;

    // depth bias to avoid pixel self-shadowing.
    float lightSpaceDepth = posLightSpace.z - SHADOW_DEPTH_BIAS;

    // find sub-pixel weights.
    float2 shadowMapSize = float2(1024.0f, 1024.0f); 
    float4 subPixelCoords = float4(1.0f, 1.0f, 1.0f, 1.0f);
    subPixelCoords.xy = frac(shadowMapSize * shadowTexCoord);
    subPixelCoords.zw = 1.0f - subPixelCoords.xy;
    float4 bilinearWeights = subPixelCoords.zxzx * subPixelCoords.wwyy;

    // 2x2 percentage closer filtering.
    float2 invShadowMapSize = 1.0f / shadowMapSize;
    float4 shadowDepths;
    shadowDepths.x = shadowMap.Sample(sampleClamp, shadowTexCoord);
    shadowDepths.y = shadowMap.Sample(sampleClamp, shadowTexCoord + float2(invShadowMapSize.x, 0.0f));
    shadowDepths.z = shadowMap.Sample(sampleClamp, shadowTexCoord + float2(0.0f, invShadowMapSize.y));
    shadowDepths.w = shadowMap.Sample(sampleClamp, shadowTexCoord + invShadowMapSize);

    // compute weight
    float4 shadowTests = (shadowDepths >= lightSpaceDepth) ? 1.0f : 0.0f;
    return dot(bilinearWeights, shadowTests);
}

float4 main(PSInput input) : SV_Target
{
    float4 color = input.Color * shadowFactor(input.posWorld);
    
    return color;
}