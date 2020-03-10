// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/KaleidoInfluencer.h"
#include "KaleidoBoxInfluencer.generated.h"

/**
 * 
 */
UCLASS()
class KALEIDO_API AKaleidoBoxInfluencer : public AKaleidoInfluencer
{
	GENERATED_BODY()

public:

	AKaleidoBoxInfluencer(const FObjectInitializer& Initializer);

	virtual EInfluencerShape GetInfluencerShape() const override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* BoxComponent;
};
