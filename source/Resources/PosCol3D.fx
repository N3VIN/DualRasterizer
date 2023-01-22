//----------
// Global Variables.
//----------

// float4x4 matrix global variables.
float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorldMatrix : WORLD;
float4x4 gViewInvMatrix : VIEWINVERSE;

// Texture2D global variables.
Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossMap : GlossMap;

// float global variables.
float gPI = float(3.1415926f);
float gLightIntensity;
float gSpecularShininess = float(25.0f);

// float3 global variables.
float3 gLightDirection;
float3 gAmbient = float3(0.025f, 0.025f, 0.025f);

// SamplerState global variable.
SamplerState gSampleState
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap; // or Mirror, Clamp, Border
    AddressV = Wrap; // or Mirror, Clamp, Border
};

// BlendState global variable.
BlendState gBlendState
{
	BlendEnable[0] = false;
	SrcBlend = src_alpha;
	DestBlend = inv_src_alpha;
	BlendOp = add;
	SrcBlendAlpha = zero;
	DestBlendAlpha = zero;
	BlendOpAlpha = add;
	RenderTargetWriteMask[0] = 0x0F;
};

// DepthStencilState global variable.
DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = true;
	DepthFunc = less;
	StencilEnable = false;

    // when stencilEnable is false the following doesnt matter.
	StencilReadMask = 0x0F;
	StencilWriteMask = 0x0F;

	FrontFaceStencilFunc = always;
	BackFaceStencilFunc = always;

	FrontFaceStencilDepthFail = keep;
	BackFaceStencilDepthFail = keep;

	FrontFaceStencilPass = keep;
	BackFaceStencilPass = keep;

	FrontFaceStencilFail = keep;
	BackFaceStencilFail = keep;
};

//----------
// Input/Output Structs.
//----------

struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Color : COLOR;
    float2 Uv : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float3 Color : COLOR;
    float2 Uv : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float4 WorldPosition : WPOSITION;
};

//----------
// Functions.
//----------

float3 Lambert(float kd, float3 cd)
{
    float3 lambert = float3(cd * kd) / gPI;
    return lambert;
}

float3 Phong(float3 specularColor, float ks, float exp, float3 l, float3 v, float3 n)
{
    float3 reflect = l - (2 * (dot(n, l)) * n);
    float cosAlpha = dot(reflect, v);
    float phong = ks * pow(saturate(cosAlpha), exp);
    return specularColor * phong;
}

float LambertCosineLaw(float3 n, float3 l)
{
    return max(0.0f, dot(n, -l));
}

//----------
// Vertex Shader.
//----------

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    float4 projectedVertex = mul(float4(input.Position, 1.0f), gWorldViewProj);
    output.Position = projectedVertex;
    output.Color = input.Color;
    output.Uv = input.Uv;
    output.Normal = mul(normalize(input.Normal), (float3x3)gWorldMatrix);
    //output.Tangent = -mul(normalize(input.Tangent), (float3x3)gWorldMatrix);
    output.Tangent = mul(normalize(input.Tangent), (float3x3)gWorldMatrix);
    output.WorldPosition = mul(input.Position, gWorldMatrix);
    return output;
}

//----------
// Pixel Shader.
//----------

float4 PS(VS_OUTPUT input) : SV_TARGET
{
    float3 binormal = normalize(cross(input.Normal, input.Tangent));
    float4x3 tangentSpaceMatrix = float4x3(input.Tangent, binormal, input.Normal, float3(0.f, 0.f, 0.f));

    float3 normalMapCol = (2.0f * gNormalMap.Sample(gSampleState, input.Uv)) - float3(1.0f, 1.0f, 1.0f);
    normalMapCol /= 255.f;
    float3 tangentSpaceVector = input.Normal;
    tangentSpaceVector = normalize(mul(normalMapCol, tangentSpaceMatrix)); // just comment this part out if you dont want normal map.
    
    float lambertCosineLaw = LambertCosineLaw(tangentSpaceVector, gLightDirection);
    if(lambertCosineLaw < 0.0f)
    {
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
    }
    float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInvMatrix[3].xyz);

    float specularExp = gSpecularShininess * gGlossMap.Sample(gSampleState, input.Uv).x;
    float3 specular = Phong(gSpecularMap.Sample(gSampleState, input.Uv), 1.0f, specularExp, gLightDirection, viewDirection, tangentSpaceVector);

    float3 lambert = Lambert(1.0f, gDiffuseMap.Sample(gSampleState, input.Uv));

    //return float4(lambert * lambertCosineLaw * gLightIntensity, 1.0f); // lambert final.
    //return float4(specular, 1.0f); // specular final.
    //return float4(lambertCosineLaw, lambertCosineLaw, lambertCosineLaw, 1.0f); // observed area final.
    return float4(((lambert * gLightIntensity) + specular) * lambertCosineLaw, 1.0f) + float4(gAmbient, 0.0f);
}

//----------
// Techinique.
//----------

technique11 DefaultTechnique
{
    pass P0
    {
        //SetRasterizerState(gRasterizerState);
        SetDepthStencilState(gDepthStencilState, 0);
        SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS() ) );
    }
}
