struct ViewProjection
{
    matrix VP;
};

struct Model
{
    matrix M;
};

ConstantBuffer<ViewProjection> ViewProjectionCB : register(b0);
ConstantBuffer<Model> ModelCB : register(b1);


struct VSInput
{
    float3 Position : POSITION;
    float3 Color    : COLOR;
};

struct VSOutput
{
    float4 Position : SV_Position;
    float4 posWorld : POSITION;
    float4 Color    : COLOR;
};

VSOutput main(VSInput Input)
{
    VSOutput output;

    output.posWorld = mul(ModelCB.M, float4(Input.Position, 1.0f));

    matrix ModelViewProjection = mul(ViewProjectionCB.VP, ModelCB.M);

    output.Position = mul(ModelViewProjection, float4(Input.Position, 1.0f));
    output.Color = float4(Input.Color, 1.0f);

    return output;
}