// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/KaleidoInfluencer.h"
#include "Components/SphereComponent.h"

#define GetShaderParamImpl(Type, GetFunc, ParamName) \
	Type Result;                \
	const FKaleidoShaderParamEntry* Entry = Params.FindByPredicate([ParamName](const FKaleidoShaderParamEntry& Entry) \
	{ return Entry.ParamName == ParamName; }); \
	if (Entry) Result = Entry->GetFunc();  \
	return Result; \
//
//template <typename Type>
//Type FKaleidoShaderDef::GetShaderParam<Type>(FName ParamName) const
//{
//	Type Result;
//	return Result;
//}

template <>
FVector4 FKaleidoShaderDef::GetShaderParam<FVector4>(FName ParamName) const
{
	GetShaderParamImpl(FVector4, GetVector4Value, ParamName)
}

template <>
FVector FKaleidoShaderDef::GetShaderParam<FVector>(FName ParamName) const
{
	GetShaderParamImpl(FVector, GetVector3Value, ParamName)
}

template <>
float FKaleidoShaderDef::GetShaderParam<float>(FName ParamName) const
{
	GetShaderParamImpl(float, GetFloatValue, ParamName)
}

template <>
int32 FKaleidoShaderDef::GetShaderParam<int32>(FName ParamName) const
{
	GetShaderParamImpl(int32, GetIntValue, ParamName)
}

template <>
bool FKaleidoShaderDef::GetShaderParam<bool>(FName ParamName) const
{
	GetShaderParamImpl(bool, GetBoolValue, ParamName)
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
