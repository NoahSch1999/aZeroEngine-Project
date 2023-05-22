cbuffer Color : register(b0)
{
    int pickingColor;
}

struct FragmentInput
{
    float4 position : SV_Position;
};

int main(FragmentInput fragment) : SV_TARGET
{
    return pickingColor;
}