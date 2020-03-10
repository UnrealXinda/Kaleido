// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/KaleidoInfluencer.h"

AKaleidoInfluencer::AKaleidoInfluencer(const FObjectInitializer& Initializer) :
	Super(Initializer)
{

}

const TArray<EInfluencerType>& AKaleidoInfluencer::GetInfluencerTypes() const
{
	return InfluencerTypes;
}

EInfluencerShape AKaleidoInfluencer::GetInfluencerShape() const
{
	return EInfluencerShape::Default;
}

