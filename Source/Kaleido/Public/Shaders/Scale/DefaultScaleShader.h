// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shaders/KaleidoShaderTemplates.h"
#include "Shaders/KaleidoComputeShader.h"

BEGIN_KALEIDO_SHADER_PARAMETER_STRUCT(FDefaultScaleShaderParameters, )
END_KALEIDO_SHADER_PARAMETER_STRUCT()
IMPLEMENT_KALEIDO_SHADER_PARAMETER_STRUCT(FDefaultScaleShaderParameters, "DefaultScaleShaderUniform");

class FDefaultScaleShader : public FKaleidoComputeShader
{
public:

	using FParameters = FDefaultScaleShaderParameters;
	DECLARE_KALEIDO_COMPUTE_SHADER(FDefaultScaleShader)

	FDefaultScaleShader() {}
	FDefaultScaleShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
		FKaleidoComputeShader(Initializer) {}
};

IMPLEMENT_SHADER_TYPE(, FDefaultScaleShader, TEXT("/Plugin/Kaleido/Scale/DefaultScaleShader.usf"), TEXT("DefaultScaleCS"), SF_Compute);

template<>
FDefaultScaleShader::FParameters CreateKaleidoShaderParameter<FDefaultScaleShader::FParameters>(const UKaleidoInstancedMeshComponent& Kaleido, const AKaleidoInfluencer* Influencer)
{
	// TODO: These are thread unsafe	 
	FDefaultScaleShader::FParameters UniformParam;
	UniformParam.ModelTransform      = Kaleido.GetComponentTransform().ToMatrixWithScale();
	UniformParam.TranslationInertia  = Kaleido.TranslationInertia;
	UniformParam.RotationInertia     = Kaleido.RotationInertia;
	UniformParam.ScaleInertia        = Kaleido.ScaleInertia;

	return UniformParam;
}