// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Bird.generated.h"


class USphereComponent;
class UBoxComponent;
class ARebelWolvesGameMode;
class UGameManager;
class UProjectileMovementComponent;
class ARebelWolvesProjectile;


UCLASS()
class REBELWOLVES_API ABird : public AActor
{
	GENERATED_BODY()


	UPROPERTY()
		ARebelWolvesGameMode* GameMode;
	
public:	
	// Sets default values for this actor's properties
	ABird();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	


	/////////////////////////////////////////////////////////////////
	/////						OVERRIDING						/////
	/////////////////////////////////////////////////////////////////

	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UFUNCTION()
		void OnOverlapBegin(class UPrimitiveComponent* OverlapComponent, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, 
			int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


	UFUNCTION()

		void OnStop(const FHitResult& Hit);


	/////////////////////////////////////////////////////////////////
	/////						COMPONENTS						/////
	/////////////////////////////////////////////////////////////////


	//UPROPERTY(VisibleAnywhere)
	//	USceneComponent* DefaultRoot;

	UPROPERTY(VisibleAnywhere, Category = "Components")
		class UStaticMeshComponent* BirdMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
		UBoxComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
		USphereComponent* PerceptionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY()
		UGameManager* GameManager;


	/////////////////////////////////////////////////////////////////
	/////						MOVEMENT						/////
	/////////////////////////////////////////////////////////////////



	UPROPERTY(VisibleAnywhere, Category = "Flock")
		float MaxVelocity = 500;

	
	UPROPERTY(VisibleAnywhere, Category = "Flock")
		float MaxAcceleration = 1000;

	UPROPERTY(VisibleAnywhere, Category = "Flock")
		bool PredatorDetected = false;

	UPROPERTY(VisibleAnywhere, Category = "Flock")
		float FOV = -0.5f;


	/////////////////////////////////////////////////////////////////
	/////						STEERING						/////
	/////////////////////////////////////////////////////////////////

	UPROPERTY(EditAnywhere, Category = "Flock")
		float BirdPerceptionRadius = 1500;

	UPROPERTY(EditAnywhere, Category = "Flock")
		float BirdSeparationRadius = 400;

	UPROPERTY(EditAnywhere, Category = "Flock")
		float LookAhead = 700;

	UPROPERTY()
		ARebelWolvesProjectile* predator;

	UFUNCTION()
		void Flock(float DeltaTime);

	UFUNCTION()
		void RunAway(float DeltaTime);

	UFUNCTION()
		TArray<ABird*> FilterBirds();

	UFUNCTION()
		FVector Cohesion(const TArray<ABird*>& birds);

	UFUNCTION()
		FVector Separation(const TArray<ABird*>& birds);

	UFUNCTION()
		FVector Alignment(const TArray<ABird*>& birds);

	UFUNCTION()
		FVector Reversal(FVector _Velocity);

	UFUNCTION()
		FVector ObstacleAvoidance(FVector _Velocity);
};
