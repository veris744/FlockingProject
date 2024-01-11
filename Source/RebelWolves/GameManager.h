// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameManager.generated.h"

/**
 * 
 */
UCLASS()
class REBELWOLVES_API UGameManager : public UObject
{
	GENERATED_BODY()

	static UGameManager* Instance;


	UPROPERTY();
	TArray<ABird*> AllBirds;

	UPROPERTY();
	TArray<ARebelWolvesProjectile*> AllPredators;


	UPROPERTY(Transient)
		class URWUserWidget* HUDWidget;



	const float kCohesion = 200;
	const float kSeparation = 350;
	const float kAlignment = 100;
	const float kAvoidance = 10000;
	FVector MapCenter = FVector::ZeroVector;
	
public:
	UGameManager()
	{
		Instance = this;
	}

	void SetHudWidget(URWUserWidget* _HUDWidget) { HUDWidget = _HUDWidget; };

	bool SetConfiguration();

	static UGameManager* GetGameManager();

	FVector2D Size;
	int MaxHeight;
	TArray<float> BuildingHeights;

	constexpr float GetCohesionFactor() { return kCohesion; };
	constexpr float GetSeparationFactor() { return kSeparation; };
	constexpr float GetAlignmentFactor() { return kAlignment; };
	constexpr float GetAvoidanceFactor() { return kAvoidance; };
	inline FVector GetMapCenter() { return MapCenter; };

	UFUNCTION()
		void AddBird(ABird* _bird);

	UFUNCTION()
		void RemoveBird(ABird* _bird);

	UFUNCTION()
		ABird* GetBird(int i);

	UFUNCTION()
		inline int GetNumBird() { return AllBirds.Num(); };


	UFUNCTION()
		void AddPredator(ARebelWolvesProjectile* _predator);

	UFUNCTION()
		void RemovePredator(ARebelWolvesProjectile* _predator);

	UFUNCTION()
		ARebelWolvesProjectile* GetPredator(int i);

	UFUNCTION()
		inline int GetNumPredator() { return AllPredators.Num(); };


	UFUNCTION()
		void TransformPredator(ARebelWolvesProjectile* predator);


	UPROPERTY()
		TSubclassOf<class ABird> BirdClass;
};
