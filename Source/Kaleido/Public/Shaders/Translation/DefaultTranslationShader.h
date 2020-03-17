// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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
