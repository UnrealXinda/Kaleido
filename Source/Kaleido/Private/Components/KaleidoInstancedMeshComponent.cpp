// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/KaleidoInstancedMeshComponent.h"
#include "KaleidoMacros.h"

UKaleidoInstancedMeshComponent::UKaleidoInstancedMeshComponent(const FObjectInitializer& Initializer) :
	Super(Initializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

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

		void* DstPtr = InitialTransforms.GetData();
		void* SrcPtr = PerInstanceSMData.GetData();
		FMemory::Memcpy(DstPtr, SrcPtr, InstanceCount * sizeof(FMatrix));

		FRHIResourceCreateInfo CreateInfo(&InitialTransforms);

		TransformBuffer = RHICreateStructuredBuffer(
			sizeof(FMatrix),                          // Stride
			sizeof(FMatrix) * InstanceCount,          // Size
			BUF_UnorderedAccess | BUF_ShaderResource, // Usage
			CreateInfo                                // Create info
		);

		InitialTransformBuffer = RHICreateStructuredBuffer(
			sizeof(FMatrix),                 // Stride
			sizeof(FMatrix) * InstanceCount, // Size
			BUF_ShaderResource,              // Usage
			CreateInfo                       // Create info
		);

		TransformBufferUAV = RHICreateUnorderedAccessView(TransformBuffer, true, false);
		InitialTransformBufferSRV = RHICreateShaderResourceView(InitialTransformBuffer);
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

	SafeReleaseBufferResource(TransformBuffer);
	SafeReleaseBufferResource(TransformBufferUAV);
	SafeReleaseBufferResource(InitialTransformBuffer);
	SafeReleaseBufferResource(InitialTransformBufferSRV);

#undef SafeReleaseBufferResource
}
