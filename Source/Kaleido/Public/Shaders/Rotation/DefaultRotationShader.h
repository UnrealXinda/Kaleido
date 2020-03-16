// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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
