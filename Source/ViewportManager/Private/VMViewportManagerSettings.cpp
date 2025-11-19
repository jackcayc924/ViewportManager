// Copyright jackcayc924 2025. All Rights Reserved.

#include "VMViewportManagerSettings.h"

#include "ViewportManager.h"

UVMViewportManagerSettings::UVMViewportManagerSettings()
{
	bApplyDefaultLayoutOnWorldInit = true;
	bAutoAddMissingLocalPlayers = true;
}

FName UVMViewportManagerSettings::GetCategoryName() const
{
	return TEXT("Plugins");
}

FName UVMViewportManagerSettings::GetSectionName() const
{
	return TEXT("Viewport Manager");
}
