//  Wise Labs: Gameworks (c) 2020-2024

#pragma once

#include "CoreMinimal.h"
#include "FSCTypes.h"
#include "GameplayTagContainer.h"
#include "NiagaraSystem.h"
#include "RecoilAnimationComponent.h"
#include "Engine/DataAsset.h"
#include "FNRFireWeaponData.generated.h"

UCLASS()
class FRONTLINESHOOTERCORE_API UFNRFireWeaponData : public UDataAsset
{
	GENERATED_BODY()

public:
	UFNRFireWeaponData();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "FireWeapon")
	FGeneralWeaponData General;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Components")
	TSoftObjectPtr<UStaticMesh> WeaponMagazineMesh{nullptr};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Projectile")
	FGameplayTag ProjectileMode{FscProjectileModeTags::Cartridge};
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Projectile")
	FVector2D ProjectileVelocity{10000.0f, 50000.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Projectile")
	float ProjectileGravity{0.5f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Projectile")
	TSoftObjectPtr<UMaterialInterface> ProjectileMaterial{nullptr};
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly,  Category = "Projectile")
	TSoftObjectPtr<UNiagaraSystem> ProjectileNiagaraParticle{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Ammo")
	FGameplayTag AmmoMode{FscAmmoTypeTags::Light};
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Ammo")
	int32 InitialMagAmmo = 31;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Ammo")
	int32 MaxAmmoInMag = 31;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Ammo")
	float ReloadTime = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection")
	float InspectionCameraLength = 50.0f;	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Shot")
	TEnumAsByte<EFireMode_PRAS> StartFireMode{Auto};
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Shot")
	FGameplayTagContainer PossibleFireModes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Shot", DisplayName = "Delay Between Bullets (Burst mode Only)")
	float DelayBetweenBullets = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Shot")
	float FireRate = 600;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Shot")
    int BulletsPerShoot = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Shot")
	FVector2D SpreadRange = {-2.f, 5.f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Shot|Damage")
	FVector2D DamageRange = {20.f, 40.f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Shot|Damage")
	FVector2D HeadShotDamageRange = {1.f, 1.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Aiming")
	bool bShouldUseAds{false};
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Aiming")
	float AimingFOV{80.f};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Recoil", DisplayName = "Recoil Pitch Curve")
	UCurveFloat* RecoilPitchCurve{nullptr};
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Recoil", DisplayName = "Recoil Yaw Curve")
	UCurveFloat* RecoilYawCurve{nullptr};
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Recoil", DisplayName = "Recoil Velocity")
	float RecoilVelocity{0.04f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil", DisplayName = "Recoil Time Velocity")
	float RecoilTimeVelocity{0.04f};
	
	// Effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "VFX")
	TSoftObjectPtr<UParticleSystem> MuzzleFlashCascadeParticle{nullptr};
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "VFX")
	FVector MuzzleFlashCascadeParticleScale{FVector::One()};
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "VFX")
	TSoftObjectPtr<UNiagaraSystem> MuzzleFlashNiagaraParticle{nullptr};
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "VFX")
	FVector MuzzleFlashNiagaraParticleScale{FVector::One()};
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "VFX")
	FGameplayTagContainer MuzzleFlashMode {FscMuzzleFlashModeTags::UseCascade};

	// Sounds
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "SFX")
	TSoftObjectPtr<USoundBase> FireSound{nullptr};
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "SFX")
	TSoftObjectPtr<USoundBase> SilencedFireSound{nullptr};
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "SFX")
	TSoftObjectPtr<USoundBase> EmptyMagSound{nullptr};
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "SFX")
	TSoftObjectPtr<USoundBase> MagInSound{nullptr};
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "SFX")
	TSoftObjectPtr<USoundBase> MagOutSound{nullptr};
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "SFX")
	TSoftObjectPtr<USoundBase> MagAuxSound{nullptr};
	
	// Character Animations
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Animations")
	TSoftObjectPtr<UAnimMontage> CharacterReloadMontage{nullptr};
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Animations")
	FGameplayTagContainer FireAnimationMode{FscFireAnimationModeTags::UseProceduralFireAnimation};
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Animations")
	FRecoilAnimData CharacterFireAnim;
	
	// Weapon Animations
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Animations")
	TSoftObjectPtr<UAnimSequence> WeaponReloadAnim{nullptr};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Attach")
	FName HandMagazineSocket{"MagazineSocket"};
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Attach")
	FName MagazineSocket{};
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Attach")
	FName MagazineEjectSocket{"EjectPoint"};
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Attach")
	FName AimingCameraSocket{"CameraOffsetSocket"};
	
	static FFireWeaponData AsStructure(const UFNRFireWeaponData* WeaponDataAsset)
	{
		FFireWeaponData WeaponData;
		WeaponData.GeneralWeaponData = WeaponDataAsset->General;
		WeaponData.AmmoMode = WeaponDataAsset->AmmoMode;
		WeaponData.bShouldAds = WeaponDataAsset->bShouldUseAds;
		WeaponData.PossibleFireModes = WeaponDataAsset->PossibleFireModes;
		WeaponData.DamageRange = WeaponDataAsset->DamageRange;
		WeaponData.FireRate = WeaponDataAsset->FireRate;
		WeaponData.FireSound = WeaponDataAsset->FireSound;
		WeaponData.MagazineSocket = WeaponDataAsset->MagazineSocket;
		WeaponData.ProjectileGravity = WeaponDataAsset->ProjectileGravity;
		WeaponData.ProjectileMaterial = WeaponDataAsset->ProjectileMaterial;
		WeaponData.ProjectileMode = WeaponDataAsset->ProjectileMode;
		WeaponData.ProjectileVelocity = WeaponDataAsset->ProjectileVelocity;
		WeaponData.RecoilVelocity = WeaponDataAsset->RecoilVelocity;
		WeaponData.ReloadTime = WeaponDataAsset->ReloadTime;
		WeaponData.SpreadRange = WeaponDataAsset->SpreadRange;
		WeaponData.AimingCameraSocket = WeaponDataAsset->AimingCameraSocket;
		WeaponData.BulletsPerShoot = WeaponDataAsset->BulletsPerShoot;
		WeaponData.CharacterFireAnim = WeaponDataAsset->CharacterFireAnim;
		WeaponData.CharacterReloadMontage = WeaponDataAsset->CharacterReloadMontage;
		WeaponData.DelayBetweenBullets = WeaponDataAsset->DelayBetweenBullets;
		WeaponData.EmptyMagSound = WeaponDataAsset->EmptyMagSound;
		WeaponData.HandMagazineSocket = WeaponDataAsset->HandMagazineSocket;
		WeaponData.HeadShotDamageRange = WeaponDataAsset->HeadShotDamageRange;
		WeaponData.InitialMagAmmo = WeaponDataAsset->InitialMagAmmo;
		WeaponData.MagAuxSound = WeaponDataAsset->MagAuxSound;
		WeaponData.MagazineEjectSocket = WeaponDataAsset->MagazineEjectSocket;
		WeaponData.MagInSound = WeaponDataAsset->MagInSound;
		WeaponData.MagOutSound = WeaponDataAsset->MagOutSound;
		WeaponData.ProjectileNiagaraParticle = WeaponDataAsset->ProjectileNiagaraParticle;
		WeaponData.RecoilPitchCurve = WeaponDataAsset->RecoilPitchCurve;
		WeaponData.RecoilTimeVelocity = WeaponDataAsset->RecoilTimeVelocity;
		WeaponData.RecoilYawCurve = WeaponDataAsset->RecoilYawCurve;
		WeaponData.SilencedFireSound = WeaponDataAsset->SilencedFireSound;
		WeaponData.StartFireMode = WeaponDataAsset->StartFireMode;
		WeaponData.WeaponMagazineMesh = WeaponDataAsset->WeaponMagazineMesh;
		WeaponData.WeaponReloadAnim = WeaponDataAsset->WeaponReloadAnim;
		WeaponData.AimFOV = WeaponDataAsset->AimingFOV;
		WeaponData.MuzzleFlashMode = WeaponDataAsset->MuzzleFlashMode;
		WeaponData.MaxAmmoInMag = WeaponDataAsset->MaxAmmoInMag;
		WeaponData.MuzzleFlashCascadeParticle = WeaponDataAsset->MuzzleFlashCascadeParticle;
		WeaponData.MuzzleFlashNiagaraParticle = WeaponDataAsset->MuzzleFlashNiagaraParticle;
		WeaponData.MuzzleFlashCascadeParticleScale = WeaponDataAsset->MuzzleFlashCascadeParticleScale;
		WeaponData.MuzzleFlashNiagaraParticleScale = WeaponDataAsset->MuzzleFlashNiagaraParticleScale;
		WeaponData.FireAnimationMode = WeaponDataAsset->FireAnimationMode;
		WeaponData.InspectionCameraLength = WeaponDataAsset->InspectionCameraLength;
		
		return WeaponData;
	}
};