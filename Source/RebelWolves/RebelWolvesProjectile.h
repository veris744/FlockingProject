// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RebelWolvesProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UGameManager;
class ABird;

UCLASS(config=Game)
class ARebelWolvesProjectile : public AActor
{
	GENERATED_BODY()

	/** Sphere collision component */
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	USphereComponent* CollisionComp;

	/** Projectile movement component */
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;



	UPROPERTY()
		UGameManager* GameManager;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	ARebelWolvesProjectile();

	virtual void Tick(float DeltaTime) override;

	/** called when projectile hits something */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Returns CollisionComp subobject **/
	USphereComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ProjectileMovement subobject **/
	UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }


	UPROPERTY(VisibleAnywhere, Category = Predator)
		float DefaultSpeed = 500;


	UPROPERTY(VisibleAnywhere, Category = Predator)
		float MaxSpeed = 900;

	UPROPERTY(VisibleAnywhere, Category = Predator)
		float MinEnergyExp = 200;

	UPROPERTY(VisibleAnywhere, Category = Predator)
		float EnergyRecovered = 5;

	UPROPERTY(VisibleAnywhere, Category = Predator)
		float MaxEnergy = 40;

	UPROPERTY(VisibleAnywhere, Category = Predator)
		float AccelerationRadius = 1000;

	UPROPERTY(EditAnywhere, Category = "Flock")
		float LookAhead = 700;

	UPROPERTY(VisibleAnywhere, Category = Predator)
		FVector Velocity;

	UPROPERTY(VisibleAnywhere, Category = Predator)
		FVector Acceleration;

	UPROPERTY(VisibleAnywhere, Category = Predator)
		ABird* Target;

	UPROPERTY(VisibleAnywhere, Category = Predator)
		float Energy;

	UFUNCTION()
		float SetTarget();

	UFUNCTION()

		void OnStop(const FHitResult& Hit);

	UFUNCTION()
		void TargetWasCatched();

	UFUNCTION()
		FVector Reversal(FVector _Velocity);

	UFUNCTION()
		FVector ObstacleAvoidance(FVector _Velocity);

};

