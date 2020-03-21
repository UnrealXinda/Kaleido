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

using ComputeFunc = auto (*)(FRHICommandListImmediate&, const UKaleidoInstancedMeshComponent&, const AKaleidoInfluencer*) -> void;
using KaleidoComputeFuncMap = TMap<FName, ComputeFunc>;

template<class ShaderParamType>
ShaderParamType CreateKaleidoShaderParameter(
	const UKaleidoInstancedMeshComponent& Kaleido,
	const AKaleidoInfluencer* Influencer)
{
	ShaderParamType Param;
	return Param;
}

template<class ShaderType, typename ShaderParamType = ShaderType::FParameters>
void ComputeTransforms_RenderThread(
	FRHICommandListImmediate& RHICmdList,
	const UKaleidoInstancedMeshComponent& Kaleido,
	const AKaleidoInfluencer* Influencer)
{
	check(IsInRenderingThread());

	TShaderMapRef<ShaderType> KaleidoShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
	RHICmdList.SetComputeShader(KaleidoShader->GetComputeShader());

	// Bind shader buffers
	FUnorderedAccessViewRHIRef DirtyFlagBufferUAV = Kaleido.GetDirtyFlagBufferUAV();
	FUnorderedAccessViewRHIRef InstanceTransformBufferUAV = Kaleido.GetInstanceTransformBufferUAV();
	FShaderResourceViewRHIRef InitialTransformBufferSRV = Kaleido.GetInitialTransformBufferSRV();
	KaleidoShader->BindTransformBuffers(RHICmdList, DirtyFlagBufferUAV, InstanceTransformBufferUAV, InitialTransformBufferSRV);

	// Create shader uniform
	ShaderParamType Param = CreateKaleidoShaderParameter<ShaderParamType>(Kaleido, Influencer);

	// Bind shader uniform
	KaleidoShader->SetShaderParameters(RHICmdList, Param);

	// Dispatch shader
	const int32 InstanceCount = Kaleido.GetInstanceCount();
	const int32 ThreadGroupCountX = FMath::CeilToInt(InstanceCount / 128.f);
	const int32 ThreadGroupCountY = 1;
	const int32 ThreadGroupCountZ = 1;
	DispatchComputeShader(RHICmdList, *KaleidoShader, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);

	// Unbind shader buffers
	KaleidoShader->UnbindTransformBuffers(RHICmdList);

	// Wait for compute shader to finish to avoid race condition
	const int32 BufferSize = InstanceCount * sizeof(FMatrix);
	FStructuredBufferRHIRef TransformBufferRHIRef = Kaleido.GetInstanceTransformBufferRHIRef();
	RHILockStructuredBuffer(TransformBufferRHIRef, 0, BufferSize, EResourceLockMode::RLM_ReadOnly);
	RHIUnlockStructuredBuffer(TransformBufferRHIRef);
}