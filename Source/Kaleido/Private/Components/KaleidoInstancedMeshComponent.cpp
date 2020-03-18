// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/KaleidoInstancedMeshComponent.h"
#include "Actors/KaleidoInfluencer.h"
#include "KaleidoMacros.h"

// TODO: remove these includes
#include "Actors/KaleidoSphereInfluencer.h"
#include "Actors/KaleidoBoxInfluencer.h"
#include "Shaders/Scale/InclusiveScaleShader.h"
#include "Shaders/Scale/DefaultScaleShader.h"
#include "Shaders/Rotation/InclusiveRotationShader.h"
#include "Shaders/Rotation/DefaultRotationShader.h"
#include "Shaders/Translation/InclusiveTranslationShader.h"
#include "Shaders/Translation/DefaultTranslationShader.h"

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

	TArray<TWeakObjectPtr<AKaleidoInfluencer>> TranslationInfluencers;
	TArray<TWeakObjectPtr<AKaleidoInfluencer>> RotationInfluencers;
	TArray<TWeakObjectPtr<AKaleidoInfluencer>> ScaleInfluencers;

	for (AActor* Actor : OverlappingActors)
	{
		if (AKaleidoInfluencer* Influencer = Cast<AKaleidoInfluencer>(Actor))
		{
			if (Influencer->TranslationShaderName != NAME_None)
			{
				TranslationInfluencers.Add(Influencer);
			}

			if (Influencer->RotationShaderName != NAME_None)
			{
				RotationInfluencers.Add(Influencer);
			}

			if (Influencer->ScaleShaderName != NAME_None)
			{
				ScaleInfluencers.Add(Influencer);
			}
		}
	}

	// TODO: there are race conditions in terms of when shaders get dispatched.
	// Will try to cache all influencers and their corresponding params and dispatch
	// only one render command.

	ENQUEUE_RENDER_COMMAND(DefaultScaleComputeCommand)
	(
		[TranslationInfluencers, RotationInfluencers, ScaleInfluencers, this](FRHICommandListImmediate& RHICmdList)
		{
			ProcessTranslationInfluencers_RenderThread(RHICmdList, TranslationInfluencers);
			ProcessRotationInfluencers_RenderThread(RHICmdList, RotationInfluencers);
			ProcessScaleInfluencers_RenderThread(RHICmdList, ScaleInfluencers);
			CopyBackInstanceTransformBuffer_RenderThread();
		}
	);
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

void UKaleidoInstancedMeshComponent::ProcessTranslationInfluencers_RenderThread(FRHICommandListImmediate& RHICmdList, const TArray<TWeakObjectPtr<AKaleidoInfluencer>>& Influencers)
{
	// TODO: These are thread unsafe
	FMatrix ComponentTransform = GetComponentTransform().ToMatrixWithScale();

	for (TWeakObjectPtr<AKaleidoInfluencer> Influencer : Influencers)
	{
		if (Influencer.IsValid())
		{
			if (const AKaleidoSphereInfluencer* SphereInfluencer = Cast<AKaleidoSphereInfluencer>(Influencer))
			{
				// TODO: These are thread unsafe
				FMatrix InfluencerTransform = SphereInfluencer->GetActorTransform().ToMatrixWithScale();
				float InfluencerRadius = SphereInfluencer->GetInfluencerRadius();

				FInclusiveTranslationShader::FParameters UniformParam;
				UniformParam.ModelTransform      = ComponentTransform;
				UniformParam.InfluencerTransform = InfluencerTransform;
				UniformParam.TranslationInertia  = TranslationInertia;
				UniformParam.RotationInertia     = RotationInertia;
				UniformParam.ScaleInertia        = ScaleInertia;

				UniformParam.MinTranslation      = 0.0f;    // TODO: these should come from influencer
				UniformParam.MaxTranslation      = 100.0f;  // TODO: these should come from influencer
				UniformParam.InfluencerRadius    = InfluencerRadius;

				ComputeTransforms<FInclusiveTranslationShader>(RHICmdList, UniformParam);
			}
		}
	}

	if (Influencers.Num() == 0)
	{
		FDefaultTranslationShader::FParameters UniformParam;
		UniformParam.ModelTransform      = ComponentTransform;
		UniformParam.TranslationInertia  = TranslationInertia;
		UniformParam.RotationInertia     = RotationInertia;
		UniformParam.ScaleInertia        = ScaleInertia;

		ComputeTransforms<FDefaultTranslationShader>(RHICmdList, UniformParam);
	}
}

void UKaleidoInstancedMeshComponent::ProcessRotationInfluencers_RenderThread(FRHICommandListImmediate& RHICmdList, const TArray<TWeakObjectPtr<AKaleidoInfluencer>>& Influencers)
{
	// TODO: These are thread unsafe
	FMatrix ComponentTransform = GetComponentTransform().ToMatrixWithScale();

	for (TWeakObjectPtr<AKaleidoInfluencer> Influencer : Influencers)
	{
		if (Influencer.IsValid())
		{
			if (const AKaleidoSphereInfluencer* SphereInfluencer = Cast<AKaleidoSphereInfluencer>(Influencer))
			{
				// TODO: These are thread unsafe
				FMatrix InfluencerTransform = SphereInfluencer->GetActorTransform().ToMatrixWithScale();
				float InfluencerRadius = SphereInfluencer->GetInfluencerRadius();

				FInclusiveRotationShader::FParameters UniformParam;
				UniformParam.ModelTransform      = ComponentTransform;
				UniformParam.InfluencerTransform = InfluencerTransform;
				UniformParam.TranslationInertia  = TranslationInertia;
				UniformParam.RotationInertia     = RotationInertia;
				UniformParam.ScaleInertia        = ScaleInertia;

				UniformParam.InfluencerRadius    = InfluencerRadius;

				ComputeTransforms<FInclusiveRotationShader>(RHICmdList, UniformParam);
			}
		}
	}

	if (Influencers.Num() == 0)
	{
		FDefaultRotationShader::FParameters UniformParam;
		UniformParam.ModelTransform      = ComponentTransform;
		UniformParam.TranslationInertia  = TranslationInertia;
		UniformParam.RotationInertia     = RotationInertia;
		UniformParam.ScaleInertia        = ScaleInertia;

		ComputeTransforms<FDefaultRotationShader>(RHICmdList, UniformParam);
	}
}

void UKaleidoInstancedMeshComponent::ProcessScaleInfluencers_RenderThread(FRHICommandListImmediate& RHICmdList, const TArray<TWeakObjectPtr<AKaleidoInfluencer>>& Influencers)
{
	// TODO: These are thread unsafe
	FMatrix ComponentTransform = GetComponentTransform().ToMatrixWithScale();

	for (TWeakObjectPtr<AKaleidoInfluencer> Influencer : Influencers)
	{
		if (Influencer.IsValid())
		{
			if (const AKaleidoSphereInfluencer* SphereInfluencer = Cast<AKaleidoSphereInfluencer>(Influencer))
			{
				// TODO: These are thread unsafe
				FMatrix InfluencerTransform = SphereInfluencer->GetActorTransform().ToMatrixWithScale();
				float InfluencerRadius = SphereInfluencer->GetInfluencerRadius();

				FInclusiveScaleShader::FParameters UniformParam;
				UniformParam.ModelTransform      = ComponentTransform;
				UniformParam.InfluencerTransform = InfluencerTransform;
				UniformParam.TranslationInertia  = TranslationInertia;
				UniformParam.RotationInertia     = RotationInertia;
				UniformParam.ScaleInertia        = ScaleInertia;

				UniformParam.MinScale            = FVector(0.3);       // TODO: these should come from influencer
				UniformParam.MaxScale            = FVector::OneVector; // TODO: these should come from influencer
				UniformParam.InfluencerRadius    = InfluencerRadius;
				UniformParam.Direction           = 0;

				ComputeTransforms<FInclusiveScaleShader>(RHICmdList, UniformParam);
			}
		}
	}

	if (Influencers.Num() == 0)
	{
		FDefaultScaleShader::FParameters UniformParam;
		UniformParam.ModelTransform     = ComponentTransform;
		UniformParam.TranslationInertia = TranslationInertia;
		UniformParam.RotationInertia    = RotationInertia;
		UniformParam.ScaleInertia       = ScaleInertia;

		ComputeTransforms<FDefaultScaleShader>(RHICmdList, UniformParam);
	}
}

template<class ShaderType, class ShaderParamType>
void UKaleidoInstancedMeshComponent::ComputeTransforms(FRHICommandListImmediate& RHICmdList, ShaderParamType Param)
{
	check(IsInRenderingThread());

	TShaderMapRef<ShaderType> KaleidoShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
	RHICmdList.SetComputeShader(KaleidoShader->GetComputeShader());

	// Bind shader buffers
	KaleidoShader->BindTransformBuffers(RHICmdList, InstanceTransformBufferUAV, InitialTransformBufferSRV);

	// Bind shader uniform
	KaleidoShader->SetShaderParameters(RHICmdList, Param);

	// Dispatch shader
	const int ThreadGroupCountX = FMath::CeilToInt(GetInstanceCount() / 128.f);
	const int ThreadGroupCountY = 1;
	const int ThreadGroupCountZ = 1;
	DispatchComputeShader(RHICmdList, *KaleidoShader, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);

	// Unbind shader buffers
	KaleidoShader->UnbindTransformBuffers(RHICmdList);
}
