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

IMPLEMENT_SHADER_TYPE(, FRandomScaleShader, TEXT("/Plugin/Kaleido/Scale/RandomScaleShader.usf"), TEXT("RandomScaleCS"), SF_Compute);

template<>
FRandomScaleShader::FParameters CreateKaleidoShaderParameter<FRandomScaleShader::FParameters>(const UKaleidoInstancedMeshComponent& Kaleido, const AKaleidoInfluencer* Influencer)
{
	// TODO: These are thread unsafe	 
	FRandomScaleShader::FParameters UniformParam;
	UniformParam.ModelTransform      = Kaleido.GetComponentTransform().ToMatrixWithScale();
	UniformParam.InfluencerTransform = Influencer->GetActorTransform().ToMatrixWithScale();
	UniformParam.TranslationInertia  = Kaleido.TranslationInertia;
	UniformParam.RotationInertia     = Kaleido.RotationInertia;
	UniformParam.ScaleInertia        = Kaleido.ScaleInertia;

	UniformParam.InfluencerRadius    = Influencer->GetInfluencerRadius();
	UniformParam.MinScale            = FVector(0.3);       // TODO: these should come from influencer
	UniformParam.MaxScale            = FVector::OneVector; // TODO: these should come from influencer

	return UniformParam;
}