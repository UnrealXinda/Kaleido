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
	
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors);

	TArray<const AKaleidoInfluencer*> TranslationInfluencers;
	TArray<const AKaleidoInfluencer*> RotationInfluencers;
	TArray<const AKaleidoInfluencer*> ScaleInfluencers;

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

	ProcessTranslationInfluencers(TranslationInfluencers);
	ProcessRotationInfluencers(RotationInfluencers);
	ProcessScaleInfluencers(ScaleInfluencers);

	// Force recreation of the render data when proxy is created
	InstanceUpdateCmdBuffer.NumEdits++;

	MarkRenderStateDirty();
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

void UKaleidoInstancedMeshComponent::ProcessTranslationInfluencers(const TArray<const AKaleidoInfluencer*>& TranslationInfluencers)
{

}

void UKaleidoInstancedMeshComponent::ProcessRotationInfluencers(const TArray<const AKaleidoInfluencer*>& RotationInfluencers)
{
	FMatrix ComponentTransform = GetComponentTransform().ToMatrixWithScale();

	for (const AKaleidoInfluencer* Influencer : RotationInfluencers)
	{
		if (const AKaleidoSphereInfluencer* SphereInfluencer = Cast<AKaleidoSphereInfluencer>(Influencer))
		{
			FMatrix InfluencerTransform = SphereInfluencer->GetActorTransform().ToMatrixWithScale();
			float InfluencerRadius = SphereInfluencer->GetInfluencerRadius();

			ENQUEUE_RENDER_COMMAND(InfluencerComputeCommand)
			(
				[InfluencerTransform, ComponentTransform, InfluencerRadius, this](FRHICommandListImmediate& RHICmdList)
				{
					check(IsInRenderingThread());

					TShaderMapRef<FInclusiveRotationShader> KaleidoShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
					RHICmdList.SetComputeShader(KaleidoShader->GetComputeShader());

					// Bind shader buffers
					KaleidoShader->BindTransformBuffers(RHICmdList, InstanceTransformBufferUAV, InitialTransformBufferSRV);

					// Bind shader uniform
					FInclusiveRotationShader::FParameters UniformParam;
					UniformParam.ModelTransform      = ComponentTransform;
					UniformParam.InfluencerTransform = InfluencerTransform;
					UniformParam.TranslationInertia  = TranslationInertia;
					UniformParam.RotationInertia     = RotationInertia;
					UniformParam.ScaleInertia        = ScaleInertia;

					UniformParam.InfluencerRadius = InfluencerRadius;
					KaleidoShader->SetShaderParameters(RHICmdList, UniformParam);

					// Dispatch shader
					const int ThreadGroupCountX = FMath::CeilToInt(GetInstanceCount() / 128.f);
					const int ThreadGroupCountY = 1;
					const int ThreadGroupCountZ = 1;
					DispatchComputeShader(RHICmdList, *KaleidoShader, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);

					// Unbind shader buffers
					KaleidoShader->UnbindTransformBuffers(RHICmdList);

					// TODO: not to do this on every compute pass, only on the last pass
					// Read back the transform buffer.
					CopyBackInstanceTransformBuffer_RenderThread();
				}
			);
		}
	}

	if (RotationInfluencers.Num() == 0)
	{
		ENQUEUE_RENDER_COMMAND(DefaultRotationComputeCommand)
		(
			[ComponentTransform, this](FRHICommandListImmediate& RHICmdList)
			{
				check(IsInRenderingThread());

				TShaderMapRef<FDefaultRotationShader> KaleidoShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
				RHICmdList.SetComputeShader(KaleidoShader->GetComputeShader());

				// Bind shader buffers
				KaleidoShader->BindTransformBuffers(RHICmdList, InstanceTransformBufferUAV, InitialTransformBufferSRV);

				// Bind shader uniform
				FDefaultRotationShader::FParameters UniformParam;
				UniformParam.ModelTransform      = ComponentTransform;
				UniformParam.TranslationInertia  = TranslationInertia;
				UniformParam.RotationInertia     = RotationInertia;
				UniformParam.ScaleInertia        = ScaleInertia;
				KaleidoShader->SetShaderParameters(RHICmdList, UniformParam);

				// Dispatch shader
				const int ThreadGroupCountX = FMath::CeilToInt(GetInstanceCount() / 128.f);
				const int ThreadGroupCountY = 1;
				const int ThreadGroupCountZ = 1;
				DispatchComputeShader(RHICmdList, *KaleidoShader, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);

				// Unbind shader buffers
				KaleidoShader->UnbindTransformBuffers(RHICmdList);

				// TODO: not to do this on every compute pass, only on the last pass
				// Read back the transform buffer.
				CopyBackInstanceTransformBuffer_RenderThread();
			}
		);
	}
}

void UKaleidoInstancedMeshComponent::ProcessScaleInfluencers(const TArray<const AKaleidoInfluencer*>& ScaleInfluencers)
{
	FMatrix ComponentTransform = GetComponentTransform().ToMatrixWithScale();

	for (const AKaleidoInfluencer* Influencer : ScaleInfluencers)
	{
		if (const AKaleidoSphereInfluencer* SphereInfluencer = Cast<AKaleidoSphereInfluencer>(Influencer))
		{
			FMatrix InfluencerTransform = SphereInfluencer->GetActorTransform().ToMatrixWithScale();
			float InfluencerRadius = SphereInfluencer->GetInfluencerRadius();

			ENQUEUE_RENDER_COMMAND(InfluencerComputeCommand)
			(
				[InfluencerTransform, ComponentTransform, InfluencerRadius, this](FRHICommandListImmediate& RHICmdList)
				{
					check(IsInRenderingThread());

					TShaderMapRef<FInclusiveScaleShader> KaleidoShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
					RHICmdList.SetComputeShader(KaleidoShader->GetComputeShader());

					// Bind shader buffers
					KaleidoShader->BindTransformBuffers(RHICmdList, InstanceTransformBufferUAV, InitialTransformBufferSRV);

					// Bind shader uniform
					FInclusiveScaleShader::FParameters UniformParam;
					UniformParam.ModelTransform      = ComponentTransform;
					UniformParam.InfluencerTransform = InfluencerTransform;
					UniformParam.TranslationInertia  = TranslationInertia;
					UniformParam.RotationInertia     = RotationInertia;
					UniformParam.ScaleInertia        = ScaleInertia;

					UniformParam.MinScale         = FVector(0.3);       // TODO: these should come from influencer
					UniformParam.MaxScale         = FVector::OneVector; // TODO: these should come from influencer
					UniformParam.InfluencerRadius = InfluencerRadius;
					UniformParam.Direction        = 0;
					KaleidoShader->SetShaderParameters(RHICmdList, UniformParam);

					// Dispatch shader
					const int ThreadGroupCountX = FMath::CeilToInt(GetInstanceCount() / 128.f);
					const int ThreadGroupCountY = 1;
					const int ThreadGroupCountZ = 1;
					DispatchComputeShader(RHICmdList, *KaleidoShader, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);

					// Unbind shader buffers
					KaleidoShader->UnbindTransformBuffers(RHICmdList);

					// TODO: not to do this on every compute pass, only on the last pass
					// Read back the transform buffer.
					CopyBackInstanceTransformBuffer_RenderThread();
				}
			);
		}
	}

	if (ScaleInfluencers.Num() == 0)
	{
		ENQUEUE_RENDER_COMMAND(DefaultScaleComputeCommand)
		(
			[ComponentTransform, this](FRHICommandListImmediate& RHICmdList)
			{
				check(IsInRenderingThread());

				TShaderMapRef<FDefaultScaleShader> KaleidoShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
				RHICmdList.SetComputeShader(KaleidoShader->GetComputeShader());

				// Bind shader buffers
				KaleidoShader->BindTransformBuffers(RHICmdList, InstanceTransformBufferUAV, InitialTransformBufferSRV);

				// Bind shader uniform
				FDefaultScaleShader::FParameters UniformParam;
				UniformParam.ModelTransform      = ComponentTransform;
				UniformParam.TranslationInertia  = TranslationInertia;
				UniformParam.RotationInertia     = RotationInertia;
				UniformParam.ScaleInertia        = ScaleInertia;
				KaleidoShader->SetShaderParameters(RHICmdList, UniformParam);

				// Dispatch shader
				const int ThreadGroupCountX = FMath::CeilToInt(GetInstanceCount() / 128.f);
				const int ThreadGroupCountY = 1;
				const int ThreadGroupCountZ = 1;
				DispatchComputeShader(RHICmdList, *KaleidoShader, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);

				// Unbind shader buffers
				KaleidoShader->UnbindTransformBuffers(RHICmdList);

				// TODO: not to do this on every compute pass, only on the last pass
				// Read back the transform buffer.
				CopyBackInstanceTransformBuffer_RenderThread();
			}
		);
	}
}