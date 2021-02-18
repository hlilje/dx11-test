struct PS_INPUT {
    float3 _Position    : POSITION;
    float3 _Color       : COLOR;
};

float4 main(PS_INPUT Input) : SV_TARGET {
    return float4(Input._Color, 1.0f);
}
