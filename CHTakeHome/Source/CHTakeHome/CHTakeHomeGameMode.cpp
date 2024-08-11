// Copyright Epic Games, Inc. All Rights Reserved.

#include "CHTakeHomeGameMode.h"
#include "CHTakeHomeCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACHTakeHomeGameMode::ACHTakeHomeGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
