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

	/* The scale inertia property of the kaleido component. Used as lerp alpha. Clamped between 0 and 1 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, ClampMax = 1), Category = "Kaleido Properties")
	FVector TranslationInertia;

	/* The scale inertia property of the kaleido component. Used as lerp alpha. Clamped between 0 and 1 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, ClampMax = 1), Category = "Kaleido Properties")
	FVector RotationInertia;

	/* The scale inertia property of the kaleido component. Used as lerp alpha. Clamped between 0 and 1 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, ClampMax = 1), Category = "Kaleido Properties")
	FVector ScaleInertia;
	
public:

	UKaleidoInstancedMeshComponent(const FObjectInitializer& Initializer);

	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:

	FStructuredBufferRHIRef    InstanceTransformBuffer;
	FUnorderedAccessViewRHIRef InstanceTransformBufferUAV;

	FStructuredBufferRHIRef    InitialTransformBuffer;
	FShaderResourceViewRHIRef  InitialTransformBufferSRV;

protected:

	void InitComputeResources();
	void ReleaseComputeResources();

	void ProcessTranslationInfluencers(const TArray<const AKaleidoInfluencer*>& TranslationInfluencers);
	void ProcessRotationInfluencers(const TArray<const AKaleidoInfluencer*>& RotationInfluencers);
	void ProcessScaleInfluencers(const TArray<const AKaleidoInfluencer*>& ScaleInfluencers);

	void CopyBackInstanceTransformBuffer_RenderThread();

};
