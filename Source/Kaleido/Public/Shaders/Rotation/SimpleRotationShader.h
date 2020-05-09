// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shaders/KaleidoShaderTemplates.h"
#include "Shaders/KaleidoComputeShader.h"

BEGIN_KALEIDO_SHADER_PARAMETER_STRUCT(FSimpleRotationShaderParameters, )
	SHADER_PARAMETER(float, InfluencerRadius)
END_KALEIDO_SHADER_PARAMETER_STRUCT()
IMPLEMENT_KALEIDO_SHADER_PARAMETER_STRUCT(FSimpleRotationShaderParameters, "SimpleRotationShaderUniform");

class FSimpleRotationShader : public FKaleidoComputeShader
{
public:

	using FParameters = FSimpleRotationShaderParameters;
	DECLARE_KALEIDO_COMPUTE_SHADER(FSimpleRotationShader)

	FSimpleRotationShader() {}
	FSimpleRotationShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
		FKaleidoComputeShader(Initializer) {}
};

IMPLEMENT_GLOBAL_SHADER(FSimpleRotationShader, "/Plugin/Kaleido/Rotation/SimpleRotationShader.usf", "SimpleRotationCS", SF_Compute);

template<>
FSimpleRotationShader::FParameters CreateKaleidoShaderParameter<FSimpleRotationShader::FParameters>(
	const FKaleidoState&     KaleidoState,
	const FInfluencerState&  InfluencerState,
	const FKaleidoShaderDef& ShaderDef)
{ 
	FSimpleRotationShader::FParameters UniformParam;
	SetDefaultKaleidoShaderParameters(UniformParam, KaleidoState, InfluencerState);

	return UniformParam;
}