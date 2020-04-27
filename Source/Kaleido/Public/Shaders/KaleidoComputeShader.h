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

	FKaleidoComputeShader() {}
	FKaleidoComputeShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
		FGlobalShader(Initializer)
	{
		DirtyFlagBuffer.Bind(Initializer.ParameterMap, TEXT("DirtyFlagBuffer"));
		OutputInstanceTransformBuffer.Bind(Initializer.ParameterMap, TEXT("OutputInstanceTransformBuffer"));
		InputInstanceTransformBuffer.Bind(Initializer.ParameterMap, TEXT("InputInstanceTransformBuffer"));
		InitialTransformBuffer.Bind(Initializer.ParameterMap, TEXT("InitialTransformBuffer"));
	}

	void BindTransformBuffers(
		FRHICommandList& RHICmdList,
		FUnorderedAccessViewRHIRef DirtyFlagBufferUAV,
		FUnorderedAccessViewRHIRef InstanceTransformBufferUAV,
		FShaderResourceViewRHIRef InstanceTransformBufferSRV,
		FShaderResourceViewRHIRef InitialTransformBufferSRV)
	{
		SetUAVParameter(RHICmdList, GetComputeShader(), DirtyFlagBuffer,               DirtyFlagBufferUAV);
		SetUAVParameter(RHICmdList, GetComputeShader(), OutputInstanceTransformBuffer, InstanceTransformBufferUAV);
		SetSRVParameter(RHICmdList, GetComputeShader(), InputInstanceTransformBuffer,  InstanceTransformBufferSRV);
		SetSRVParameter(RHICmdList, GetComputeShader(), InitialTransformBuffer,        InitialTransformBufferSRV);
	}

	void UnbindTransformBuffers(FRHICommandList& RHICmdList)
	{
		SetUAVParameter(RHICmdList, GetComputeShader(), DirtyFlagBuffer,               FUnorderedAccessViewRHIRef());
		SetUAVParameter(RHICmdList, GetComputeShader(), OutputInstanceTransformBuffer, FUnorderedAccessViewRHIRef());
		SetSRVParameter(RHICmdList, GetComputeShader(), InputInstanceTransformBuffer,  FShaderResourceViewRHIRef());
		SetSRVParameter(RHICmdList, GetComputeShader(), InitialTransformBuffer,        FShaderResourceViewRHIRef());
	}

	virtual bool Serialize(FArchive& Ar) override
	{
		bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
		Ar << DirtyFlagBuffer;
		Ar << OutputInstanceTransformBuffer;
		Ar << InputInstanceTransformBuffer;
		Ar << InitialTransformBuffer;
		return bShaderHasOutdatedParameters;
	}

protected:

	FShaderResourceParameter DirtyFlagBuffer;
	FShaderResourceParameter OutputInstanceTransformBuffer;
	FShaderResourceParameter InputInstanceTransformBuffer;
	FShaderResourceParameter InitialTransformBuffer;
};

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

IMPLEMENT_SHADER_TYPE(, FKaleidoDefaultShader, TEXT("/Plugin/Kaleido/KaleidoDefaultShader.usf"), TEXT("KaleidoDefaultCS"), SF_Compute);

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