Texture2D uTexDiffuse1 : register(t0);
SamplerState uTex1Sampler : register(s0);

Texture2D uTexDiffuse2 : register(t1);
SamplerState uTex2Sampler : register(s1);

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
	float2 tex0 : TEXCOORD0;
	float2 tex1 : TEXCOORD1;
};

cbuffer cbPerFrame
{
	float uLightMapEnable;
	float4 uSelectedColor;
	float uSelected;
};


float4 main(PS_INPUT input) : SV_TARGET
{
	float4 color1 = uTexDiffuse1.Sample(uTex1Sampler, input.tex0);
	float4 color2 = uTexDiffuse2.Sample(uTex2Sampler, input.tex1);
	float4 color3 = color1 * color2 * input.color;
	color3.a = color1.a * input.color.a; 
	if(round(uLightMapEnable)==0) {
		color3 = color1 * input.color;
	}

	float4 color4 = color3;
	if(round(uSelected)==1) {
		color4=lerp(color3,uSelectedColor,uSelectedColor.a);
		color4.a= color3.a;
	}

	return color4;
}