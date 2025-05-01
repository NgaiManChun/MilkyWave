

//*****************************************************************************
// �萔�o�b�t�@
//*****************************************************************************

// �}�g���N�X�o�b�t�@
cbuffer ConstantBuffer : register(b0)
{
    matrix worldViewProjection;
    float2 uvOffset;
    float2 uvRange;
}

// �}�e���A���o�b�t�@
cbuffer MaterialBuffer : register(b1)
{
    float4 color;
}

cbuffer LightBuffer : register(b2)
{
    float4 ambientLight;
    float4 driectLight;
    float4 driectLightLocal;
}

cbuffer BoneBuffer : register(b3)
{
    float4x4 boneOffsetMatrix[100];
    float4x4 boneWorldMatrix[100];
};

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

//=============================================================================
// ���_�V�F�[�_
//=============================================================================
void VertexShaderPolygon(in float4 inPosition : POSITION0,
						  in float4 inNormal : NORMAL0,
						  in float4 inDiffuse : COLOR0,
						  in float2 inTexCoord : TEXCOORD0,

						  out float4 outPosition : SV_POSITION,
						  out float4 outNormal : NORMAL0,
						  out float2 outTexCoord : TEXCOORD0,
						  out float4 outDiffuse : COLOR0)
{
    outPosition = mul(inPosition, worldViewProjection);
    outNormal = inNormal;
    outTexCoord = inTexCoord * uvRange + uvOffset;

    outDiffuse = inDiffuse * color;
    
    
}

void BoneVertexShaderPolygon(
    in float4 inPosition : POSITION0,
    in float4 inNormal : NORMAL0,
    in float4 inDiffuse : COLOR0,
    in float2 inTexCoord : TEXCOORD0,
    in uint4 inBoneIndexes : BLENDINDICES0,
    in float4 inBoneWeights : BLENDWEIGHT0,

    out float4 outPosition : SV_POSITION,
    out float4 outNormal : NORMAL0,
    out float2 outTexCoord : TEXCOORD0,
    out float4 outDiffuse : COLOR0)
{
    float4 skinnedPosition = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float3 skinnedNormal = float3(0, 0, 0);
    float3 normal = (float3) inNormal;
    //matrix boneTransform = boneMatrix[inBoneIndexes[0]] * inBoneWeights[0];
    
    
    for (int i = 0; i < 4; ++i)
    {
        skinnedPosition += inBoneWeights[i] * mul(boneWorldMatrix[inBoneIndexes[i]], mul(boneOffsetMatrix[inBoneIndexes[i]], inPosition));
        skinnedNormal += inBoneWeights[i] * mul((float3x3) boneWorldMatrix[inBoneIndexes[i]], mul((float3x3) boneOffsetMatrix[inBoneIndexes[i]], normal));
        //float3x3 rotationMatrix = (float3x3) boneMatrix[inBoneIndexes[i]];
        
        //skinnedNormal += inBoneWeights[i] * mul(rotationMatrix, normal);
    }
    
    //skinnedPosition += inBoneWeights[0] * mul(boneMatrix[inBoneIndexes[0]], inPosition);
    
    //skinnedPosition += inBoneWeights[0] * mul(boneWorldMatrix[inBoneIndexes[0]], mul(boneOffsetMatrix[inBoneIndexes[0]], inPosition));
    //skinnedPosition += inBoneWeights[1] * mul(boneWorldMatrix[inBoneIndexes[1]], mul(boneOffsetMatrix[inBoneIndexes[1]], inPosition));
    //skinnedPosition += inBoneWeights[2] * mul(boneWorldMatrix[inBoneIndexes[2]], mul(boneOffsetMatrix[inBoneIndexes[2]], inPosition));
    //skinnedPosition += inBoneWeights[3] * mul(boneWorldMatrix[inBoneIndexes[3]], mul(boneOffsetMatrix[inBoneIndexes[3]], inPosition));
    
    //boneTransform += boneMatrix[inBoneIndexes[0]] * inBoneWeights[0];
    //boneTransform += boneMatrix[inBoneIndexes[1]] * inBoneWeights[1];
    //boneTransform += boneMatrix[inBoneIndexes[2]] * inBoneWeights[2];
    //boneTransform += boneMatrix[inBoneIndexes[3]] * inBoneWeights[3];
    
    //outPosition = mul(skinnedPosition, worldViewProjection);
    outPosition = mul(skinnedPosition, worldViewProjection);
    //outPosition = mul(inPosition, worldViewProjection);
    //outPosition = mul(skinnedPosition, worldViewProjection);
    //outPosition = mul(
    //    mul(
    //        inPosition,
    //        matrix(
    //            1.0f, 0.0f, 0.0f, 0.0f,
    //            0.0f, 1.0f, 0.0f, 0.0f,
    //            0.0f, 0.0f, 1.0f, 0.0f,
    //            0.0f, 0.0f, 0.0f, 1.0f
    //        ) + boneMatrix[0]
    //    ),
    //worldViewProjection);
    
    outNormal = float4(normalize(skinnedNormal).xyz, 0);
    outTexCoord = inTexCoord * uvRange + uvOffset;

    outDiffuse = inDiffuse * color;
    
    
    //outPosition = mul(inPosition, worldViewProjection);
    
    //outNormal = inNormal;
    //outTexCoord = inTexCoord * uvRange + uvOffset;

    //outDiffuse = inDiffuse * color;
    
    
}

//=============================================================================
// �s�N�Z���V�F�[�_
//=============================================================================
void PixelShaderTexture(in float4 inPosition : POSITION0,
						 in float4 inNormal : NORMAL0,
						 in float2 inTexCoord : TEXCOORD0,
						 in float4 inDiffuse : COLOR0,

						 out float4 outDiffuse : SV_Target)
{
    outDiffuse = g_Texture.Sample(g_SamplerState, inTexCoord) * inDiffuse;
    
    float3 light = ambientLight.rgb * ambientLight.a; // float3(0.0f, 0.0f, 0.0f);
    light += driectLight.rgb * driectLight.a * saturate(dot(normalize(inNormal.xyz), normalize(driectLightLocal.xyz)));
    //outDiffuse.rgb *= ambientLight.rgb * ambientLight.a;
    outDiffuse.rgb *= light;
    
    
    // �����t�F�[�h�A�E�g
    //float far = 2000.0f; // ����
    //float border = 0.8f; // ���ځA�p�[�Z���g
    //float d = distance(inPosition.xyz, float3(0.0f, 0.0f, 0.0f));
    //outDiffuse.a *= 1.0f - saturate(d / far - border) / (1.0f - border);
}

//=============================================================================
// �s�N�Z���V�F�[�_�A�e�N�X�`���Ȃ�
//=============================================================================
void PixelShaderPolygon(in float4 inPosition : POSITION0,
						 in float4 inNormal : NORMAL0,
						 in float2 inTexCoord : TEXCOORD0,
						 in float4 inDiffuse : COLOR0,

						 out float4 outDiffuse : SV_Target)
{
    outDiffuse = inDiffuse;
    float3 light = ambientLight.rgb * ambientLight.a; // float3(0.0f, 0.0f, 0.0f);
    light += driectLight.rgb * driectLight.a * saturate(dot(normalize(inNormal.xyz), normalize(driectLightLocal.xyz)));
    //outDiffuse.rgb *= ambientLight.rgb * ambientLight.a;
    outDiffuse.rgb *= light;
}

//=============================================================================
// �s�N�Z���V�F�[�_�A���C�e�B���O�Ȃ�
//=============================================================================
void PixelShaderNoLighting(in float4 inPosition : POSITION0,
						 in float4 inNormal : NORMAL0,
						 in float2 inTexCoord : TEXCOORD0,
						 in float4 inDiffuse : COLOR0,

						 out float4 outDiffuse : SV_Target)
{
    outDiffuse = g_Texture.Sample(g_SamplerState, inTexCoord) * inDiffuse;
    
}


