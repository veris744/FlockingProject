// Fill out your copyright notice in the Description page of Project Settings.


#include "GameManager.h"
#include "RebelWolvesProjectile.h"
#include "Bird.h"
#include "UI/RWUserWidget.h"
#include <Misc/DefaultValueHelper.h>

UGameManager* UGameManager::Instance = nullptr;

bool UGameManager::SetConfiguration()
{
	if (!GConfig) return false;


	TArray<FString> tempSize;

	GConfig->GetArray(TEXT("Setup"),
		TEXT("Size"),
		tempSize,
		GEditorPerProjectIni);

	if (tempSize.Num() != 2)
	{
		return false;
	}


	float x;
	float y;
	if (!FDefaultValueHelper::ParseFloat(tempSize[0], x)
		|| !FDefaultValueHelper::ParseFloat(tempSize[1], y))
	{
		return false;
	}

	Size.X = x;
	Size.Y = y;


	TArray<FString> tempHeights;

	GConfig->GetArray(TEXT("Setup"),
		TEXT("Height"),
		tempHeights,
		GEditorPerProjectIni);

	float h;

	for (auto val : tempHeights)
	{
		if (!FDefaultValueHelper::ParseFloat(val, h))	return false;

		BuildingHeights.Add(h);
	}

	int tempMaxHeight;

	GConfig->GetInt(TEXT("Setup"),
		TEXT("MaxHeight"),
		tempMaxHeight,
		GEditorPerProjectIni);

	if (tempMaxHeight < 0)	return false;

	MaxHeight = tempMaxHeight;

	return true;
}

UGameManager* UGameManager::GetGameManager()
{
	return Instance;
}



void UGameManager::AddBird(ABird* _bird)
{
	AllBirds.Add(_bird);
	HUDWidget->UpdateBirdCount(AllBirds.Num());
}

void UGameManager::RemoveBird(ABird* _bird)
{
	if (AllBirds.Contains(_bird))
	{
		AllBirds.Remove(_bird);
		HUDWidget->UpdateBirdCount(AllBirds.Num());
	}
}

ABird* UGameManager::GetBird(int i)
{
	if (i >= AllBirds.Num())	return nullptr;

	return AllBirds[i];
}

void UGameManager::AddPredator(ARebelWolvesProjectile* _predator)
{
	AllPredators.Add(_predator);
	HUDWidget->UpdatePredatorCount(AllPredators.Num());
}

void UGameManager::RemovePredator(ARebelWolvesProjectile* _predator)
{
	if (AllPredators.Contains(_predator))
	{
		for (auto bird : AllBirds)
		{
			if (bird->PredatorsInRange.Contains(_predator))
			{
				bird->PredatorsInRange.Remove(_predator);
			}
		}

		AllPredators.Remove(_predator);
		HUDWidget->UpdatePredatorCount(AllPredators.Num());
	}
}

ARebelWolvesProjectile* UGameManager::GetPredator(int i)
{
	if (i >= AllPredators.Num())	return nullptr;

	return AllPredators[i];
}


void UGameManager::TransformPredator(ARebelWolvesProjectile* predator)
{
	FVector predatorLocation = predator->GetActorLocation();
	FRotator predatorRotation = predator->GetActorRotation();
	FActorSpawnParameters SpawnInfo;

	RemovePredator(predator);
	predator->Destroy();
	ABird* newbird = GetWorld()->SpawnActor<ABird>(BirdClass, predatorLocation, predatorRotation, SpawnInfo);
	AddBird(newbird);
}

FVector UGameManager::ReversalBehavior(FVector Location, FVector _Velocity, float LookAhead)
{
	FVector LookAheadPos = Location + _Velocity.GetSafeNormal() * LookAhead;

	float xBoundary = Size.X / 2;
	float yBoundary = Size.Y / 2;
	float zBoundary = MaxHeight;

	FVector weight = FVector::ZeroVector;
	//FVector wtf = GetActorLocation();
	//DrawDebugLine(GetWorld(), wtf, LookAheadPos, FColor::Red, false, 3);

	if (LookAheadPos.X < -xBoundary || LookAheadPos.X > xBoundary)
	{
		if (LookAheadPos.X < 0)
			weight.X = 1;
		else
			weight.X = -1;
	}

	if (LookAheadPos.Y < -yBoundary || LookAheadPos.Y > yBoundary)
	{
		if (LookAheadPos.Y < 0)
			weight.Y = 1;
		else
			weight.Y = -1;
	}

	if (LookAheadPos.Z < 1000 || LookAheadPos.Z > zBoundary)
	{
		if (LookAheadPos.Z < 1000)
			weight.Z = 1;
		else
			weight.Z = -1;
	}

	return weight;
}

void UGameManager::UpdateAmmoUI(int ammo)
{
	HUDWidget->UpdateAmmoCount(ammo);
}
