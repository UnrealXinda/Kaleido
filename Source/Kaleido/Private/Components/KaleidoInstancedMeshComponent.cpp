// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/KaleidoInstancedMeshComponent.h"
#include "Actors/KaleidoInfluencer.h"
#include "KaleidoMacros.h"

// TODO: remove these includes
#include "Shaders/KaleidoShaders.h"
#include "Shaders/KaleidoShaderTemplates.h"

UKaleidoInstancedMeshComponent::UKaleidoInstancedMeshComponent(const FObjectInitializer& Initializer) :
	Super(Initializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	TranslationInertia = FVector(0.5);
	RotationInertia    = FVector(0.5);
	ScaleInertia       = FVector(0.5);

	SetCollisionProfileName(CollisionProfile_Kaleido);
}

void UKaleidoInstancedMeshComponent::BeginPlay()
{
	Super::BeginPlay();

	InitComputeResources();
}

void UKaleidoInstancedMeshComponent::BeginDestroy()
{
	Super::BeginDestroy();

	ReleaseComputeResources();
}

void UKaleidoInstancedMeshComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TickTransforms();

	// Force recreation of the render data when proxy is created
	InstanceUpdateCmdBuffer.NumEdits++;

	MarkRenderStateDirty();
}

void UKaleidoInstancedMeshComponent::TickTransforms()
{
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors);

	TArray<TWeakObjectPtr<AKaleidoInfluencer>> Influencers;

	for (AActor* Actor : OverlappingActors)
	{
		if (AKaleidoInfluencer* Influencer = Cast<AKaleidoInfluencer>(Actor))
		{
			Influencers.Add(Influencer);
		}
	}

	ENQUEUE_RENDER_COMMAND(DefaultScaleComputeCommand)
	(
		[Influencers, this](FRHICommandListImmediate& RHICmdList)
		{
			ProcessInfluencers_RenderThread(RHICmdList, Influencers);
			CopyBackInstanceTransformBuffer_RenderThread();
		}
	);
}

void UKaleidoInstancedMeshComponent::ProcessInfluencers_RenderThread(FRHICommandListImmediate& RHICmdList, const TArray<TWeakObjectPtr<AKaleidoInfluencer>>& Influencers)
{
	bool bHasTranslationShader = false;
	bool bHasRotationShader    = false;
	bool bHasScaleShader       = false;

	for (TWeakObjectPtr<AKaleidoInfluencer> InfluencerPtr : Influencers)
	{
		if (InfluencerPtr.IsValid())
		{
			if (const AKaleidoInfluencer* Influencer = Cast<AKaleidoInfluencer>(InfluencerPtr))
			{
				if (Influencer->TranslationShaderName != NAME_None)
				{
					bHasTranslationShader = true;
					ComputeTransforms_RenderThread<FInclusiveTranslationShader>(RHICmdList, *this, Influencer);
				}

				if (Influencer->RotationShaderName != NAME_None)
				{
					bHasRotationShader = true;
					ComputeTransforms_RenderThread<FInclusiveRotationShader>(RHICmdList, *this, Influencer);
				}

				if (Influencer->ScaleShaderName != NAME_None)
				{
					bHasScaleShader = true;
					ComputeTransforms_RenderThread<FInclusiveScaleShader>(RHICmdList, *this, Influencer);
				}
			}
		}
	}

	if (!bHasTranslationShader)
	{
		ComputeTransforms_RenderThread<FDefaultTranslationShader>(RHICmdList, *this, nullptr);
	}

	if (!bHasRotationShader)
	{
		ComputeTransforms_RenderThread<FDefaultRotationShader>(RHICmdList, *this, nullptr);
	}

	if (!bHasScaleShader)
	{
		ComputeTransforms_RenderThread<FDefaultScaleShader>(RHICmdList, *this, nullptr);
	}
}

void UKaleidoInstancedMeshComponent::CopyBackInstanceTransformBuffer_RenderThread()
{
	const int32 BufferSize = GetInstanceCount() * sizeof(FMatrix);
	void* SrcPtr = RHILockStructuredBuffer(InstanceTransformBuffer.GetReference(), 0, BufferSize, EResourceLockMode::RLM_ReadOnly);
	void* DstPtr = PerInstanceSMData.GetData();
	FMemory::Memcpy(DstPtr, SrcPtr, BufferSize);
	RHIUnlockStructuredBuffer(InstanceTransformBuffer.GetReference());
}

void UKaleidoInstancedMeshComponent::InitComputeResources()
{
	// Release buffer before allocating new ones
	ReleaseComputeResources();

	// Copy current transform to array
	const int32 InstanceCount = GetInstanceCount();

	if (InstanceCount > 0)
	{
		TResourceArray<FMatrix> InitialTransforms;
		InitialTransforms.AddUninitialized(InstanceCount);

		const int32 BufferSize = InstanceCount * sizeof(FMatrix);
		void* DstPtr = InitialTransforms.GetData();
		void* SrcPtr = PerInstanceSMData.GetData();
		FMemory::Memcpy(DstPtr, SrcPtr, BufferSize);

		FRHIResourceCreateInfo CreateInfo(&InitialTransforms);

		InitialTransformBuffer = RHICreateStructuredBuffer(
			sizeof(FMatrix),                 // Stride
			sizeof(FMatrix) * InstanceCount, // Size
			BUF_ShaderResource,              // Usage
			CreateInfo                       // Create info
		);

		InstanceTransformBuffer = RHICreateStructuredBuffer(
			sizeof(FMatrix),                          // Stride
			sizeof(FMatrix) * InstanceCount,          // Size
			BUF_UnorderedAccess | BUF_ShaderResource, // Usage
			CreateInfo                                // Create info
		);

		InitialTransformBufferSRV = RHICreateShaderResourceView(InitialTransformBuffer);
		InstanceTransformBufferUAV = RHICreateUnorderedAccessView(InstanceTransformBuffer, true, false);
	}
}

void UKaleidoInstancedMeshComponent::ReleaseComputeResources()
{
#define SafeReleaseBufferResource(Buffer)   \
	do {                                    \
		if (Buffer.IsValid()) {             \
			Buffer->Release();              \
		}                                   \
	} while(0);

	SafeReleaseBufferResource(InstanceTransformBuffer);
	SafeReleaseBufferResource(InstanceTransformBufferUAV);
	SafeReleaseBufferResource(InitialTransformBuffer);
	SafeReleaseBufferResource(InitialTransformBufferSRV);

#undef SafeReleaseBufferResource
}