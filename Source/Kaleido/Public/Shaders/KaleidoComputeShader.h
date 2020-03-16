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
		InstanceTransformBuffer.Bind(Initializer.ParameterMap, TEXT("InstanceTransformBuffer"));
		InitialTransformBuffer.Bind(Initializer.ParameterMap, TEXT("InitialTransformBuffer"));
	}

	void BindTransformBuffers(FRHICommandList& RHICmdList, FUnorderedAccessViewRHIRef InstanceTransformBufferUAV, FShaderResourceViewRHIRef InitialTransformBufferSRV)
	{
		SetUAVParameter(RHICmdList, GetComputeShader(), InstanceTransformBuffer, InstanceTransformBufferUAV);
		SetSRVParameter(RHICmdList, GetComputeShader(), InitialTransformBuffer, InitialTransformBufferSRV);
	}

	void UnbindTransformBuffers(FRHICommandList& RHICmdList)
	{
		SetUAVParameter(RHICmdList, GetComputeShader(), InstanceTransformBuffer, FUnorderedAccessViewRHIRef());
		SetSRVParameter(RHICmdList, GetComputeShader(), InitialTransformBuffer, FShaderResourceViewRHIRef());
	}

	virtual bool Serialize(FArchive& Ar) override
	{
		bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
		Ar << InstanceTransformBuffer;
		Ar << InitialTransformBuffer;
		return bShaderHasOutdatedParameters;
	}

protected:

	FShaderResourceParameter InstanceTransformBuffer;
	FShaderResourceParameter InitialTransformBuffer;
};