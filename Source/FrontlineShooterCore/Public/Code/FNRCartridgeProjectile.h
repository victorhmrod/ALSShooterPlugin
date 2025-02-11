// All rights reserved Wise Labs ®

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FNRCartridgeProjectile.generated.h"

UCLASS()
class FRONTLINESHOOTERCORE_API AFNRCartridgeProjectile : public AActor
{
	GENERATED_BODY()

public:
	AFNRCartridgeProjectile();

	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaSeconds) override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(BlueprintReadOnly, Category="Damageable", Replicated)
	float Damage{0.f};

	// This is the speed of the projectile at spawn.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Velocity", meta=(ExposeOnSpawn = true))
	float MuzzleVelocity{30000};

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Meshes")
	UMaterialInterface* ProjectileMaterial{nullptr};

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Meshes")
	class UNiagaraSystem* ShrapnelFlameFX{nullptr};
};
