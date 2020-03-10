// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "KaleidoInstancedMeshComponent.generated.h"

UCLASS(hidecategories = (Object, LOD, Instances), editinlinenew, meta = (BlueprintSpawnableComponent), ClassGroup = Rendering, DisplayName = "KaleidoInstancedMeshComponent")
class KALEIDO_API UKaleidoInstancedMeshComponent : public UInstancedStaticMeshComponent
{
	GENERATED_BODY()
	
public:

	UKaleidoInstancedMeshComponent(const FObjectInitializer& Initializer);

	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:

	void InitComputeResources();
	void ReleaseComputeResources();


	FStructuredBufferRHIRef    TransformBuffer;
	FUnorderedAccessViewRHIRef TransformBufferUAV;

	FStructuredBufferRHIRef    InitialTransformBuffer;
	FShaderResourceViewRHIRef  InitialTransformBufferSRV;
};
