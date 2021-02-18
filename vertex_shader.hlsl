struct VS_INPUT {
    float3 _Position    : POSITION;
    float3 _Color       : COLOR;
};

struct VS_OUTPUT {
    float4 _Position    : SV_POSITION;
    float3 _Color       : COLOR;
};

VS_OUTPUT main(VS_INPUT Input)
{
    VS_OUTPUT Output;

    Output._Position = float4(Input._Position, 1.0f);
    Output._Color = Input._Color;
    
    return Output;
}
