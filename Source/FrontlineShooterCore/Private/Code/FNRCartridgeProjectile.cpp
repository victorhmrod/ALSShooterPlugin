// All rights reserved Wise Labs ®

#include "Code/FNRCartridgeProjectile.h"

#include "Net/UnrealNetwork.h"

AFNRCartridgeProjectile::AFNRCartridgeProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = false;
}

void AFNRCartridgeProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void AFNRCartridgeProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AFNRCartridgeProjectile::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Damage);
}
