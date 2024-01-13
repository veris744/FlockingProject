// Fill out your copyright notice in the Description page of Project Settings.


#include "Teleport.h"
#include <Components/BoxComponent.h>
#include <RebelWolves/RebelWolvesCharacter.h>
#include "GameManager.h"
#include "Components/CapsuleComponent.h"

// Sets default values
ATeleport::ATeleport()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;


	CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComp"));
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	SetRootComponent(CollisionComp);
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ATeleport::OnOverlapBegin);

}

void ATeleport::OnOverlapBegin(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->GetClass()->IsChildOf(ARebelWolvesCharacter::StaticClass()))
	{
		UGameManager* Manager = UGameManager::GetGameManager();

		if (Manager)
		{
			ARebelWolvesCharacter* Player = Cast<ARebelWolvesCharacter>(OtherActor);
			Player->SetActorLocation(Manager->GetCharacterStart() + GetActorUpVector() * Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
			
		}

	}
}



// Called when the game starts or when spawned
void ATeleport::BeginPlay()
{
	Super::BeginPlay();
	

	UGameManager* Manager = UGameManager::GetGameManager();

	if (Manager)
	{
		CollisionComp->SetBoxExtent(FVector(Manager->Size.X + 100, Manager->Size.X + 100, 50));
		SetActorLocation(FVector(0, 0, -200));
	}
}
