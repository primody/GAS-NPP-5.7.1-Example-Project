// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetPredictionGAS5_5GameMode.h"
#include "NetPredictionGAS5_5Character.h"
#include "UObject/ConstructorHelpers.h"

ANetPredictionGAS5_5GameMode::ANetPredictionGAS5_5GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
