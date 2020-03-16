// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "Kaleido.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FKaleidoModule"

void FKaleidoModule::StartupModule()
{
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("Kaleido"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/Kaleido"), PluginShaderDir);
}

void FKaleidoModule::ShutdownModule()
{
	ResetAllShaderSourceDirectoryMappings();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FKaleidoModule, Kaleido)