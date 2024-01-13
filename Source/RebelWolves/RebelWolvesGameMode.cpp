// Copyright Epic Games, Inc. All Rights Reserved.

#include "RebelWolvesGameMode.h"
#include "RebelWolvesCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "UI/RWUserWidget.h"
#include "Bird.h"
#include "GameManager.h"

ARebelWolvesGameMode::ARebelWolvesGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;
}

void ARebelWolvesGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);


	Manager = NewObject<UGameManager>(this, TEXT("GameManager"));
	if (!Manager->SetConfiguration())
	{
		UE_LOG(LogTemp, Fatal, TEXT("Config File is wrong."));
	}
	if (BirdClass)
	{
		Manager->BirdClass = BirdClass;
	}
	if (FloorClass)
	{
		Manager->FloorClass = FloorClass;
	}
	if (BuildingClass)
	{
		Manager->BuildingClass = BuildingClass;
	}

	if (HUD)
	{
		HUDWidget = CreateWidget<URWUserWidget>(this->GetGameInstance(), HUD);
		HUDWidget->AddToViewport();
		HUDWidget->SetVisibility(ESlateVisibility::Visible);
		Manager->SetHudWidget(HUDWidget);
	}

	Manager->LoadLevel();
}
