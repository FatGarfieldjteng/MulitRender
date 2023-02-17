struct LightViewProjection
{
    matrix VP;
};

struct Model
{
    matrix M;
};

ConstantBuffer<LightViewProjection> LightViewProjectionCB : register(b0);
ConstantBuffer<Model> ModelCB : register(b1);


struct VSInput
{
    float3 Position : POSITION;
};

struct VSOutput
{
    float4 Position : SV_Position;
};

VSOutput main(VSInput Input)
{
    VSOutput output;

    //matrix ModelViewProjection = mul(ModelCB.M, ViewProjectionCB.VP);
    matrix LightModelViewProjection = mul(LightViewProjectionCB.VP, ModelCB.M);

    output.Position = mul(LightModelViewProjection, float4(Input.Position, 1.0f));

    return output;
}