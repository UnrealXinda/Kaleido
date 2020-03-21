// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/KaleidoInstancedMeshComponent.h"
#include "Actors/KaleidoInfluencer.h"
#include "KaleidoMacros.h"

#include "Shaders/KaleidoShaders.h"

class FClearDirtyFlagShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FClearDirtyFlagShader, Global)

public:
	FClearDirtyFlagShader() {}
	FClearDirtyFlagShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		DirtyFlagBuffer.Bind(Initializer.ParameterMap, TEXT("DirtyFlagBuffer"));
	}

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM5);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}

	virtual bool Serialize(FArchive& Ar) override
	{
		bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
		Ar << DirtyFlagBuffer;
		return bShaderHasOutdatedParameters;
	}

	void BindShaderBuffers(FRHICommandList& RHICmdList, FUnorderedAccessViewRHIRef DirtyFlagBufferUAV)
	{
		FRHIComputeShader* ComputeShaderRHI = GetComputeShader();
		SetUAVParameter(RHICmdList, ComputeShaderRHI, DirtyFlagBuffer, DirtyFlagBufferUAV);
	}

	void UnbindShaderBuffers(FRHICommandList& RHICmdList)
	{
		FRHIComputeShader* ComputeShaderRHI = GetComputeShader();
		SetUAVParameter(RHICmdList, ComputeShaderRHI, DirtyFlagBuffer, FUnorderedAccessViewRHIRef());
	}

private:

	FShaderResourceParameter DirtyFlagBuffer;
};

IMPLEMENT_SHADER_TYPE(, FClearDirtyFlagShader, TEXT("/Plugin/Kaleido/ClearDirtyFlagShader.usf"), TEXT("ClearDirtyFlagCS"), SF_Compute);

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

	ENQUEUE_RENDER_COMMAND(KaleidoComputeCommand)
	(
		[Influencers, this](FRHICommandListImmediate& RHICmdList)
		{
			ClearDirtyFlagBuffer_RenderThread(RHICmdList);
			ProcessInfluencers_RenderThread(RHICmdList, Influencers);
			CopyBackInstanceTransformBuffer_RenderThread(RHICmdList);
		}
	);
}

void UKaleidoInstancedMeshComponent::ProcessInfluencers_RenderThread(FRHICommandListImmediate& RHICmdList, const TArray<TWeakObjectPtr<AKaleidoInfluencer>>& Influencers)
{
	for (TWeakObjectPtr<AKaleidoInfluencer> Influencer : Influencers)
	{
		if (Influencer.IsValid())
		{
			Kaleido::ComputeTransforms(RHICmdList, Influencer->TranslationShaderName, *this, Influencer.Get());
			Kaleido::ComputeTransforms(RHICmdList, Influencer->RotationShaderName, *this, Influencer.Get());
			Kaleido::ComputeTransforms(RHICmdList, Influencer->ScaleShaderName, *this, Influencer.Get());
		}
	}

	ComputeTransforms_RenderThread<FKaleidoDefaultShader>(RHICmdList, *this, nullptr);
}

void UKaleidoInstancedMeshComponent::ClearDirtyFlagBuffer_RenderThread(FRHICommandListImmediate& RHICmdList)
{
	check(IsInRenderingThread());

	TShaderMapRef<FClearDirtyFlagShader> KaleidoShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
	RHICmdList.SetComputeShader(KaleidoShader->GetComputeShader());

	// Bind shader buffers
	KaleidoShader->BindShaderBuffers(RHICmdList, DirtyFlagBufferUAV);

	// Dispatch shader
	const int32 ThreadGroupCountX = FMath::CeilToInt(GetInstanceCount() / 128.f);
	const int32 ThreadGroupCountY = 1;
	const int32 ThreadGroupCountZ = 1;
	DispatchComputeShader(RHICmdList, *KaleidoShader, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);

	// Unbind shader buffers
	KaleidoShader->UnbindShaderBuffers(RHICmdList);
}

void UKaleidoInstancedMeshComponent::CopyBackInstanceTransformBuffer_RenderThread(FRHICommandListImmediate& RHICmdList)
{
	const int32 BufferSize = GetInstanceCount() * sizeof(FMatrix);
	void* DstPtr = PerInstanceSMData.GetData();
	void* SrcPtr = RHILockStructuredBuffer(InstanceTransformBuffer.GetReference(), 0, BufferSize, EResourceLockMode::RLM_ReadOnly);
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
		FRHIResourceCreateInfo DirtyFlagBufferCreateInfo;

		DirtyFlagBuffer = RHICreateStructuredBuffer(
			sizeof(uint32) * 3,                       // Stride
			sizeof(uint32) * 3 * InstanceCount,       // Size
			BUF_UnorderedAccess | BUF_ShaderResource, // Usage
			DirtyFlagBufferCreateInfo                 // Create info
		);

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

		DirtyFlagBufferUAV = RHICreateUnorderedAccessView(DirtyFlagBuffer, true, false);
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

	SafeReleaseBufferResource(DirtyFlagBuffer);
	SafeReleaseBufferResource(DirtyFlagBufferUAV);
	SafeReleaseBufferResource(InstanceTransformBuffer);
	SafeReleaseBufferResource(InstanceTransformBufferUAV);
	SafeReleaseBufferResource(InitialTransformBuffer);
	SafeReleaseBufferResource(InitialTransformBufferSRV);

#undef SafeReleaseBufferResource
}