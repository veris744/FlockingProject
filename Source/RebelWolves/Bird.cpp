// Fill out your copyright notice in the Description page of Project Settings.


#include "Bird.h"
#include <Components/SphereComponent.h>
#include <Components/BoxComponent.h>
#include "RebelWolves/RebelWolvesGameMode.h"
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

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = RootComponent;
	ProjectileMovement->bRotationFollowsVelocity = true;
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
	
	if (OtherActor->GetClass()->IsChildOf(ARebelWolvesProjectile::StaticClass()) &&
		FVector::DotProduct(GetActorForwardVector(), (OtherActor->GetActorLocation() - GetActorLocation()).GetSafeNormal()) <= FOV)
	{
		PredatorDetected = true;
		predator = Cast<ARebelWolvesProjectile>(OtherActor);
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
	//steer -= Velocity.GetSafeNormal();

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
	//steer -= Velocity.GetSafeNormal();


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
	//steer -= Velocity.GetSafeNormal();

	return (steer * GameManager->GetAlignmentFactor());
}

FVector ABird::Reversal(FVector _Velocity)
{
	FVector LookAheadPos = GetActorLocation() + _Velocity.GetSafeNormal() * LookAhead;

	float xBoundary = GameManager->Size.X / 2;
	float yBoundary = GameManager->Size.Y / 2;
	float zBoundary = GameManager->MaxHeight;

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

	return (weight * 1000);
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

	

	if (Acceleration.SquaredLength() > MaxAcceleration * MaxAcceleration)
	{
		Acceleration.Normalize();
		Acceleration *= MaxAcceleration;
	}

	Velocity += (Acceleration * DeltaTime);

	if (Velocity.IsZero())
		Velocity = -GetActorForwardVector();

	Velocity.Normalize();
	Velocity *= MaxVelocity;

	

	ProjectileMovement->Velocity = Velocity;

}

void ABird::RunAway(float DeltaTime)
{
	FVector Acceleration = FVector::ZeroVector;
	FVector Velocity = ProjectileMovement->Velocity;

	if (!predator)
	{
		return;
	}

	Acceleration = -(predator->GetActorLocation() - GetActorLocation()).GetSafeNormal() * 200;

	FVector tempVel = Velocity + (Acceleration * DeltaTime);
	Acceleration += ObstacleAvoidance(tempVel);

	tempVel = Velocity + (Acceleration * DeltaTime);
	Acceleration += Reversal(tempVel);


	if (Acceleration.SquaredLength() > MaxAcceleration * MaxAcceleration)
	{
		Acceleration.Normalize();
		Acceleration *= MaxAcceleration;
	}

	Velocity += (Acceleration * DeltaTime);

	if (Velocity.IsZero())
		Velocity = -GetActorForwardVector();

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
	//GetWorld()->LineTraceSingleByChannel(Hit, GetActorLocation(), End, ECC_Visibility, FCollisionQueryParams());
	UKismetSystemLibrary::BoxTraceSingle(GetWorld(), CollisionComp->GetComponentLocation(), End, CollisionComp->GetScaledBoxExtent() / 2,
		CollisionComp->GetComponentRotation(), UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false, TArray<AActor*>(), EDrawDebugTrace::None, Hit, true);
	if (Hit.bBlockingHit)
	{
		//FVector Avoid = End - Hit.GetActor()->GetActorLocation();
		//weight += Avoid.GetSafeNormal2D();
		/*if (FVector::DotProduct(_Velocity.GetSafeNormal2D(), (Hit.GetActor()->GetActorLocation() - GetActorLocation()).GetSafeNormal2D()) > 0)
		{
			weight -= GetActorLocation();
		}
		else
		{
			weight -= GetActorLocation();
		}*/

		//weight += Hit.ImpactNormal;
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