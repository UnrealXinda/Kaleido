// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shaders/KaleidoComputeShader.h"

BEGIN_KALEIDO_SHADER_PARAMETER_STRUCT(FInclusiveRotationShaderParameters, )
	SHADER_PARAMETER(float, InfluencerRadius)
END_KALEIDO_SHADER_PARAMETER_STRUCT()
IMPLEMENT_KALEIDO_SHADER_PARAMETER_STRUCT(FInclusiveRotationShaderParameters, "InclusiveRotationShaderUniform");

class FInclusiveRotationShader : public FKaleidoComputeShader
{
public:

	using FParameters = FInclusiveRotationShaderParameters;
	DECLARE_KALEIDO_COMPUTE_SHADER(FInclusiveRotationShader)

	FInclusiveRotationShader() {}
	FInclusiveRotationShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
		FKaleidoComputeShader(Initializer) {}
};

IMPLEMENT_SHADER_TYPE(, FInclusiveRotationShader, TEXT("/Plugin/Kaleido/Rotation/InclusiveRotationShader.usf"), TEXT("InclusiveRotationCS"), SF_Compute);
