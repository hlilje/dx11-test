struct PS_INPUT {
    float3 _Position    : POSITION;
    float4 _Color       : COLOR;
};

float4 main(PS_INPUT Input) : SV_TARGET {
    return Input._Color;
}
