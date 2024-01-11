// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RebelWolvesGameMode.generated.h"


class UGameManager;
class ABird;
class ARebelWolvesProjectile;


UCLASS(minimalapi)
class ARebelWolvesGameMode : public AGameModeBase
{
	GENERATED_BODY()



public:
	ARebelWolvesGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;


	UPROPERTY(BlueprintReadOnly)
		UGameManager* Manager;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUDs", Meta = (BlueprintProtected = "true"))
		TSubclassOf<class UUserWidget> HUD;


	UPROPERTY(Transient)
		class URWUserWidget* HUDWidget;


	UPROPERTY(EditDefaultsOnly, Category = Manager)
		TSubclassOf<class ABird> BirdClass;

};



