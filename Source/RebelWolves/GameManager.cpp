// Fill out your copyright notice in the Description page of Project Settings.


#include "GameManager.h"
#include "RebelWolvesProjectile.h"
#include "Bird.h"
#include "UI/RWUserWidget.h"
#include <Misc/DefaultValueHelper.h>
#include <Kismet/GameplayStatics.h>

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


	MaxHeight = BuildingHeights.Max();;

	return true;
}

void UGameManager::LoadLevel()
{
	for (auto Bird : AllBirds)
	{
		Bird->SetActorLocation(FVector(Bird->GetActorLocation().X, Bird->GetActorLocation().Y, MaxHeight - HeightMargin / 2));
	}

	if (!FloorClass)	return;

	FActorSpawnParameters SpawnInfo;
	SpawnInfo;

	AActor* floor = GetWorld()->SpawnActor<AActor>(FloorClass, FVector(0, 0, 0), FRotator(0, 0, 0), SpawnInfo);
	floor->SetActorScale3D(FVector(Size.X / 100, Size.Y / 100, 1));


	if (!BuildingClass)	return;

	int rows = FMath::FloorToInt(Size.X / (BuildingDimentions.X + BuildingMargin));
	int cols = FMath::FloorToInt(Size.Y / (BuildingDimentions.Y + BuildingMargin));


	FVector pos;
	AActor* tempBuilding;
	int num = 0;

	for (int i = 1; i < rows; i+=2)
	{
		for (int j = 1; j < cols; j+=2)
		{
			pos = FVector(i * (BuildingDimentions.X + BuildingMargin) - (Size.X / 2), j * (BuildingDimentions.Y + BuildingMargin) - (Size.Y / 2), 
				BuildingHeights[num]/2);
			tempBuilding = GetWorld()->SpawnActor<AActor>(BuildingClass, pos, FRotator(0, 0, 0), SpawnInfo);
			tempBuilding->SetActorScale3D(FVector(BuildingDimentions.X / 100, BuildingDimentions.Y / 100, BuildingHeights[num] / 100));

			++num;

			if (num >= BuildingHeights.Num())
				break;
		}
		if (num >= BuildingHeights.Num())
			break;
	}

	if (num < BuildingHeights.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Floor not big enough for all buildings"));
	}

	return;
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

	if (LookAheadPos.Z < 800 || LookAheadPos.Z > zBoundary)
	{
		if (LookAheadPos.Z < 800)
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
