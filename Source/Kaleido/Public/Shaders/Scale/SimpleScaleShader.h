// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shaders/KaleidoShaderTemplates.h"
#include "Shaders/KaleidoComputeShader.h"

BEGIN_KALEIDO_SHADER_PARAMETER_STRUCT(FSimpleScaleShaderParameters, )
	SHADER_PARAMETER(FVector, MinScale)
	SHADER_PARAMETER(FVector, MaxScale)
	SHADER_PARAMETER(float,   InfluencerRadius)
	SHADER_PARAMETER(int,     Direction) // 0 for MinScale at influencer center, 1 for MaxScale at influencer center
END_KALEIDO_SHADER_PARAMETER_STRUCT()
IMPLEMENT_KALEIDO_SHADER_PARAMETER_STRUCT(FSimpleScaleShaderParameters, "SimpleScaleShaderUniform");

class FSimpleScaleShader : public FKaleidoComputeShader
{
public:

	using FParameters = FSimpleScaleShaderParameters;
	DECLARE_KALEIDO_COMPUTE_SHADER(FSimpleScaleShader)

	FSimpleScaleShader() {}
	FSimpleScaleShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
		FKaleidoComputeShader(Initializer) {}
};

IMPLEMENT_SHADER_TYPE(, FSimpleScaleShader, TEXT("/Plugin/Kaleido/Scale/SimpleScaleShader.usf"), TEXT("SimpleScaleCS"), SF_Compute);

template<>
FSimpleScaleShader::FParameters CreateKaleidoShaderParameter<FSimpleScaleShader::FParameters>(
	const FKaleidoState&     KaleidoState,
	const FInfluencerState&  InfluencerState,
	const FKaleidoShaderDef& ShaderDef)
{
	// TODO: These are thread unsafe	 
	FSimpleScaleShader::FParameters UniformParam;
	SetDefaultKaleidoShaderParameters(UniformParam, KaleidoState, InfluencerState);

	UniformParam.MinScale  = ShaderDef.GetShaderParam<FVector>(FName("MinScale"));
	UniformParam.MaxScale  = ShaderDef.GetShaderParam<FVector>(FName("MaxScale"));
	UniformParam.Direction = ShaderDef.GetShaderParam<int32>(FName("Direction"));

	return UniformParam;
}