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
	[](FRHICommandListImmediate& RHICmdList,                         \
	   const FKaleidoState&      KaleidoState,                       \
	   const FInfluencerState&   InfluencerState,                    \
	   const FKaleidoShaderDef&  ShaderDef)                          \
	{ ComputeTransforms_RenderThread<ShaderClassName>(RHICmdList, KaleidoState, InfluencerState, ShaderDef); } \
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
		const FKaleidoState&      KaleidoState,
		const FInfluencerState&   InfluencerState,
		const FKaleidoShaderDef&  ShaderDef)
	{
		if (!ShaderDef.ShaderName.IsNone())
		{
			if (const ComputeFunc* Func = KaleidoComputeShaderFuncMap.Find(ShaderDef.ShaderName))
			{
				(*Func)(RHICmdList, KaleidoState, InfluencerState, ShaderDef);
			}
			else
			{
				if (GEngine)
				{
					const TCHAR* ShaderNameStr = *ShaderDef.ShaderName.ToString();
					GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Cyan, FString::Printf(TEXT("shader name %s not found"), ShaderNameStr));
				}
			}
		}
	}

	bool IsValidShaderDef(const FKaleidoShaderDef& ShaderDef)
	{
		return KaleidoComputeShaderFuncMap.Find(ShaderDef.ShaderName) != nullptr;
	}
}