// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RWUserWidget.generated.h"



class UImage;
class UTextBlock;
class ARebelWolvesGameMode;
class UButton;

/**
 * 
 */
UCLASS()
class REBELWOLVES_API URWUserWidget : public UUserWidget
{
	GENERATED_BODY()



	UPROPERTY()
		ARebelWolvesGameMode* GameMode;
	

public:
	virtual void NativeConstruct() override;


	UPROPERTY(meta = (BindWidget))
		UImage* CrosshairImage;

	UPROPERTY(meta = (BindWidget))
		UImage* TutorialImage;


	UPROPERTY(meta = (BindWidget))
		UTextBlock* BirdsCount;


	UPROPERTY(meta = (BindWidget))
		UTextBlock* PredatorsCount;

	UPROPERTY(meta = (BindWidget))
		UTextBlock* AmmoCount;

	UPROPERTY(meta = (BindWidget))
		UTextBlock* ResultText;

	UPROPERTY(meta = (BindWidget))
		UButton* RestartButton;

	UFUNCTION()
		void UpdateBirdCount(int num);

	UFUNCTION()
		void UpdatePredatorCount(int num);

	UFUNCTION()
		void UpdateAmmoCount(int num);

	UFUNCTION()
		void OnClickedRestart();

};

