// All rights reserved Wise Labs �

#pragma once

#include "CoreMinimal.h"
#include "FNRWeapon.h"
#include "Data/FNRGrenadeWeaponData.h"
#include "Data/FSCTypes.h"
#include "FNRGrenade.generated.h"

class UExplosionComponent;

UCLASS()
class FRONTLINESHOOTERCORE_API AFNRGrenade : public AFNRWeapon
{
	GENERATED_BODY()

#pragma region Components
public:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TObjectPtr<class UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TObjectPtr<UExplosionComponent> ExplosionComponent;
	
#pragma endregion Components
	
#pragma region Variables
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	UFNRGrenadeWeaponData* GrenadeData{nullptr};

#pragma endregion Variables

#pragma region Functions
	// Sets default values for this actor's properties
	AFNRGrenade();
	
	virtual void OnInteract(class UInteractorComponent* Interactor, UInteractableComponent* Interactable) override;

public:
	virtual bool Fire(const bool bFire) override;

	UFUNCTION(Server, Reliable)
	void Fire_Server();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual FGeneralWeaponData GetGeneralData() const override;
	
	virtual bool HasAmmo() override;
#pragma endregion Functions
	
};
