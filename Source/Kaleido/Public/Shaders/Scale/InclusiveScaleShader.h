// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shaders/KaleidoShaderTemplates.h"
#include "Shaders/KaleidoComputeShader.h"

BEGIN_KALEIDO_SHADER_PARAMETER_STRUCT(FInclusiveScaleShaderParameters, )
	SHADER_PARAMETER(FVector, MinScale)
	SHADER_PARAMETER(FVector, MaxScale)
	SHADER_PARAMETER(float,   InfluencerRadius)
	SHADER_PARAMETER(int,     Direction) // 0 for MinScale at influencer center, 1 for MaxScale at influencer center
END_KALEIDO_SHADER_PARAMETER_STRUCT()
IMPLEMENT_KALEIDO_SHADER_PARAMETER_STRUCT(FInclusiveScaleShaderParameters, "InclusiveScaleShaderUniform");

class FInclusiveScaleShader : public FKaleidoComputeShader
{
public:

	using FParameters = FInclusiveScaleShaderParameters;
	DECLARE_KALEIDO_COMPUTE_SHADER(FInclusiveScaleShader)

	FInclusiveScaleShader() {}
	FInclusiveScaleShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
		FKaleidoComputeShader(Initializer) {}
};

IMPLEMENT_SHADER_TYPE(, FInclusiveScaleShader, TEXT("/Plugin/Kaleido/Scale/InclusiveScaleShader.usf"), TEXT("InclusiveScaleCS"), SF_Compute);

template<>
FInclusiveScaleShader::FParameters CreateKaleidoShaderParameter<FInclusiveScaleShader::FParameters>(const UKaleidoInstancedMeshComponent& Kaleido, const AKaleidoInfluencer* Influencer)
{
	// TODO: These are thread unsafe	 
	FInclusiveScaleShader::FParameters UniformParam;
	UniformParam.ModelTransform      = Kaleido.GetComponentTransform().ToMatrixWithScale();
	UniformParam.InfluencerTransform = Influencer->GetActorTransform().ToMatrixWithScale();
	UniformParam.TranslationInertia  = Kaleido.TranslationInertia;
	UniformParam.RotationInertia     = Kaleido.RotationInertia;
	UniformParam.ScaleInertia        = Kaleido.ScaleInertia;

	UniformParam.InfluencerRadius    = Influencer->GetInfluencerRadius();
	UniformParam.MinScale            = FVector(0.3);       // TODO: these should come from influencer
	UniformParam.MaxScale            = FVector::OneVector; // TODO: these should come from influencer
	UniformParam.Direction           = 0;

	return UniformParam;
}