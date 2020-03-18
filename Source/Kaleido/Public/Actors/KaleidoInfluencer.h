// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KaleidoInfluencer.generated.h"

UCLASS()
class KALEIDO_API AKaleidoInfluencer : public AActor
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Influencer")
	FName TranslationShaderName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Influencer")
	FName RotationShaderName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Influencer")
	FName ScaleShaderName;
	
public:	

	AKaleidoInfluencer(const FObjectInitializer& Initializer);

	float GetInfluencerRadius() const;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* SphereComponent;

};
