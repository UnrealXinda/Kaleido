// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shaders/KaleidoShaderTemplates.h"
#include "Shaders/KaleidoComputeShader.h"

BEGIN_KALEIDO_SHADER_PARAMETER_STRUCT(FRandomRotationShaderParameters, )
	SHADER_PARAMETER(float, InfluencerRadius)
END_KALEIDO_SHADER_PARAMETER_STRUCT()
IMPLEMENT_KALEIDO_SHADER_PARAMETER_STRUCT(FRandomRotationShaderParameters, "RandomRotationShaderUniform");

class FRandomRotationShader : public FKaleidoComputeShader
{
public:

	using FParameters = FRandomRotationShaderParameters;
	DECLARE_KALEIDO_COMPUTE_SHADER(FRandomRotationShader)

	FRandomRotationShader() {}
	FRandomRotationShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
		FKaleidoComputeShader(Initializer) {}
};

IMPLEMENT_GLOBAL_SHADER(FRandomRotationShader, "/Plugin/Kaleido/Rotation/RandomRotationShader.usf", "RandomRotationCS", SF_Compute);

template<>
FRandomRotationShader::FParameters CreateKaleidoShaderParameter<FRandomRotationShader::FParameters>(
	const FKaleidoState&     KaleidoState,
	const FInfluencerState&  InfluencerState,
	const FKaleidoShaderDef& ShaderDef)
{
	FRandomRotationShader::FParameters UniformParam;
	SetDefaultKaleidoShaderParameters(UniformParam, KaleidoState, InfluencerState);

	return UniformParam;
}