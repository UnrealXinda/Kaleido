// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shaders/KaleidoShaderTemplates.h"
#include "Shaders/KaleidoComputeShader.h"

BEGIN_KALEIDO_SHADER_PARAMETER_STRUCT(FDefaultRotationShaderParameters, )
END_KALEIDO_SHADER_PARAMETER_STRUCT()
IMPLEMENT_KALEIDO_SHADER_PARAMETER_STRUCT(FDefaultRotationShaderParameters, "DefaultRotationShaderUniform");

class FDefaultRotationShader : public FKaleidoComputeShader
{
public:

	using FParameters = FDefaultRotationShaderParameters;
	DECLARE_KALEIDO_COMPUTE_SHADER(FDefaultRotationShader)

	FDefaultRotationShader() {}
	FDefaultRotationShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
		FKaleidoComputeShader(Initializer) {}
};

IMPLEMENT_SHADER_TYPE(, FDefaultRotationShader, TEXT("/Plugin/Kaleido/Rotation/DefaultRotationShader.usf"), TEXT("DefaultRotationCS"), SF_Compute);

template<>
FDefaultRotationShader::FParameters CreateKaleidoShaderParameter<FDefaultRotationShader::FParameters>(const UKaleidoInstancedMeshComponent& Kaleido, const AKaleidoInfluencer* Influencer)
{
	// TODO: These are thread unsafe	 
	FDefaultRotationShader::FParameters UniformParam;
	UniformParam.ModelTransform      = Kaleido.GetComponentTransform().ToMatrixWithScale();
	UniformParam.TranslationInertia  = Kaleido.TranslationInertia;
	UniformParam.RotationInertia     = Kaleido.RotationInertia;
	UniformParam.ScaleInertia        = Kaleido.ScaleInertia;

	return UniformParam;
}