// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#define CollisionProfile_Kaleido      TEXT("Kaleido")
#define CollisionProfile_Influencer   TEXT("Influencer")


#define BEGIN_KALEIDO_SHADER_PARAMETER_STRUCT(StructTypeName, PrefixKeywords)  \
BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(StructTypeName, PrefixKeywords)           \
	SHADER_PARAMETER(FMatrix, InfluencerTransform)                             \
	SHADER_PARAMETER(FMatrix, ModelTransform)                                  \
	SHADER_PARAMETER(FVector, TranslationInertia)                              \
	SHADER_PARAMETER(FVector, RotationInertia)                                 \
	SHADER_PARAMETER(FVector, ScaleInertia)

#define END_KALEIDO_SHADER_PARAMETER_STRUCT() \
END_GLOBAL_SHADER_PARAMETER_STRUCT()

#define IMPLEMENT_KALEIDO_SHADER_PARAMETER_STRUCT(StructTypeName, ShaderVariableName) \
IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(StructTypeName, ShaderVariableName)

#define DECLARE_KALEIDO_COMPUTE_SHADER(ShaderTypeName)                                         \
DECLARE_SHADER_TYPE(ShaderTypeName, Global)                                                    \
static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)     \
{                                                                                              \
	return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);                \
}                                                                                              \
static bool ShouldCache(EShaderPlatform Platform)                                              \
{                                                                                              \
	return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM5);                           \
}                                                                                              \
static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, \
	FShaderCompilerEnvironment& OutEnvironment)                                                \
{                                                                                              \
	FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);                   \
}                                                                                              \
void SetShaderParameters(FRHICommandList& RHICmdList, const FParameters& Parameters)           \
{                                                                                              \
	auto Param = GetUniformBufferParameter<FParameters>();                                     \
	SetUniformBufferParameterImmediate(RHICmdList, GetComputeShader(), Param, Parameters);     \
}                                                                                              \
public:


