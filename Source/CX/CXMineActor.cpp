// CXMineActor.cpp

#include "CXMineActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "CXPawn.h"

ACXMineActor::ACXMineActor()
{
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	RootComponent = CollisionComp;
	CollisionComp->SetSphereRadius(80.0f);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);

	bReplicates = true;
	NetUpdateFrequency = 10.0f;
	bAlwaysRelevant = true;
}

void ACXMineActor::BeginPlay()
{
	Super::BeginPlay();
}

void ACXMineActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (HasAuthority())
	{
		// Don't trigger by the owner immediately upon placement
		if (OtherActor && OtherActor != GetOwner())
		{
			if (ACXPawn* VictimPawn = Cast<ACXPawn>(OtherActor))
			{
				// Disable throttle for 2 seconds on the victim
				VictimPawn->ApplyThrottleMalfunction(2.0f);

				Destroy();
			}
		}
	}
}

void ACXMineActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (EndPlayReason == EEndPlayReason::Destroyed && GetNetMode() != NM_DedicatedServer)
	{
		if (ExplosionParticle)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionParticle, GetActorLocation());
		}
		if (ExplosionSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation());
		}
	}
	Super::EndPlay(EndPlayReason);
}