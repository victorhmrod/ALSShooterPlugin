//  Wise Labs: Gameworks (c) 2020-2024

#include "Code/FNRRocketProjectile.h"

#include "Net/UnrealNetwork.h"

AFNRRocketProjectile::AFNRRocketProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = false;
}

void AFNRRocketProjectile::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
