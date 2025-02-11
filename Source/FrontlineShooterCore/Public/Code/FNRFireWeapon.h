// All rights reserved Wise Labs ®

#pragma once

#include "CoreMinimal.h"
#include "FNRWeapon.h"
#include "GameFramework/Actor.h"
#include "FNRFireWeapon.generated.h"

class IFNRWeaponComponentInterface;
class UFNRInventoryItem;

USTRUCT(BlueprintType)
struct FBulletSpawnData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FTransform ProjectileTransform;

	FBulletSpawnData()
		: ProjectileTransform(FTransform::Identity)
	{}

	FBulletSpawnData(FTransform const& InProjectileTransform)
		: ProjectileTransform(InProjectileTransform)
	{}
};

UCLASS()
class FRONTLINESHOOTERCORE_API AFNRFireWeapon : public AFNRWeapon
{
	GENERATED_BODY()
// lilith me deu
#pragma region Components
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> MagazineComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UFNRAttachmentWeaponComponent> AttachmentComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class USceneComponent> IronsightLocationComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class URecoilAnimationComponent> RecoilAnimationComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class URotatingMovementComponent> RotatingMovementComponent;
	
#pragma endregion Components
	
#pragma region Variables
protected:
	UPROPERTY(EditAnywhere, Category = "Info")
	UFNRFireWeaponData* WeaponDataAsset;

public:
	UPROPERTY(BlueprintReadWrite, Category = "Info", DisplayName="WeaponData")
	FFireWeaponData Internal_FireWeaponData;
	
	UPROPERTY(BlueprintReadOnly, Category = "Info")
	bool bIsReloading;

	UPROPERTY(BlueprintReadOnly, Category = "Info")
	int BulletsInMag;

	UPROPERTY(EditDefaultsOnly, Category = "Info")
	TSoftClassPtr<class AFNRCartridgeProjectile> CartridgeProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Info")
	TSoftClassPtr<class AFNRRocketProjectile> RocketProjectileClass;

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<class UFNRInventoryItem> AmmoType{};

	UPROPERTY()	
	TSubclassOf<AFNRCartridgeProjectile> LoadedCartridgeProjectileClass;

	UPROPERTY()	
	TObjectPtr<UMaterialInterface> LoadedProjectileMaterial;

	UPROPERTY()	
	TObjectPtr<UNiagaraSystem> LoadedProjectileNiagaraParticle;

	UPROPERTY()	
	TSubclassOf<class AFNRRocketProjectile> LoadedRocketProjectileClass;

	UPROPERTY(Replicated, ReplicatedUsing="OnRep_CartridgeSpawnData")
	FBulletSpawnData CartridgeSpawnData;

	UFUNCTION()
	void OnRep_CartridgeSpawnData() const;

	UPROPERTY(Replicated, ReplicatedUsing="OnRep_RocketSpawnData")
	FBulletSpawnData RocketSpawnData;

	UFUNCTION()
	void OnRep_RocketSpawnData() const;
	
protected:
	bool bWantsFire;

private:
	float LastTimeCurve = 0.0f;

	int BulletsRemaining = -1;

public:
	UPROPERTY()
	TEnumAsByte<EFireMode_PRAS> CurrentFireMode{};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Essential", meta=(ExposeOnSpawn="true"))
	bool bEnableRotation;

	UPROPERTY(Replicated)
	FTransform RecoilOffset;
	
	#pragma region Loaded
	UPROPERTY()
	USoundBase* LoadedFireSound;
	
	UPROPERTY()
	UAnimMontage* LoadedReloadCharacterMontage;
	
	UPROPERTY()
	USoundBase* LoadedEmptyMagSound;
	
	UPROPERTY()
	USoundBase* LoadedSilencedSound;

	UPROPERTY()
	UParticleSystem* LoadedCascadedParticle;

	UPROPERTY()
	UNiagaraSystem* LoadedNiagaraParticle;

	UPROPERTY()
	UStaticMesh* LoadedMagazineMesh;

	UPROPERTY()
	UAnimSequence* LoadedWeaponReloadAnim;

	#pragma endregion Loaded
	
#pragma endregion Variables
	
#pragma region Functions
	
	#pragma region Unreal Defaults
public:	
	// Sets default values for this actor's properties
	AFNRFireWeapon();

	virtual void Tick(float DeltaSeconds) override;

	virtual TArray<class UMeshComponent*> GetGlowableMeshes() const override;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PreInitializeComponents() override;

	virtual void ReadValues() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	#pragma endregion Unreal Defaults

	#pragma region CPP Only
protected:
	FTimerHandle ResetRecoilTimerHandle;
	void ApplyRecoil();
	
	FHitResult FireTrace();
	
	void FireProjectile(const FTransform& ProjectileTransform);

	FRotator GetSpread(const FRotator& CurrentRotation, const float& Precision = 100.0f) const;

private:
	FRotator CurrentTargetRecoil;

	virtual void RefreshCPPOnly() override;

	bool SemiFire();
	
	bool AutoFire();

	bool BurstFire();

public:
	void SetFireMode(const EFireMode_PRAS& NewFireMode);
	bool bIsRecoiling = false;
	
	#pragma endregion CPP Only

    #pragma region Blueprint Exposed
public:
	virtual FGeneralWeaponData GetGeneralData() const override
	{
		return Internal_FireWeaponData.GeneralWeaponData;
	}
	
	FTimerHandle InFireRateTimerHandle, FireWithDelayTimerHandle;
	virtual bool Fire(const bool bFire) override;

	virtual bool AIFire(const bool bFire = true, const FVector TargetLocation = FVector(0.0f, 0.0f, 0.0f), const float Precision = 60.0f) override;
	
	float GetFireRate() const;

	UFUNCTION(Server, Reliable)
	void Server_SetIsFiring(const bool bFiring);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetIsFiring(const bool bFiring);
	
	virtual bool HasAmmo() override;

	UFUNCTION(BlueprintImplementableEvent, Category="Essential", DisplayName="Fire")
	void K2_Fire();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Essential", meta=(ExposeOnSpawn=true), Replicated)
	bool SimulatePhysics{true};
	
	FTimerHandle ReloadTimerHandle;
	void Reload();

	void ReloadByAnim(const int& Quantity);
	
	virtual void SetAimingStatus(bool bStatus) override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Essential")
	EFireMode_PRAS GetCurrentFireMode() const;

	#pragma endregion Blueprint Exposed

	#pragma region Replicated Functions
private:
	UFUNCTION(Server, Reliable)
	void Server_SpawnCartridge(const FTransform& ProjectileTransform);

	UFUNCTION(Server,Reliable)
	void Server_SpawnRocket(const FTransform& ProjectileTransform);

	UFUNCTION()
	void SpawnCartridgeProjectile(const FTransform& ProjectileTransform) const;

	UFUNCTION()
	void SpawnRocketProjectile(const FTransform& ProjectileTransform) const;

private:
	
	void EndReload();
	
	void EndReloadByAnim(const int Quantity);
	
	#pragma endregion Replicated Functions


protected:
	void RefreshRecoilOffset();
	
#pragma endregion Functions

};
