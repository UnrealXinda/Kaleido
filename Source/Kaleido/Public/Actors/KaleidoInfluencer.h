// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KaleidoInfluencer.generated.h"

// Used to define each shader parameter
USTRUCT(BlueprintType)
struct FKaleidoShaderParamEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ParamName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector4 Value;

	FVector4 GetVector4Value() const { return Value; }
	FVector  GetVector3Value() const { return FVector(Value); }
	float    GetFloatValue() const   { return Value.X; }
	int32    GetIntValue() const     { return StaticCast<int32>(Value.X); }
	bool     GetBoolValue() const    { return Value.X != 0.0f; }
};

// Used to define shader parameters
USTRUCT(BlueprintType)
struct FKaleidoShaderDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ShaderName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FKaleidoShaderParamEntry> Params;

	template <typename Type>
	Type GetShaderParam(FName ParamName) const;
};

// Used to pass influencer's current state to render thread
struct FInfluencerState
{
	FMatrix InfluencerTransform;
	float   InfluencerRadius;
};

UCLASS()
class KALEIDO_API AKaleidoInfluencer : public AActor
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Influencer")
	TArray<FKaleidoShaderDef> Shaders;
	
public:	

	AKaleidoInfluencer(const FObjectInitializer& Initializer);

	float GetInfluencerRadius() const;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* SphereComponent;

};
