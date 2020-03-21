// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Shaders/KaleidoShaderTemplates.h"

// Translation shaders
#include "Shaders/Translation/InclusiveTranslationShader.h"

// Rotation shaders
#include "Shaders/Scale/InclusiveScaleShader.h"

// Scale shaders
#include "Shaders/Rotation/InclusiveRotationShader.h"

#define REGISTER_KALEIDO_COMPUTE_SHADER(ShaderName, ShaderClassName) \
{                                                                    \
	FName(TEXT(ShaderName)),                                         \
	[](FRHICommandListImmediate& RHICmdList, const UKaleidoInstancedMeshComponent& Kaleido, const AKaleidoInfluencer* Influencer) \
	{ ComputeTransforms_RenderThread<ShaderClassName>(RHICmdList, Kaleido, Influencer);	} \
}

namespace Kaleido
{
	KaleidoComputeFuncMap KaleidoComputeShaderFuncMap =
	{
		// Default shaders
		REGISTER_KALEIDO_COMPUTE_SHADER("Default", FKaleidoDefaultShader),

		// Custom translation shaders
		REGISTER_KALEIDO_COMPUTE_SHADER("InclusiveTranslation", FInclusiveTranslationShader),

		// Custom rotation shaders
		REGISTER_KALEIDO_COMPUTE_SHADER("InclusiveRotation", FInclusiveRotationShader),

		// Custom scale shaders
		REGISTER_KALEIDO_COMPUTE_SHADER("InclusiveScale", FInclusiveScaleShader)
	};

	void ComputeTransforms(
		FRHICommandListImmediate& RHICmdList,
		FName ShaderName,
		const UKaleidoInstancedMeshComponent& KaleidoComp,
		const AKaleidoInfluencer* Influencer)
	{
		if (!ShaderName.IsNone())
		{
			if (const ComputeFunc* Func = KaleidoComputeShaderFuncMap.Find(ShaderName))
			{
				(*Func)(RHICmdList, KaleidoComp, Influencer);
			}
			else
			{
				if (GEngine)
				{
					const TCHAR* OwnerNameStr = *(KaleidoComp.GetOwner()->GetFName().ToString());
					const TCHAR* ShaderNameStr = *ShaderName.ToString();
					GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Cyan, FString::Printf(TEXT("%s with shader name %s not found"), OwnerNameStr, ShaderNameStr));
				}
			}
		}
	}
}