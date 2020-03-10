// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/KaleidoBoxInfluencer.h"
#include "KaleidoMacros.h"
#include "Components/BoxComponent.h"

AKaleidoBoxInfluencer::AKaleidoBoxInfluencer(const FObjectInitializer& Initializer) :
	Super(Initializer)
{
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetCollisionProfileName(CollisionProfile_Influencer);
	RootComponent = BoxComponent;
}

EInfluencerShape AKaleidoBoxInfluencer::GetInfluencerShape() const
{
	return EInfluencerShape::Box;
}
