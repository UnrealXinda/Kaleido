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

IMPLEMENT_SHADER_TYPE(, FSimpleRotationShader, TEXT("/Plugin/Kaleido/Rotation/SimpleRotationShader.usf"), TEXT("SimpleRotationCS"), SF_Compute);

template<>
FSimpleRotationShader::FParameters CreateKaleidoShaderParameter<FSimpleRotationShader::FParameters>(const UKaleidoInstancedMeshComponent& Kaleido, const AKaleidoInfluencer* Influencer)
{
	// TODO: These are thread unsafe	 
	FSimpleRotationShader::FParameters UniformParam;
	UniformParam.ModelTransform      = Kaleido.GetComponentTransform().ToMatrixWithScale();
	UniformParam.InfluencerTransform = Influencer->GetActorTransform().ToMatrixWithScale();
	UniformParam.TranslationInertia  = Kaleido.TranslationInertia;
	UniformParam.RotationInertia     = Kaleido.RotationInertia;
	UniformParam.ScaleInertia        = Kaleido.ScaleInertia;

	UniformParam.InfluencerRadius    = Influencer->GetInfluencerRadius();

	return UniformParam;
}