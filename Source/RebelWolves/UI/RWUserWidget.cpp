// Fill out your copyright notice in the Description page of Project Settings.


#include "RWUserWidget.h"
#include "Components/TextBlock.h"
#include "RebelWolves/RebelWolvesGameMode.h"
#include "RebelWolves/GameManager.h"

void URWUserWidget::NativeConstruct()
{
	if (UGameManager::GetGameManager())
	{
		BirdsCount->SetText(FText::FromString(FString::FromInt(UGameManager::GetGameManager()->GetNumBird())));
		PredatorsCount->SetText(FText::FromString(FString::FromInt(UGameManager::GetGameManager()->GetNumPredator())));
	}
	
}

void URWUserWidget::UpdateBirdCount(int num)
{
	BirdsCount->SetText(FText::FromString(FString::FromInt(num)));
}

void URWUserWidget::UpdatePredatorCount(int num)
{
	PredatorsCount->SetText(FText::FromString(FString::FromInt(num)));
}
