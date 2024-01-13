// Fill out your copyright notice in the Description page of Project Settings.


#include "RWUserWidget.h"
#include "Components/TextBlock.h"
#include "RebelWolves/RebelWolvesGameMode.h"
#include "RebelWolves/GameManager.h"
#include <RebelWolves/RebelWolvesCharacter.h>
#include <Kismet/GameplayStatics.h>

void URWUserWidget::NativeConstruct()
{
	if (UGameManager::GetGameManager())
	{
		FString s = "BIRDS: ";
		BirdsCount->SetText(FText::FromString(s + FString::FromInt(UGameManager::GetGameManager()->GetNumBird())));

		s = "PREDATORS: ";
		PredatorsCount->SetText(FText::FromString(s + FString::FromInt(UGameManager::GetGameManager()->GetNumPredator())));

	}

	ResultText->SetText(FText::FromString(""));
	
}

void URWUserWidget::UpdateBirdCount(int num)
{
	FString s = "BIRDS: ";
	BirdsCount->SetText(FText::FromString(s + FString::FromInt(num)));
}

void URWUserWidget::UpdatePredatorCount(int num)
{
	FString s = "PREDATORS: ";
	PredatorsCount->SetText(FText::FromString(s + FString::FromInt(num)));
}

void URWUserWidget::UpdateAmmoCount(int num)
{
	FString s = "AMMO: ";
	AmmoCount->SetText(FText::FromString(s + FString::FromInt(num)));
	
}
