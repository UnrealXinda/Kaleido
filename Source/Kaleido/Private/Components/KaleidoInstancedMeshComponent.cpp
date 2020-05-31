// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/KaleidoInstancedMeshComponent.h"
#include "Actors/KaleidoInfluencer.h"
#include "KaleidoMacros.h"

#include "Shaders/KaleidoShaders.h"

DECLARE_STATS_GROUP(TEXT("Kaleido"),                STATGROUP_Kaleido,      STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Kaleido Compute Shader"),  STAT_ComputeShader,     STATGROUP_Kaleido);
DECLARE_CYCLE_STAT(TEXT("Kaleido Compute Command"), STAT_ComputeTransforms, STATGROUP_Kaleido);
DECLARE_CYCLE_STAT(TEXT("Copy Back"),               STAT_CopyBack,          STATGROUP_Kaleido);
DECLARE_CYCLE_STAT(TEXT("Clear Dirty Flags"),       STAT_ClearDirtyFlags,   STATGROUP_Kaleido);

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

	void BindShaderBuffers(FRHICommandList& RHICmdList, FUnorderedAccessViewRHIRef DirtyFlagBufferUAV)
	{
		FRHIComputeShader* ComputeShaderRHI = RHICmdList.GetBoundComputeShader();
		SetUAVParameter(RHICmdList, ComputeShaderRHI, DirtyFlagBuffer, DirtyFlagBufferUAV);
	}

	void UnbindShaderBuffers(FRHICommandList& RHICmdList)
	{
		FRHIComputeShader* ComputeShaderRHI = RHICmdList.GetBoundComputeShader();
		SetUAVParameter(RHICmdList, ComputeShaderRHI, DirtyFlagBuffer, FUnorderedAccessViewRHIRef());
	}

private:

	LAYOUT_FIELD(FShaderResourceParameter, DirtyFlagBuffer);
};

IMPLEMENT_GLOBAL_SHADER(FClearDirtyFlagShader, "/Plugin/Kaleido/ClearDirtyFlagShader.usf", "ClearDirtyFlagCS", SF_Compute);

UKaleidoInstancedMeshComponent::UKaleidoInstancedMeshComponent(const FObjectInitializer& Initializer) :
	Super(Initializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	TranslationInertia = FVector(0.1);
	RotationInertia    = FVector(0.1);
	ScaleInertia       = FVector(0.1);

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

	TArray<FKaleidoComputeInfo> ComputeInfos;

	for (AActor* Actor : OverlappingActors)
	{
		if (AKaleidoInfluencer* Influencer = Cast<AKaleidoInfluencer>(Actor))
		{
			FInfluencerState InfluencerState;
			InfluencerState.InfluencerTransform = Influencer->GetActorTransform().ToMatrixWithScale();
			InfluencerState.InfluencerRadius = Influencer->GetInfluencerRadius();

			for (const FKaleidoShaderDef& ShaderDef : Influencer->Shaders)
			{
				if (Kaleido::IsValidShaderDef(ShaderDef) && ShaderDef.bEnabled)
				{
					FKaleidoComputeInfo Info;
					Info.InfluencerState = InfluencerState;
					Info.ShaderDef = ShaderDef;

					ComputeInfos.Add(MoveTemp(Info));
				}
			}
		}
	}

	// Add default compute shader
	FKaleidoComputeInfo DefaultComputeInfo;
	DefaultComputeInfo.ShaderDef.ShaderName = "Default";
	ComputeInfos.Add(MoveTemp(DefaultComputeInfo));

	FKaleidoState KaleidoState;
	KaleidoState.KaleidoTransform   = GetComponentTransform().ToMatrixWithScale();
	KaleidoState.TranslationInertia = TranslationInertia;
	KaleidoState.RotationInertia    = RotationInertia;
	KaleidoState.ScaleInertia       = ScaleInertia;
	KaleidoState.InstanceCount      = GetInstanceCount();

	KaleidoState.DirtyFlagBufferUAV         = DirtyFlagBufferUAV;
	KaleidoState.InitialTransformBufferSRV  = InitialTransformBufferSRV;

	ENQUEUE_RENDER_COMMAND(KaleidoComputeCommand)
	(
		[KaleidoState, ComputeInfos, this](FRHICommandListImmediate& RHICmdList) mutable -> void
		{
			ClearDirtyFlagBuffer_RenderThread(RHICmdList);
			ProcessInfluencers_RenderThread(RHICmdList, KaleidoState, ComputeInfos);
			CopyBackInstanceTransformBuffer_RenderThread(RHICmdList);
		}
	);
}

void UKaleidoInstancedMeshComponent::ProcessInfluencers_RenderThread(FRHICommandListImmediate& RHICmdList, FKaleidoState& KaleidoState, const TArray<FKaleidoComputeInfo>& ComputeInfos)
{
	check(IsInRenderingThread());

	SCOPE_CYCLE_COUNTER(STAT_ComputeTransforms);

	for (const FKaleidoComputeInfo& Info : ComputeInfos)
	{
		// Flip buffers
		FrontBufferIndex ^= 0x01;

		FUnorderedAccessViewRHIRef FrontBufferUAV = InstanceTransformBufferUAVs[FrontBufferIndex];
		FShaderResourceViewRHIRef  BackBufferSRV = InstanceTransformBufferSRVs[FrontBufferIndex ^ 0x01];

		KaleidoState.InstanceTransformBufferSRV = BackBufferSRV;
		KaleidoState.InstanceTransformBufferUAV = FrontBufferUAV;

		Kaleido::ComputeTransforms(RHICmdList, KaleidoState, Info.InfluencerState, Info.ShaderDef);
	}
}

void UKaleidoInstancedMeshComponent::ClearDirtyFlagBuffer_RenderThread(FRHICommandListImmediate& RHICmdList)
{
	check(IsInRenderingThread());

	SCOPE_CYCLE_COUNTER(STAT_ClearDirtyFlags);

	RHICmdList.TransitionResource(
		EResourceTransitionAccess::ERWBarrier,
		EResourceTransitionPipeline::EComputeToCompute,
		DirtyFlagBufferUAV,
		nullptr);

	TShaderMapRef<FClearDirtyFlagShader> KaleidoShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
	RHICmdList.SetComputeShader(KaleidoShader.GetComputeShader());

	// Bind shader buffers
	KaleidoShader->BindShaderBuffers(RHICmdList, DirtyFlagBufferUAV);

	// Dispatch shader
	const int32 ThreadGroupCountX = FMath::CeilToInt(GetInstanceCount() / 128.f);
	const int32 ThreadGroupCountY = 1;
	const int32 ThreadGroupCountZ = 1;
	DispatchComputeShader(RHICmdList, KaleidoShader, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);

	// Unbind shader buffers
	KaleidoShader->UnbindShaderBuffers(RHICmdList);
}

void UKaleidoInstancedMeshComponent::CopyBackInstanceTransformBuffer_RenderThread(FRHICommandListImmediate& RHICmdList)
{
	check(IsInRenderingThread());

	SCOPE_CYCLE_COUNTER(STAT_CopyBack);

	RHICmdList.TransitionResource(
		EResourceTransitionAccess::ERWBarrier,
		EResourceTransitionPipeline::EComputeToCompute,
		InstanceTransformBufferUAVs[FrontBufferIndex],
		nullptr);

	const int32 BufferSize = GetInstanceCount() * sizeof(FMatrix);
	const int32 BackBufferIndex = FrontBufferIndex ^ 0x01;

	FStructuredBufferRHIRef BackBuffer = InstanceTransformBuffers[BackBufferIndex];
	FStructuredBufferRHIRef FrontBuffer = InstanceTransformBuffers[FrontBufferIndex];

	// Copy back transform buffer to SM data
	void* DstPtr = PerInstanceSMData.GetData();
	void* SrcPtr = RHILockStructuredBuffer(FrontBuffer.GetReference(), 0, BufferSize, EResourceLockMode::RLM_ReadOnly);
	FMemory::Memcpy(DstPtr, SrcPtr, BufferSize);
	RHIUnlockStructuredBuffer(FrontBuffer.GetReference());
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
			BUF_UnorderedAccess,                      // Usage
			DirtyFlagBufferCreateInfo                 // Create info
		);
		DirtyFlagBufferUAV = RHICreateUnorderedAccessView(DirtyFlagBuffer, true, false);

		InitialTransformBuffer = RHICreateStructuredBuffer(
			sizeof(FMatrix),                 // Stride
			sizeof(FMatrix) * InstanceCount, // Size
			BUF_ShaderResource,              // Usage
			CreateInfo                       // Create info
		);
		InitialTransformBufferSRV = RHICreateShaderResourceView(InitialTransformBuffer);

		for (int Idx = 0; Idx < UE_ARRAY_COUNT(InstanceTransformBuffers); ++Idx)
		{
			InstanceTransformBuffers[Idx] = RHICreateStructuredBuffer(
				sizeof(FMatrix),                          // Stride
				sizeof(FMatrix) * InstanceCount,          // Size
				BUF_UnorderedAccess | BUF_ShaderResource, // Usage
				CreateInfo                                // Create info
			);
			InstanceTransformBufferSRVs[Idx] = RHICreateShaderResourceView(InstanceTransformBuffers[Idx]);
			InstanceTransformBufferUAVs[Idx] = RHICreateUnorderedAccessView(InstanceTransformBuffers[Idx], true, false);
		}
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
	
	SafeReleaseBufferResource(InitialTransformBuffer);
	SafeReleaseBufferResource(InitialTransformBufferSRV);

	SafeReleaseBufferResource(DirtyFlagBuffer);
	SafeReleaseBufferResource(DirtyFlagBufferUAV);

	for (int Idx = 0; Idx < UE_ARRAY_COUNT(InstanceTransformBuffers); ++Idx)
	{
		SafeReleaseBufferResource(InstanceTransformBuffers[Idx]);
		SafeReleaseBufferResource(InstanceTransformBufferSRVs[Idx]);
		SafeReleaseBufferResource(InstanceTransformBufferUAVs[Idx]);
	}

#undef SafeReleaseBufferResource
}