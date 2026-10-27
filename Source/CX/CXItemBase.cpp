// CXItemBase.cpp

#include "CXItemBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "CXPawn.h"

ACXItemBase::ACXItemBase()
{
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	RootComponent = CollisionComp;
	CollisionComp->SetSphereRadius(50.0f);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);

	bReplicates = true;
	NetUpdateFrequency = 1.0f;
	bAlwaysRelevant = true;
}

void ACXItemBase::BeginPlay()
{
	Super::BeginPlay();
}

void ACXItemBase::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (HasAuthority() && !IsActorBeingDestroyed())
	{
		if (ACXPawn* Pawn = Cast<ACXPawn>(OtherActor))
		{
			OnPickedUp(Pawn);
		}
	}
}

void ACXItemBase::OnPickedUp(ACXPawn* Character)
{
	Destroy();
}

void ACXItemBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (EndPlayReason == EEndPlayReason::Destroyed && GetNetMode() != NM_DedicatedServer)
	{
		if (PickupParticle)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PickupParticle, GetActorLocation());
		}
		if (PickupSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), PickupSound, GetActorLocation());
		}
	}
	Super::EndPlay(EndPlayReason);
}
