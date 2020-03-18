// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shaders/KaleidoShaderTemplates.h"
#include "Shaders/KaleidoComputeShader.h"

BEGIN_KALEIDO_SHADER_PARAMETER_STRUCT(FDefaultTranslationShaderParameters, )
END_KALEIDO_SHADER_PARAMETER_STRUCT()
IMPLEMENT_KALEIDO_SHADER_PARAMETER_STRUCT(FDefaultTranslationShaderParameters, "DefaultTranslationShaderUniform");

class FDefaultTranslationShader : public FKaleidoComputeShader
{
public:

	using FParameters = FDefaultTranslationShaderParameters;
	DECLARE_KALEIDO_COMPUTE_SHADER(FDefaultTranslationShader)

	FDefaultTranslationShader() {}
	FDefaultTranslationShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
		FKaleidoComputeShader(Initializer) {}
};

IMPLEMENT_SHADER_TYPE(, FDefaultTranslationShader, TEXT("/Plugin/Kaleido/Translation/DefaultTranslationShader.usf"), TEXT("DefaultTranslationCS"), SF_Compute);

template<>
FDefaultTranslationShader::FParameters CreateKaleidoShaderParameter<FDefaultTranslationShader::FParameters>(const UKaleidoInstancedMeshComponent& Kaleido, const AKaleidoInfluencer* Influencer)
{
	// TODO: These are thread unsafe	 
	FDefaultTranslationShader::FParameters UniformParam;
	UniformParam.ModelTransform      = Kaleido.GetComponentTransform().ToMatrixWithScale();
	UniformParam.TranslationInertia  = Kaleido.TranslationInertia;
	UniformParam.RotationInertia     = Kaleido.RotationInertia;
	UniformParam.ScaleInertia        = Kaleido.ScaleInertia;

	return UniformParam;
}