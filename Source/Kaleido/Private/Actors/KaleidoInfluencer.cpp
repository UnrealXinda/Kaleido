// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/KaleidoInfluencer.h"
#include "Components/SphereComponent.h"

template <typename Type>
Type FKaleidoShaderDef::GetShaderParam(FName ParamName) const
{
	Type Result;
	auto Predicate = [ParamName](const FKaleidoShaderParamEntry& Entry) { return Entry.ParamName == ParamName; };

	if (const FKaleidoShaderParamEntry* Entry = Params.FindByPredicate(Predicate))
	{
		Result = Entry->GetValue<Type>();
	}

	return Result;
}

AKaleidoInfluencer::AKaleidoInfluencer(const FObjectInitializer& Initializer) :
	Super(Initializer)
{
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetCollisionProfileName(TEXT("Influencer"));
	RootComponent = SphereComponent;
}

float AKaleidoInfluencer::GetInfluencerRadius() const
{
	return SphereComponent->GetScaledSphereRadius();
}
