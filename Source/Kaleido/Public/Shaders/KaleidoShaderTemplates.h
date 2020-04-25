// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RenderCore/Public/GlobalShader.h"
#include "RenderCore/Public/ShaderParameterUtils.h"
#include "RenderCore/Public/ShaderParameterMacros.h"

#include "Public/GlobalShader.h"
#include "Public/ShaderParameterUtils.h"
#include "RHI/Public/RHICommandList.h"

#include "Components/KaleidoInstancedMeshComponent.h"
#include "Actors/KaleidoInfluencer.h"

using ComputeFunc = auto (*)(
	FRHICommandListImmediate&,
	const FKaleidoState&,
	const FInfluencerState&,
	const FKaleidoShaderDef&) -> void;

using KaleidoComputeFuncMap = TMap<FName, ComputeFunc>;

template<class ShaderParamType>
ShaderParamType CreateKaleidoShaderParameter(
	const FKaleidoState&     KaleidoState,
	const FInfluencerState&  InfluencerState,
	const FKaleidoShaderDef& ShaderDef)
{
	ShaderParamType Param;
	return Param;
}

template<class ShaderParamType>
void SetDefaultKaleidoShaderParameters(
	ShaderParamType&        Uniform,
	const FKaleidoState&    KaleidoState,
	const FInfluencerState& InfluencerState)
{
	Uniform.ModelTransform      = KaleidoState.KaleidoTransform;
	Uniform.InfluencerTransform = InfluencerState.InfluencerTransform;
	Uniform.TranslationInertia  = KaleidoState.TranslationInertia;
	Uniform.RotationInertia     = KaleidoState.RotationInertia;
	Uniform.ScaleInertia        = KaleidoState.ScaleInertia;
	Uniform.InfluencerRadius    = InfluencerState.InfluencerRadius;
}

template<class ShaderType, typename ShaderParamType = ShaderType::FParameters>
void ComputeTransforms_RenderThread(
	FRHICommandListImmediate& RHICmdList,
	const FKaleidoState&      KaleidoState,
	const FInfluencerState&   InfluencerState,
	const FKaleidoShaderDef&  ShaderDef)
{
	check(IsInRenderingThread());
	const int32 InstanceCount = KaleidoState.InstanceCount;
	const int32 BufferSize = InstanceCount * sizeof(FMatrix);
	FStructuredBufferRHIRef InstanceTransformBuffer = KaleidoState.InstanceTransformBuffer;
	RHILockStructuredBuffer(InstanceTransformBuffer, 0, BufferSize, EResourceLockMode::RLM_ReadOnly);

	TShaderMapRef<ShaderType> KaleidoShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
	RHICmdList.SetComputeShader(KaleidoShader->GetComputeShader());

	// Bind shader buffers
	FUnorderedAccessViewRHIRef DirtyFlagBufferUAV         = KaleidoState.DirtyFlagBufferUAV;
	FUnorderedAccessViewRHIRef InstanceTransformBufferUAV = KaleidoState.InstanceTransformBufferUAV;
	FShaderResourceViewRHIRef  InitialTransformBufferSRV  = KaleidoState.InitialTransformBufferSRV;
	KaleidoShader->BindTransformBuffers(RHICmdList, DirtyFlagBufferUAV, InstanceTransformBufferUAV, InitialTransformBufferSRV);

	// Create shader uniform
	ShaderParamType Param = CreateKaleidoShaderParameter<ShaderParamType>(KaleidoState, InfluencerState, ShaderDef);

	// Bind shader uniform
	KaleidoShader->SetShaderParameters(RHICmdList, Param);

	// Dispatch shader
	const int32 ThreadGroupCountX = FMath::CeilToInt(InstanceCount / 128.f);
	const int32 ThreadGroupCountY = 1;
	const int32 ThreadGroupCountZ = 1;
	DispatchComputeShader(RHICmdList, *KaleidoShader, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);

	// Unbind shader buffers
	KaleidoShader->UnbindTransformBuffers(RHICmdList);

	// Wait for compute shader to finish to avoid race condition
	RHIUnlockStructuredBuffer(InstanceTransformBuffer);
}