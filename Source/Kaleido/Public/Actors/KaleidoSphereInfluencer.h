// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/KaleidoInfluencer.h"
#include "KaleidoSphereInfluencer.generated.h"

/**
 * 
 */
UCLASS()
class KALEIDO_API AKaleidoSphereInfluencer : public AKaleidoInfluencer
{
	GENERATED_BODY()
	
public:

	AKaleidoSphereInfluencer(const FObjectInitializer& Initializer);

	virtual EInfluencerShape GetInfluencerShape() const override;

	float GetInfluencerRadius() const;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* SphereComponent;

};
