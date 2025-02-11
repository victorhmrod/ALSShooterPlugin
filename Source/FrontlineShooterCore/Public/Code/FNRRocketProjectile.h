//  Wise Labs: Gameworks (c) 2020-2024

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FNRRocketProjectile.generated.h"

UCLASS()
class FRONTLINESHOOTERCORE_API AFNRRocketProjectile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFNRRocketProjectile();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Damageable", meta=(ExposeOnSpawn = true))
	float Damage{0.f};
};
