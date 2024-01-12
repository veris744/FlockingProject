// Copyright Epic Games, Inc. All Rights Reserved.

#include "RebelWolvesProjectile.h"
#include "Components/SphereComponent.h"
#include "Bird.h"
#include "GameManager.h"
#include "RebelWolves/RebelWolvesGameMode.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include <Kismet/KismetSystemLibrary.h>


ARebelWolvesProjectile::ARebelWolvesProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComp->OnComponentHit.AddDynamic(this, &ARebelWolvesProjectile::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0;
	ProjectileMovement->OnProjectileStop.AddDynamic(this, &ARebelWolvesProjectile::OnStop);

	Energy = MaxEnergy;
}

void ARebelWolvesProjectile::Tick(float DeltaTime)
{

	if (!GameManager || GameManager->GetNumBird() == 0)
	{
		ProjectileMovement->Velocity = FVector::ZeroVector;
		return;
	}
	if (Energy <= 0)
	{
		GameManager->TransformPredator(this);
		return;
	}

	float distance = SetTarget();

	/*if (distance <= (CollisionComp->GetScaledSphereRadius() + Target->CollisionComp->GetScaledSphereRadius()))
	{
		TargetWasCatched();
		return;
	}*/

	Velocity = Target->GetActorLocation() - GetActorLocation();

	float speed = DefaultSpeed;
	float EnergyUsed = MinEnergyExp;

	if (distance < AccelerationRadius)
	{
		speed = DefaultSpeed + (distance * (MaxSpeed - DefaultSpeed) / AccelerationRadius);
		EnergyUsed = speed;
	}

	FVector tempVel = Velocity + (Acceleration * DeltaTime);
	Acceleration += ObstacleAvoidance(tempVel);

	tempVel = Velocity + (Acceleration * DeltaTime);
	Acceleration += Reversal(tempVel);

	Velocity.Normalize();
	Velocity *= speed;

	Energy -= (EnergyUsed /100) * DeltaTime;


	//if (GEngine)
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("%f"), Energy));

	ProjectileMovement->Velocity = Velocity;
}



void ARebelWolvesProjectile::BeginPlay()
{
	Super::BeginPlay();

	GameManager = UGameManager::GetGameManager();

	if (GameManager)
	{
		GameManager->AddPredator(this);
	}

}

void ARebelWolvesProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (GameManager)
	{
		GameManager->RemovePredator(this);
	}
}


void ARebelWolvesProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{

	if ((OtherActor != nullptr) && (OtherActor != this) && OtherActor->GetClass()->IsChildOf(ABird::StaticClass()))
	{
		TargetWasCatched();
	}
}

float ARebelWolvesProjectile::SetTarget()
{
	float minDist = -1;


	for (int i = 0; i < GameManager->GetNumBird(); i++)
	{
		float temp = FVector::DistSquared(GetActorLocation(), GameManager->GetBird(i)->GetActorLocation());
		if (minDist == -1 || temp < minDist)
		{
			minDist = temp;
			Target = GameManager->GetBird(i);
		}
	}
	return FMath::Sqrt(minDist);
}

void ARebelWolvesProjectile::OnStop(const FHitResult& Hit)
{
	ProjectileMovement->SetUpdatedComponent(CollisionComp);
	ProjectileMovement->Velocity = FVector(0, 0.0f, 0.0f);
	ProjectileMovement->UpdateComponentVelocity();
}

void ARebelWolvesProjectile::TargetWasCatched()
{
	if (Target)
	{
		Energy += EnergyRecovered;
		if (Energy > MaxEnergy)
		{
			Energy = MaxEnergy;
		}
		GameManager->RemoveBird(Target);
		Target->Destroy();
		Target = nullptr;
	}
	
}


FVector ARebelWolvesProjectile::Reversal(FVector _Velocity)
{
	return (1000 * GameManager->ReversalBehavior(GetActorLocation(), _Velocity, LookAhead));
}


FVector ARebelWolvesProjectile::ObstacleAvoidance(FVector _Velocity)
{
	if (!CollisionComp)	return FVector::Zero();

	FVector weight = FVector::ZeroVector;
	FHitResult Hit;
	FVector End = CollisionComp->GetComponentLocation() + _Velocity.GetSafeNormal() * LookAhead;
	//GetWorld()->LineTraceSingleByChannel(Hit, GetActorLocation(), End, ECC_Visibility, FCollisionQueryParams());
	UKismetSystemLibrary::SphereTraceSingle(GetWorld(), CollisionComp->GetComponentLocation(), End, CollisionComp->GetScaledSphereRadius(),
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false, TArray<AActor*>(), EDrawDebugTrace::None, Hit, true);
	if (Hit.bBlockingHit)
	{
		FVector temp1 = FVector::CrossProduct(Hit.GetActor()->GetActorUpVector(), Hit.ImpactNormal);
		FVector temp2 = -FVector::CrossProduct(Hit.GetActor()->GetActorUpVector(), Hit.ImpactNormal);

		float dist1 = FVector::Dist2D(GetActorLocation() + temp1, GameManager->GetMapCenter());
		float dist2 = FVector::Dist2D(GetActorLocation() + temp2, GameManager->GetMapCenter());

		if (dist1 < dist2)	weight += temp1;
		else weight += temp2;
	}
	//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + weight * 300, FColor::Red, false, 3);
	return (weight * 10000);
}
