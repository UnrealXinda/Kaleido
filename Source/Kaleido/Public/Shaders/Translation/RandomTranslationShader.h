// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shaders/KaleidoShaderTemplates.h"
#include "Shaders/KaleidoComputeShader.h"

BEGIN_KALEIDO_SHADER_PARAMETER_STRUCT(FRandomTranslationShaderParameters, )
	SHADER_PARAMETER(FVector, MinTranslation)
	SHADER_PARAMETER(FVector, MaxTranslation)
	SHADER_PARAMETER(float, InfluencerRadius)
END_KALEIDO_SHADER_PARAMETER_STRUCT()
IMPLEMENT_KALEIDO_SHADER_PARAMETER_STRUCT(FRandomTranslationShaderParameters, "RandomTranslationShaderUniform");

class FRandomTranslationShader : public FKaleidoComputeShader
{
public:

	using FParameters = FRandomTranslationShaderParameters;
	DECLARE_KALEIDO_COMPUTE_SHADER(FRandomTranslationShader)

	FRandomTranslationShader() {}
	FRandomTranslationShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
		FKaleidoComputeShader(Initializer) {}
};

IMPLEMENT_SHADER_TYPE(, FRandomTranslationShader, TEXT("/Plugin/Kaleido/Translation/RandomTranslationShader.usf"), TEXT("RandomTranslationCS"), SF_Compute);

template<>
FRandomTranslationShader::FParameters CreateKaleidoShaderParameter<FRandomTranslationShader::FParameters>(
	const FKaleidoState&     KaleidoState,
	const FInfluencerState&  InfluencerState,
	const FKaleidoShaderDef& ShaderDef)
{ 
	FRandomTranslationShader::FParameters UniformParam;
	SetDefaultKaleidoShaderParameters(UniformParam, KaleidoState, InfluencerState);

	UniformParam.MinTranslation = ShaderDef.GetShaderParam<FVector>(FName("MinTranslation"));
	UniformParam.MaxTranslation = ShaderDef.GetShaderParam<FVector>(FName("MaxTranslation"));

	return UniformParam;
}