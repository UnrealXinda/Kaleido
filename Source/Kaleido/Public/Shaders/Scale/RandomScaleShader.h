// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shaders/KaleidoShaderTemplates.h"
#include "Shaders/KaleidoComputeShader.h"

BEGIN_KALEIDO_SHADER_PARAMETER_STRUCT(FRandomScaleShaderParameters, )
	SHADER_PARAMETER(FVector, MinScale)
	SHADER_PARAMETER(FVector, MaxScale)
	SHADER_PARAMETER(float,   InfluencerRadius)
END_KALEIDO_SHADER_PARAMETER_STRUCT()
IMPLEMENT_KALEIDO_SHADER_PARAMETER_STRUCT(FRandomScaleShaderParameters, "RandomScaleShaderUniform");

class FRandomScaleShader : public FKaleidoComputeShader
{
public:

	using FParameters = FRandomScaleShaderParameters;
	DECLARE_KALEIDO_COMPUTE_SHADER(FRandomScaleShader)

	FRandomScaleShader() {}
	FRandomScaleShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
		FKaleidoComputeShader(Initializer) {}
};

IMPLEMENT_GLOBAL_SHADER(FRandomScaleShader, "/Plugin/Kaleido/Scale/RandomScaleShader.usf", "RandomScaleCS", SF_Compute);

template<>
FRandomScaleShader::FParameters CreateKaleidoShaderParameter<FRandomScaleShader::FParameters>(
	const FKaleidoState&     KaleidoState,
	const FInfluencerState&  InfluencerState,
	const FKaleidoShaderDef& ShaderDef)
{
	FRandomScaleShader::FParameters UniformParam;
	SetDefaultKaleidoShaderParameters(UniformParam, KaleidoState, InfluencerState);

	UniformParam.MinScale = ShaderDef.GetShaderParam<FVector>(FName("MinScale"));
	UniformParam.MaxScale = ShaderDef.GetShaderParam<FVector>(FName("MaxScale"));

	return UniformParam;
}