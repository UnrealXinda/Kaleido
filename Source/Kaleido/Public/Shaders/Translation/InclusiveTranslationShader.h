// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shaders/KaleidoShaderTemplates.h"
#include "Shaders/KaleidoComputeShader.h"

BEGIN_KALEIDO_SHADER_PARAMETER_STRUCT(FInclusiveTranslationShaderParameters, )
	SHADER_PARAMETER(float, MinTranslation)
	SHADER_PARAMETER(float, MaxTranslation)
	SHADER_PARAMETER(float, InfluencerRadius)
END_KALEIDO_SHADER_PARAMETER_STRUCT()
IMPLEMENT_KALEIDO_SHADER_PARAMETER_STRUCT(FInclusiveTranslationShaderParameters, "InclusiveTranslationShaderUniform");

class FInclusiveTranslationShader : public FKaleidoComputeShader
{
public:

	using FParameters = FInclusiveTranslationShaderParameters;
	DECLARE_KALEIDO_COMPUTE_SHADER(FInclusiveTranslationShader)

	FInclusiveTranslationShader() {}
	FInclusiveTranslationShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
		FKaleidoComputeShader(Initializer) {}
};

IMPLEMENT_SHADER_TYPE(, FInclusiveTranslationShader, TEXT("/Plugin/Kaleido/Translation/InclusiveTranslationShader.usf"), TEXT("InclusiveTranslationCS"), SF_Compute);

template<>
FInclusiveTranslationShader::FParameters CreateKaleidoShaderParameter<FInclusiveTranslationShader::FParameters>(const UKaleidoInstancedMeshComponent& Kaleido, const AKaleidoInfluencer* Influencer)
{
	// TODO: These are thread unsafe	 
	FInclusiveTranslationShader::FParameters UniformParam;
	UniformParam.ModelTransform      = Kaleido.GetComponentTransform().ToMatrixWithScale();
	UniformParam.InfluencerTransform = Influencer->GetActorTransform().ToMatrixWithScale();
	UniformParam.TranslationInertia  = Kaleido.TranslationInertia;
	UniformParam.RotationInertia     = Kaleido.RotationInertia;
	UniformParam.ScaleInertia        = Kaleido.ScaleInertia;

	UniformParam.InfluencerRadius    = Influencer->GetInfluencerRadius();
	UniformParam.MinTranslation      = 0.0f;    // TODO: these should come from influencer
	UniformParam.MaxTranslation      = 100.0f;  // TODO: these should come from influencer

	return UniformParam;
}