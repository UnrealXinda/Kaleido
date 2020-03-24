// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shaders/KaleidoShaderTemplates.h"
#include "Shaders/KaleidoComputeShader.h"

BEGIN_KALEIDO_SHADER_PARAMETER_STRUCT(FSimpleTranslationShaderParameters, )
	SHADER_PARAMETER(FVector, MinTranslation)
	SHADER_PARAMETER(FVector, MaxTranslation)
	SHADER_PARAMETER(float,   InfluencerRadius)
END_KALEIDO_SHADER_PARAMETER_STRUCT()
IMPLEMENT_KALEIDO_SHADER_PARAMETER_STRUCT(FSimpleTranslationShaderParameters, "SimpleTranslationShaderUniform");

class FSimpleTranslationShader : public FKaleidoComputeShader
{
public:

	using FParameters = FSimpleTranslationShaderParameters;
	DECLARE_KALEIDO_COMPUTE_SHADER(FSimpleTranslationShader)

	FSimpleTranslationShader() {}
	FSimpleTranslationShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
		FKaleidoComputeShader(Initializer) {}
};

IMPLEMENT_SHADER_TYPE(, FSimpleTranslationShader, TEXT("/Plugin/Kaleido/Translation/SimpleTranslationShader.usf"), TEXT("SimpleTranslationCS"), SF_Compute);

template<>
FSimpleTranslationShader::FParameters CreateKaleidoShaderParameter<FSimpleTranslationShader::FParameters>(
	const FKaleidoState&     KaleidoState,
	const FInfluencerState&  InfluencerState,
	const FKaleidoShaderDef& ShaderDef)
{
	FSimpleTranslationShader::FParameters UniformParam;
	SetDefaultKaleidoShaderParameters(UniformParam, KaleidoState, InfluencerState);

	UniformParam.MinTranslation = ShaderDef.GetShaderParam<FVector>(FName("MinTranslation"));
	UniformParam.MaxTranslation = ShaderDef.GetShaderParam<FVector>(FName("MaxTranslation"));

	return UniformParam;
}