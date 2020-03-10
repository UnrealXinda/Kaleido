// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "Kaleido.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FKaleidoModule"

void FKaleidoModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("Kaleido"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/Kaleido"), PluginShaderDir);
}

void FKaleidoModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FKaleidoModule, Kaleido)