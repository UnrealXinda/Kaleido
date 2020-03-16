// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KaleidoInfluencer.generated.h"

UENUM(BlueprintType)
enum class EInfluencerShape : uint8
{
	Box,
	Sphere,
	Default,
};

UENUM(BlueprintType)
enum class EInfluencerType : uint8
{
	Default     = 0x00,
	Translation = 0x01 << 0,
	Rotation    = 0x01 << 1,
	Scale       = 0x01 << 2,
};

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

	const TArray<EInfluencerType>& GetInfluencerTypes() const;

	virtual EInfluencerShape GetInfluencerShape() const;	

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Influencer")
	TArray<EInfluencerType> InfluencerTypes;

};
