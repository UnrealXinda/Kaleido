// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RenderCore/Public/GlobalShader.h"
#include "RenderCore/Public/ShaderParameterUtils.h"
#include "RenderCore/Public/ShaderParameterMacros.h"

#include "Public/GlobalShader.h"
#include "Public/ShaderParameterUtils.h"
#include "RHI/Public/RHICommandList.h"

class FKaleidoComputeShader : public FGlobalShader
{
public:

	DECLARE_EXPORTED_TYPE_LAYOUT(FKaleidoComputeShader, KALEIDO_API, Virtual);

	FKaleidoComputeShader() {}
	FKaleidoComputeShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
		FGlobalShader(Initializer)
	{
		DirtyFlagBuffer.Bind(Initializer.ParameterMap, TEXT("DirtyFlagBuffer"));
		OutputInstanceTransformBuffer.Bind(Initializer.ParameterMap, TEXT("OutputInstanceTransformBuffer"));
		InputInstanceTransformBuffer.Bind(Initializer.ParameterMap, TEXT("InputInstanceTransformBuffer"));
		InitialTransformBuffer.Bind(Initializer.ParameterMap, TEXT("InitialTransformBuffer"));
	}

	virtual ~FKaleidoComputeShader() {}

	void BindTransformBuffers(
		FRHICommandList& RHICmdList,
		FUnorderedAccessViewRHIRef DirtyFlagBufferUAV,
		FUnorderedAccessViewRHIRef InstanceTransformBufferUAV,
		FShaderResourceViewRHIRef InstanceTransformBufferSRV,
		FShaderResourceViewRHIRef InitialTransformBufferSRV)
	{
		SetUAVParameter(RHICmdList, RHICmdList.GetBoundComputeShader(), DirtyFlagBuffer,               DirtyFlagBufferUAV);
		SetUAVParameter(RHICmdList, RHICmdList.GetBoundComputeShader(), OutputInstanceTransformBuffer, InstanceTransformBufferUAV);
		SetSRVParameter(RHICmdList, RHICmdList.GetBoundComputeShader(), InputInstanceTransformBuffer,  InstanceTransformBufferSRV);
		SetSRVParameter(RHICmdList, RHICmdList.GetBoundComputeShader(), InitialTransformBuffer,        InitialTransformBufferSRV);
	}

	void UnbindTransformBuffers(FRHICommandList& RHICmdList)
	{
		SetUAVParameter(RHICmdList, RHICmdList.GetBoundComputeShader(), DirtyFlagBuffer,               FUnorderedAccessViewRHIRef());
		SetUAVParameter(RHICmdList, RHICmdList.GetBoundComputeShader(), OutputInstanceTransformBuffer, FUnorderedAccessViewRHIRef());
		SetSRVParameter(RHICmdList, RHICmdList.GetBoundComputeShader(), InputInstanceTransformBuffer,  FShaderResourceViewRHIRef());
		SetSRVParameter(RHICmdList, RHICmdList.GetBoundComputeShader(), InitialTransformBuffer,        FShaderResourceViewRHIRef());
	}

protected:

	LAYOUT_FIELD(FShaderResourceParameter, DirtyFlagBuffer);
	LAYOUT_FIELD(FShaderResourceParameter, OutputInstanceTransformBuffer);
	LAYOUT_FIELD(FShaderResourceParameter, InputInstanceTransformBuffer);
	LAYOUT_FIELD(FShaderResourceParameter, InitialTransformBuffer);
};

IMPLEMENT_UNREGISTERED_TEMPLATE_TYPE_LAYOUT(, FKaleidoComputeShader);

BEGIN_KALEIDO_SHADER_PARAMETER_STRUCT(FKaleidoDefaultShaderParameters, )
	SHADER_PARAMETER(float, InfluencerRadius)
END_KALEIDO_SHADER_PARAMETER_STRUCT()
IMPLEMENT_KALEIDO_SHADER_PARAMETER_STRUCT(FKaleidoDefaultShaderParameters, "KaleidoShaderUniform");

class FKaleidoDefaultShader : public FKaleidoComputeShader
{
public:

	using FParameters = FKaleidoDefaultShaderParameters;
	DECLARE_KALEIDO_COMPUTE_SHADER(FKaleidoDefaultShader)

	FKaleidoDefaultShader() {}
	FKaleidoDefaultShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
		FKaleidoComputeShader(Initializer) {}
};

IMPLEMENT_GLOBAL_SHADER(FKaleidoDefaultShader, "/Plugin/Kaleido/KaleidoDefaultShader.usf", "KaleidoDefaultCS", SF_Compute);

template<>
FKaleidoDefaultShader::FParameters CreateKaleidoShaderParameter<FKaleidoDefaultShader::FParameters>(
	const FKaleidoState&     KaleidoState,
	const FInfluencerState&  InfluencerState,
	const FKaleidoShaderDef& ShaderDef)
{ 
	FKaleidoDefaultShader::FParameters UniformParam;
	SetDefaultKaleidoShaderParameters(UniformParam, KaleidoState, InfluencerState);

	return UniformParam;
}