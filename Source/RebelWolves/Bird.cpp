// Fill out your copyright notice in the Description page of Project Settings.


#include "Bird.h"
#include <Components/SphereComponent.h>
#include <Components/BoxComponent.h>
#include "GameFramework/PawnMovementComponent.h"
#include "GameManager.h"
#include "RebelWolvesProjectile.h"
#include <GameFramework/ProjectileMovementComponent.h>
#include <Kismet/KismetSystemLibrary.h>

// Sets default values
ABird::ABird()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BirdMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BirdMesh"));
	//BirdMesh->SetupAttachment(RootComponent);
	BirdMesh->SetHiddenInGame(false);

	SetRootComponent(BirdMesh);

	CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComp"));
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollisionComp->SetupAttachment(RootComponent);

	PerceptionComp = CreateDefaultSubobject<USphereComponent>(TEXT("PerceptionComp"));
	PerceptionComp->BodyInstance.SetCollisionProfileName("Perception");
	PerceptionComp->SetupAttachment(RootComponent);
	PerceptionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PerceptionComp->OnComponentBeginOverlap.AddDynamic(this, &ABird::OnOverlapBegin);
	//PerceptionComp->OnComponentEndOverlap.AddDynamic(this, &ABird::OnOverlapEnd);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = RootComponent;
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->ProjectileGravityScale = 0;
	ProjectileMovement->OnProjectileStop.AddDynamic(this, &ABird::OnStop);
	
}

// Called when the game starts or when spawned
void ABird::BeginPlay()
{
	Super::BeginPlay();
	
	
	GameManager = UGameManager::GetGameManager();

	if (GameManager)
	{
		GameManager->AddBird(this);
	}

	
	ProjectileMovement->Velocity = FVector(FMath::RandRange(0, 100), FMath::RandRange(0, 100), FMath::RandRange(0, 100));

}

// Called every frame
void ABird::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GameManager == nullptr)	return;
	

	FVector rot = -FVector::CrossProduct(GetActorUpVector(), ProjectileMovement->Velocity);
	SetActorRotation(FRotator(0, rot.Rotation().Yaw, 0));

	if (PredatorDetected)
	{
		RunAway(DeltaTime);
	}
	else
	{
		Flock(DeltaTime);
	}

}


void ABird::OnOverlapBegin(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
	if (OtherActor->GetClass()->IsChildOf(ARebelWolvesProjectile::StaticClass()) )
	{
		PredatorDetected = true;
		ARebelWolvesProjectile * predator = Cast<ARebelWolvesProjectile>(OtherActor);
		if (!PredatorsInRange.Contains(predator))
		{
			PredatorsInRange.Add(predator);
		}
	}
	
}


void ABird::OnStop(const FHitResult& Hit)
{
	ProjectileMovement->SetUpdatedComponent(RootComponent);
	ProjectileMovement->Velocity = Hit.ImpactNormal;
	ProjectileMovement->UpdateComponentVelocity();
}


void ABird::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (GameManager)
	{
		GameManager->RemoveBird(this);
	}

}


TArray<ABird*> ABird::FilterBirds()
{
	TArray<ABird*> FilteredBirds;

	for (int i = 0; i < GameManager->GetNumBird(); i++)
	{
		ABird* temp = GameManager->GetBird(i);

		if (temp == this)	continue;
		
		if (FVector::DistSquared(GetActorLocation(), temp->GetActorLocation()) < BirdPerceptionRadius * BirdPerceptionRadius)
		{
			FilteredBirds.Add(temp);
		}
	}

	return FilteredBirds;
}


FVector ABird::Cohesion(const TArray<ABird*>& birds)
{
	FVector AveragePosition = FVector::ZeroVector;
	for (auto bird : birds)
	{
		AveragePosition += bird->GetActorLocation();
	}

	AveragePosition = AveragePosition / birds.Num();
	FVector steer = AveragePosition - GetActorLocation();
	steer.Normalize();

	return (steer * GameManager->GetCohesionFactor());
}


FVector ABird::Separation(const TArray<ABird*>& birds)
{
	FVector SeparationDir = FVector::ZeroVector;
	float ProximityFactor = 0.0f;
	FVector steer = FVector::ZeroVector;

	int numBirds = birds.Num();

	for (auto bird : birds)
	{
		SeparationDir = GetActorLocation() - bird->GetActorLocation();
		ProximityFactor = 1 - (SeparationDir.Length() / BirdSeparationRadius);

		if (ProximityFactor < 0)
		{
			--numBirds;
			continue;
		}
		SeparationDir.Normalize();

		steer += (ProximityFactor * SeparationDir);
	}

	if (numBirds > 1)
		steer = steer / numBirds;

	steer.Normalize();

	return (steer * GameManager->GetSeparationFactor());
}



FVector ABird::Alignment(const TArray<ABird*>& birds)
{
	FVector steer = FVector::ZeroVector;

	for (auto bird : birds)
	{
		steer += bird->ProjectileMovement->Velocity.GetSafeNormal();
	}

	steer.Normalize();

	return (steer * GameManager->GetAlignmentFactor());
}

FVector ABird::Reversal(FVector _Velocity)
{
	return (GameManager->GetAvoidanceFactor() * GameManager->ReversalBehavior(GetActorLocation(), _Velocity, LookAhead, !PredatorDetected));
}

void ABird::Flock(float DeltaTime)
{

	FVector Acceleration = FVector::ZeroVector;
	FVector Velocity = ProjectileMovement->Velocity;

	TArray<ABird*> Birds = FilterBirds();

	if (!Birds.IsEmpty())
	{
		Acceleration += Cohesion(Birds);
		Acceleration += Separation(Birds);
		Acceleration += Alignment(Birds);
	}

	
	FVector tempVel = Velocity + (Acceleration * DeltaTime);
	Acceleration += ObstacleAvoidance(tempVel);

	tempVel = Velocity + (Acceleration * DeltaTime);
	Acceleration += Reversal(tempVel);

	
	Velocity += (Acceleration * DeltaTime);


	Velocity.Normalize();
	Velocity *= MaxVelocity;


	ProjectileMovement->Velocity = Velocity;

}

void ABird::RunAway(float DeltaTime)
{
	if (PredatorsInRange.IsEmpty())
	{
		PredatorDetected = false;
		return;
	}

	FVector Acceleration = FVector::ZeroVector;
	FVector Velocity = ProjectileMovement->Velocity;


	for (auto predator : PredatorsInRange)
	{
		float DistanceSQ = FVector::DistSquared(GetActorLocation(), predator->GetActorLocation());

		if (DistanceSQ > LoseSightRadius * LoseSightRadius)
		{
			PredatorsInRange.Remove(predator);
			if (PredatorsInRange.IsEmpty())
			{
				PredatorDetected = false;
				return;
			}
			continue;
		}

		Acceleration += (GetActorLocation() - predator->GetActorLocation()).GetSafeNormal();
	}

	Acceleration = Acceleration.GetSafeNormal() * 8000;

	FVector tempVel = Velocity + (Acceleration * DeltaTime);
	Acceleration += ObstacleAvoidance(tempVel);

	tempVel = Velocity + (Acceleration * DeltaTime);
	Acceleration += (Reversal(tempVel));


	Velocity += (Acceleration * DeltaTime);


	Velocity.Normalize();
	Velocity *= MaxVelocity;

	//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + Velocity.GetSafeNormal() * 300, FColor::Red, false, 3);

	ProjectileMovement->Velocity = Velocity;
}

FVector ABird::ObstacleAvoidance(FVector _Velocity)
{

	if (!CollisionComp)	return FVector::Zero();

	FVector weight = FVector::ZeroVector;
	FHitResult Hit;
	FVector End = CollisionComp->GetComponentLocation() + _Velocity.GetSafeNormal() * LookAhead;

	UKismetSystemLibrary::BoxTraceSingle(GetWorld(), CollisionComp->GetComponentLocation(), End, CollisionComp->GetScaledBoxExtent(),
		CollisionComp->GetComponentRotation(), UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false, TArray<AActor*>(), EDrawDebugTrace::None, Hit, true);
	if (Hit.bBlockingHit)
	{
		weight += Hit.ImpactNormal;
		if (Hit.ImpactNormal == Hit.GetActor()->GetActorUpVector())
		{
			FVector temp1 = Hit.GetActor()->GetActorRightVector();

			float f = FVector::DotProduct(temp1, (GetActorLocation() - Hit.GetActor()->GetActorLocation()).GetSafeNormal());

			if (f >= 0)
			{
				weight += temp1.GetSafeNormal();
			}
			else
			{
				weight -= temp1.GetSafeNormal();
			}
		}
		else
		{
			FVector temp1 = FVector::CrossProduct(Hit.GetActor()->GetActorUpVector(), Hit.ImpactNormal);

			float f = FVector::DotProduct(_Velocity.GetSafeNormal(), temp1.GetSafeNormal());


			if (f >= 0)
			{
				weight += temp1.GetSafeNormal();
			}
			else
			{
				weight -= temp1.GetSafeNormal();
			}
		}
	}
	
	return (weight * GameManager->GetAvoidanceFactor());
}
