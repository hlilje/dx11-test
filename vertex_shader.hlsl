struct VS_INPUT {
    float3 _Position    : POSITION;
    float3 _Color       : COLOR;
};

struct VS_OUTPUT {
    float4 _Position    : SV_POSITION;
    float4 _Color       : COLOR;
};

cbuffer ConstantBuffer : register(b0) {
    matrix World;
    matrix View;
    matrix Projection;
};

VS_OUTPUT main(VS_INPUT Input) {
    VS_OUTPUT Output;

    float4 position = float4(Input._Position, 1.0f);

    // Transform the position from object space to homogeneous projection space
    position = mul(position, World);
    position = mul(position, View);
    position = mul(position, Projection);
    Output._Position = position;

    // Just pass through the color data
    Output._Color = float4(Input._Color, 1.0f);

    return Output;
}
