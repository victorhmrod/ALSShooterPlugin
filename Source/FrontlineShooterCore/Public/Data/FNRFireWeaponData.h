// Wise Labs: Gameworks (c) 2020-2024

#pragma once

#include "CoreMinimal.h"
#include "FSCTypes.h"
#include "GameplayTagContainer.h"
#include "NiagaraSystem.h"
#include "RecoilAnimationComponent.h"
#include "Engine/DataAsset.h"
#include "FNRFireWeaponData.generated.h"

// Data asset class that holds information related to a weapon's behavior and attributes
UCLASS()
class FRONTLINESHOOTERCORE_API UFNRFireWeaponData : public UDataAsset
{
	GENERATED_BODY()

public:
	// Constructor
	UFNRFireWeaponData();
	
	// General weapon data (e.g., weapon name, type)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", Meta = (Tooltip = "General data of the weapon, including its name and type"))
	FGeneralWeaponData General;
	
// Magazine mesh and associated properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", Meta = (Tooltip = "Mesh representing the magazine of the weapon"))
	TSoftObjectPtr<UStaticMesh> MagazineMesh{nullptr};
	
// Projectile-related properties
// Type of projectile (e.g., cartridge)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", Meta = (Tooltip = "Type of projectile (e.g., cartridge)"))
	FGameplayTag ProjectileType{FscProjectileModeTags::Cartridge};

// Minimum and maximum projectile velocity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", Meta = (Tooltip = "Minimum and maximum projectile velocity"))
	FVector2D ProjectileVelocityRange{10000.0f, 50000.0f};

// Gravity applied to the projectile
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", Meta = (Tooltip = "Gravity applied to the projectile"))
	float ProjectileGravity{0.5f};

// Material used for the projectile
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", Meta = (Tooltip = "Material used for the projectile"))
	TSoftObjectPtr<UMaterialInterface> ProjectileMaterial{nullptr};

// Particle system for projectile effects
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Projectile", Meta = (Tooltip = "Particle system for projectile effects"))
	TSoftObjectPtr<UNiagaraSystem> ProjectileParticle{nullptr};

// Ammo-related properties
// Type of ammo compatible with the weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", Meta = (Tooltip = "Type of ammo compatible with the weapon"))
	FGameplayTag AmmoCompatibleType{FscAmmoTypeTags::Light};

// Initial ammo count in the magazine
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", Meta = (Tooltip = "Initial ammo count in the magazine"))
	int32 StartingMagAmmo = 31;

// Maximum ammo that can be held in the magazine
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", Meta = (Tooltip = "Maximum ammo that can be held in the magazine"))
	int32 MaxAmmoInMag = 31;

// Inspection-related properties
// Distance for the inspection camera
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection", Meta = (Tooltip = "Distance for the inspection camera"))
	float InspectionCameraLength = 50.0f;

// Shooting-related properties
// Starting fire mode (e.g., auto, semi-auto)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting", Meta = (Tooltip = "Starting fire mode (e.g., auto, semi-auto)"))
	TEnumAsByte<EFireMode_PRAS> StartFireMode{Auto};

// Possible fire modes (e.g., auto, burst, semi)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting", Meta = (Tooltip = "Possible fire modes (e.g., auto, burst, semi)"))
	FGameplayTagContainer PossibleFireModes;

// Delay between shots in burst mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting", DisplayName = "Delay Between Bullets (Burst mode Only)", Meta = (Tooltip = "Delay between shots in burst mode"))
	float DelayBetweenBullets = 0.1f;

// Fire rate in rounds per minute
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting", Meta = (Tooltip = "Fire rate in rounds per minute"))
	float FireRate = 600;

// Number of bullets fired per shot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting", Meta = (Tooltip = "Number of bullets fired per shot"))
	int BulletsPerShoot = 1;

// Spread range for shots
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting", Meta = (Tooltip = "Spread range for shots"))
	FVector2D SpreadRange = {-2.f, 5.f};

// Damage range for regular shots
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting|Damage", Meta = (Tooltip = "Damage range for regular shots"))
	FVector2D DamageRange = {20.f, 40.f};

// Damage range for headshots
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting|Damage", Meta = (Tooltip = "Damage range for headshots"))
	FVector2D HeadShootDamageRange = {1.f, 1.f};

// Aiming-related properties
// Should the weapon use Aim Down Sights (ADS)?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming", Meta = (Tooltip = "Should the weapon use Aim Down Sights (ADS)?"), DisplayName = "Should Use Ads (Only In First Person)")
	bool bShouldUseAds{false};

// Field of view when aiming
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming", Meta = (Tooltip = "Field of view when aiming"))
	float AimingFOV{80.f};

// Recoil-related properties
// Recoil pitch curve
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil", Meta = (Tooltip = "Recoil pitch curve"))
	UCurveFloat* RecoilPitchCurve{nullptr};

// Recoil yaw curve
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil", Meta = (Tooltip = "Recoil yaw curve"))
	UCurveFloat* RecoilYawCurve{nullptr};

// Velocity of recoil movement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil", Meta = (Tooltip = "Velocity of recoil movement"))
	float RecoilVelocity{0.04f};

// Speed of recoil time curve
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil", Meta = (Tooltip = "Speed of recoil time curve"))
	float RecoilTimeCurveSpeed{0.04f};

// Effects-related properties
// Cascade muzzle flash particle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects", Meta = (Tooltip = "Cascade muzzle flash particle"))
	TSoftObjectPtr<UParticleSystem> MuzzleFlashCascade{nullptr};

// Scale for muzzle flash cascade
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects", Meta = (Tooltip = "Scale for muzzle flash cascade"))
	FVector MuzzleFlashCascadeScale{FVector::One()};

// Niagara muzzle flash particle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects", Meta = (Tooltip = "Niagara muzzle flash particle"))
	TSoftObjectPtr<UNiagaraSystem> MuzzleFlashNiagara{nullptr};

// Scale for muzzle flash Niagara
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects", Meta = (Tooltip = "Scale for muzzle flash Niagara"))
	FVector MuzzleFlashNiagaraScale{FVector::One()};

// Mode of muzzle flash (Cascade or Niagara)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects", Meta = (Tooltip = "Mode of muzzle flash (Cascade or Niagara)"))
	FGameplayTagContainer MuzzleFlashMode{FscMuzzleFlashModeTags::UseCascade};

// Sound-related properties
// Sound for firing the weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds", Meta = (Tooltip = "Sound for firing the weapon"))
	TSoftObjectPtr<USoundBase> FireSound{nullptr};

// Sound for silenced firing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds", Meta = (Tooltip = "Sound for silenced firing"))
	TSoftObjectPtr<USoundBase> SilencedFireSound{nullptr};
	
// Sound attenuation for firing the weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds", Meta = (Tooltip = "Sound attenuation for firing the weapon"))
	TSoftObjectPtr<USoundAttenuation> FireSoundAttenuation{nullptr};

// Sound for empty magazine
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds", Meta = (Tooltip = "Sound for empty magazine"))
	TSoftObjectPtr<USoundBase> EmptyMagSound{nullptr};

// Sound for inserting a magazine
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds", Meta = (Tooltip = "Sound for inserting a magazine"))
	TSoftObjectPtr<USoundBase> MagInSound{nullptr};

// Sound for removing a magazine
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds", Meta = (Tooltip = "Sound for removing a magazine"))
	TSoftObjectPtr<USoundBase> MagOutSound{nullptr};

// Auxiliary magazine sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds", Meta = (Tooltip = "Auxiliary magazine sound"))
	TSoftObjectPtr<USoundBase> MagAuxSound{nullptr};

// Character animations for reload and fire actions
// Reload animation montage for character
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Character", Meta = (Tooltip = "Reload animation montage for character"))
	TSoftObjectPtr<UAnimMontage> CharacterReloadMontage{nullptr};

// Time for character reload (if montage is empty)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Character", DisplayName = "Character Reload Time (Only If Character Reload Montage Is Empty)", Meta = (Tooltip = "Time for character reload (if montage is empty)"))
	float CharacterReloadTime = 2.0f;

// Character fire recoil animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Character", Meta = (Tooltip = "Character fire recoil animation"))
	FRecoilAnimData CharacterFireAnimation;

// Weapon reload animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Weapon", Meta = (Tooltip = "Weapon reload animation"))
	TSoftObjectPtr<UAnimSequence> WeaponReloadAnimation{nullptr};

// Attach points for components like magazine and camera
// Socket for magazine attachment to hand
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachments", Meta = (Tooltip = "Socket for magazine attachment to hand"))
	FName HandMagazineSocket{"MagazineSocket"};

// Socket for magazine attachment to weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachments", Meta = (Tooltip = "Socket for magazine attachment to weapon"))
	FName MagazineSocket{};

// Socket for magazine ejection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachments", Meta = (Tooltip = "Socket for magazine ejection"))
	FName MagazineEjectSocket{"EjectPoint"};

// Socket for aiming camera
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachments", Meta = (Tooltip = "Socket for aiming camera"))
	FName AimingCameraSocket{"ADSCamera"};
	
	// Convert weapon data asset into structured weapon data for use in-game
	static FFireWeaponData AsStructure(const UFNRFireWeaponData* WeaponDataAsset)
	{
		FFireWeaponData WeaponData;
		WeaponData.GeneralWeaponData = WeaponDataAsset->General;
		WeaponData.AmmoMode = WeaponDataAsset->AmmoCompatibleType;
		WeaponData.bShouldAds = WeaponDataAsset->bShouldUseAds;
		WeaponData.PossibleFireModes = WeaponDataAsset->PossibleFireModes;
		WeaponData.DamageRange = WeaponDataAsset->DamageRange;
		WeaponData.FireRate = WeaponDataAsset->FireRate;
		WeaponData.FireSound = WeaponDataAsset->FireSound;
		WeaponData.MagazineSocket = WeaponDataAsset->MagazineSocket;
		WeaponData.ProjectileGravity = WeaponDataAsset->ProjectileGravity;
		WeaponData.ProjectileMaterial = WeaponDataAsset->ProjectileMaterial;
		WeaponData.ProjectileMode = WeaponDataAsset->ProjectileType;
		WeaponData.ProjectileVelocity = WeaponDataAsset->ProjectileVelocityRange;
		WeaponData.RecoilVelocity = WeaponDataAsset->RecoilVelocity;
		WeaponData.ReloadTime = WeaponDataAsset->CharacterReloadTime;
		WeaponData.SpreadRange = WeaponDataAsset->SpreadRange;
		WeaponData.AimingCameraSocket = WeaponDataAsset->AimingCameraSocket;
		WeaponData.BulletsPerShoot = WeaponDataAsset->BulletsPerShoot;
		WeaponData.CharacterFireAnim = WeaponDataAsset->CharacterFireAnimation;
		WeaponData.CharacterReloadMontage = WeaponDataAsset->CharacterReloadMontage;
		WeaponData.DelayBetweenBullets = WeaponDataAsset->DelayBetweenBullets;
		WeaponData.EmptyMagSound = WeaponDataAsset->EmptyMagSound;
		WeaponData.HandMagazineSocket = WeaponDataAsset->HandMagazineSocket;
		WeaponData.HeadShotDamageRange = WeaponDataAsset->HeadShootDamageRange;
		WeaponData.InitialMagAmmo = WeaponDataAsset->StartingMagAmmo;
		WeaponData.MagAuxSound = WeaponDataAsset->MagAuxSound;
		WeaponData.MagazineEjectSocket = WeaponDataAsset->MagazineEjectSocket;
		WeaponData.MagInSound = WeaponDataAsset->MagInSound;
		WeaponData.MagOutSound = WeaponDataAsset->MagOutSound;
		WeaponData.ProjectileNiagaraParticle = WeaponDataAsset->ProjectileParticle;
		WeaponData.RecoilPitchCurve = WeaponDataAsset->RecoilPitchCurve;
		WeaponData.RecoilTimeVelocity = WeaponDataAsset->RecoilTimeCurveSpeed;
		WeaponData.RecoilYawCurve = WeaponDataAsset->RecoilYawCurve;
		WeaponData.SilencedFireSound = WeaponDataAsset->SilencedFireSound;
		WeaponData.StartFireMode = WeaponDataAsset->StartFireMode;
		WeaponData.WeaponMagazineMesh = WeaponDataAsset->MagazineMesh;
		WeaponData.WeaponReloadAnim = WeaponDataAsset->WeaponReloadAnimation;
		WeaponData.AimFOV = WeaponDataAsset->AimingFOV;
		WeaponData.MuzzleFlashMode = WeaponDataAsset->MuzzleFlashMode;
		WeaponData.FireSoundAttenuation = WeaponDataAsset->FireSoundAttenuation;
		WeaponData.MaxAmmoInMag = WeaponDataAsset->MaxAmmoInMag;
		WeaponData.MuzzleFlashCascadeParticle = WeaponDataAsset->MuzzleFlashCascade;
		WeaponData.MuzzleFlashNiagaraParticle = WeaponDataAsset->MuzzleFlashNiagara;
		WeaponData.MuzzleFlashCascadeParticleScale = WeaponDataAsset->MuzzleFlashCascadeScale;
		WeaponData.MuzzleFlashNiagaraParticleScale = WeaponDataAsset->MuzzleFlashNiagaraScale;
		WeaponData.InspectionCameraLength = WeaponDataAsset->InspectionCameraLength;
		
		return WeaponData;
	}
};