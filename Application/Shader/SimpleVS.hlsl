struct ModelViewProjection
{
    matrix MVP;
};

ConstantBuffer<ModelViewProjection> ModelViewProjectionCB : register(b0);

struct VSInput
{
    float3 Position : POSITION;
    float3 Color    : COLOR;
};

struct VSOutput
{
    float4 Color    : COLOR;
    float4 Position : SV_Position;
};

VSOutput main(VSInput Input)
{
    VSOutput output;

    output.Position = mul(ModelViewProjectionCB.MVP, float4(Input.Position, 1.0f));
    output.Color = float4(Input.Color, 1.0f);

    return output;
}