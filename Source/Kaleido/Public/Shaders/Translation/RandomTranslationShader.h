// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shaders/KaleidoShaderTemplates.h"
#include "Shaders/KaleidoComputeShader.h"

BEGIN_KALEIDO_SHADER_PARAMETER_STRUCT(FRandomTranslationShaderParameters, )
	SHADER_PARAMETER(float, MinTranslation)
	SHADER_PARAMETER(float, MaxTranslation)
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
FRandomTranslationShader::FParameters CreateKaleidoShaderParameter<FRandomTranslationShader::FParameters>(const UKaleidoInstancedMeshComponent& Kaleido, const AKaleidoInfluencer* Influencer)
{
	// TODO: These are thread unsafe	 
	FRandomTranslationShader::FParameters UniformParam;
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