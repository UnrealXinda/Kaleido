// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/KaleidoInfluencer.h"
#include "Components/SphereComponent.h"

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
