// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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
