// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Shaders/KaleidoShaderTemplates.h"

// Translation shaders
#include "Shaders/Translation/SimpleTranslationShader.h"
#include "Shaders/Translation/RandomTranslationShader.h"

// Rotation shaders
#include "Shaders/Rotation/RandomRotationShader.h"
#include "Shaders/Rotation/SimpleRotationShader.h"

// Scale shaders
#include "Shaders/Scale/SimpleScaleShader.h"
#include "Shaders/Scale/RandomScaleShader.h"

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
		REGISTER_KALEIDO_COMPUTE_SHADER("SimpleTranslation", FSimpleTranslationShader),
		REGISTER_KALEIDO_COMPUTE_SHADER("RandomTranslation", FRandomTranslationShader),

		// Custom rotation shaders
		REGISTER_KALEIDO_COMPUTE_SHADER("RandomRotation", FRandomRotationShader),
		REGISTER_KALEIDO_COMPUTE_SHADER("SimpleRotation", FSimpleRotationShader),

		// Custom scale shaders
		REGISTER_KALEIDO_COMPUTE_SHADER("SimpleScale", FSimpleScaleShader),
		REGISTER_KALEIDO_COMPUTE_SHADER("RandomScale", FRandomScaleShader)
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