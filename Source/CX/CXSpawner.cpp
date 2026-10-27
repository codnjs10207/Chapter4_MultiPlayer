// CXSpawner.cpp

#include "CXSpawner.h"
#include "CXItemBase.h"

ACXSpawner::ACXSpawner()
{
	bReplicates = true;
}

void ACXSpawner::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		SpawnItem();
	}
}

void ACXSpawner::SpawnItem()
{
	if (ItemClassToSpawn)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnedItem = GetWorld()->SpawnActor<ACXItemBase>(ItemClassToSpawn, GetActorLocation(), GetActorRotation(), SpawnParams);

		if (SpawnedItem)
		{
			SpawnedItem->OnDestroyed.AddDynamic(this, &ACXSpawner::HandleItemDestroyed);
		}
	}
}

void ACXSpawner::HandleItemDestroyed(AActor* DestroyedActor)
{
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ACXSpawner::SpawnItem, RespawnDelay, false);
	}
}
