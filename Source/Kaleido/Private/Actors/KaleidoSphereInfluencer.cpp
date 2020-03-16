// Fill out your copyright notice in the Description page of Project Settings.


#include "KaleidoSphereInfluencer.h"
#include "Components/SphereComponent.h"

AKaleidoSphereInfluencer::AKaleidoSphereInfluencer(const FObjectInitializer& Initializer) :
	Super(Initializer)
{
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetCollisionProfileName(TEXT("Influencer"));
	RootComponent = SphereComponent;
}

EInfluencerShape AKaleidoSphereInfluencer::GetInfluencerShape() const
{
	return EInfluencerShape::Sphere;
}

float AKaleidoSphereInfluencer::GetInfluencerRadius() const
{
	return SphereComponent->GetScaledSphereRadius();
}
