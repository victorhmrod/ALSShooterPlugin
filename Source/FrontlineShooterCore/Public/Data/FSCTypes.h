// All rights reserved Wise Labs �

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NiagaraSystem.h"
#include "NativeGameplayTags.h"
#include "RecoilAnimationComponent.h"
#include "FSCTypes.generated.h"

namespace FscWeaponStateTags
{
	FRONTLINESHOOTERCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(CanFire)

	FRONTLINESHOOTERCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InAds)
}

namespace FscMuzzleFlashModeTags
{
	FRONTLINESHOOTERCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UseNiagara)

	FRONTLINESHOOTERCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UseCascade)
}

namespace FscWeaponTypeTags
{
	FRONTLINESHOOTERCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(AssaultRifle)

	FRONTLINESHOOTERCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sword)
	
	FRONTLINESHOOTERCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(PrecisionRifle)

	FRONTLINESHOOTERCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pistol)

	FRONTLINESHOOTERCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SMG)

	FRONTLINESHOOTERCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shotgun)
	
	FRONTLINESHOOTERCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Grenade)
}

namespace FscRarityType
{
	FRONTLINESHOOTERCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Commom)
	
	FRONTLINESHOOTERCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Uncommom)

	FRONTLINESHOOTERCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Rare)

	FRONTLINESHOOTERCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UltraRare)

	FRONTLINESHOOTERCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Divine)

	FRONTLINESHOOTERCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Mythic)
}

namespace FscFireModeTags
{
	FRONTLINESHOOTERCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(CanSingle)

	FRONTLINESHOOTERCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(CanAuto)

	FRONTLINESHOOTERCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(CanBurst)
}

namespace FscAmmoTypeTags
{
	FRONTLINESHOOTERCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Light)
	
	FRONTLINESHOOTERCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Heavy)

	FRONTLINESHOOTERCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(ShotgunAmmo)

	FRONTLINESHOOTERCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Precision)
	
	FRONTLINESHOOTERCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Energy)
}

namespace FscProjectileModeTags
{
	FRONTLINESHOOTERCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cartridge)

	FRONTLINESHOOTERCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Rocket)
}

USTRUCT(BlueprintType)
struct FRarity
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLinearColor Common{1, 1, 1, 1};
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLinearColor Uncommon{0.132868, 1, 0, 1};
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLinearColor Rare{0, 0.333334, 1, 1};
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLinearColor UltraRare{0.25, 0, 1, 1};
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLinearColor Divine{1, 0.15, 0, 1};
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLinearColor Mythic{1, 0, 0.15, 1};
};

UENUM(BlueprintType)
enum class EAttachmentType : uint8
{
	Aim         UMETA(DisplayName="Aim"),
	Ammo        UMETA(DisplayName="Ammo"),
	Barrel      UMETA(DisplayName="Barrel"),
	Suppressor      UMETA(DisplayName="Suppressor"),
	Stock		UMETA(DisplayName="Stock"),
	Bolt		UMETA(DisplayName="Bolt"),
	Other       UMETA(DisplayName="Other")
};

UENUM(BlueprintType)
enum class EGrenadeType : uint8
{
	Fragmentation,
	Termite,
	Flash
};

UENUM(BlueprintType)
enum class EGrenadeFilter : uint8
{
	Fragmentation,
	Termite,
	Flash,
	All
};

UENUM(BlueprintType)
enum class EAttackType : uint8
{
	Light,
	Strong,
};

USTRUCT(BlueprintType, Blueprintable)
struct FGeneralWeaponData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
	
	// ----------- General Weapon Information -----------
	// Name of the weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Info", Meta = (Tooltip = "Name of the weapon"))
	FName WeaponName;
	
	// Icon for the weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Info", Meta = (Tooltip = "Icon for the weapon"))
	FSlateBrush WeaponIcon;
	
	// Mode of the weapon (e.g., Assault Rifle, Sniper)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Info", Meta = (Tooltip = "Mode of the weapon (e.g., Assault Rifle, Sniper)"))
	FGameplayTag WeaponMode{FscWeaponTypeTags::AssaultRifle};
	
	// Rarity of the weapon (e.g., Common, Rare)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Info", Meta = (Tooltip = "Rarity of the weapon (e.g., Common, Rare)"))
	FGameplayTag Rarity{FscRarityType::Commom};
	
	// Should the weapon have sway effects when moving?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Info", Meta = (Tooltip = "Should the weapon have sway effects when moving?"))
	bool bShouldUseSway{true};
	
	// Should the weapon have a reticle when aiming?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Info", Meta = (Tooltip = "Should the weapon have a reticle when aiming?"))
	bool bShouldUseReticle{true};
	
	// ----------- Weapon Meshes & Animation -----------
	// Skeletal mesh for the weapon body
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Meshes & Animations", Meta = (Tooltip = "Skeletal mesh for the weapon body"))
	TSoftObjectPtr<USkeletalMesh> WeaponBodyMesh{nullptr};
	
	// Animation class for the weapon's behavior
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Meshes & Animations", Meta = (Tooltip = "Animation class for the weapon's behavior"))
	TSoftClassPtr<UAnimInstance> WeaponAnimInstance{nullptr};

	// ----------- Character Animations -----------
	// Tag for the overlay mode for character animations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Animations", Meta = (Tooltip = "Tag for the overlay mode for character animations"))
	FGameplayTag OverlayMode = FGameplayTag::EmptyTag;
	
	// Character equip animation montage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Animations", Meta = (Tooltip = "Character equip animation montage"))
	TSoftObjectPtr<UAnimMontage> CharacterEquipMontage{nullptr};
	
	// Character unequip animation montage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Animations", Meta = (Tooltip = "Character unequip animation montage"))
	TSoftObjectPtr<UAnimMontage> CharacterUnEquipMontage{nullptr};
	
	// Character fire animation montage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Animations", Meta = (Tooltip = "Character fire animation montage"))
	TSoftObjectPtr<UAnimMontage> CharacterFireMontage{nullptr};

	// ----------- Weapon Animations -----------
	// Weapon equip animation sequence
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Animations", Meta = (Tooltip = "Weapon equip animation sequence"))
	TSoftObjectPtr<UAnimSequence> WeaponEquipAnimation{nullptr};
	
	// Weapon unequip animation sequence
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Animations", Meta = (Tooltip = "Weapon unequip animation sequence"))
	TSoftObjectPtr<UAnimSequence> WeaponUnEquipAnimation{nullptr};
	
	// Weapon fire animation sequence
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Animations", Meta = (Tooltip = "Weapon fire animation sequence"))
	TSoftObjectPtr<UAnimSequence> WeaponFireAnimations{nullptr};
	
	// ----------- Attack/Fire Behavior -----------
	// Socket for attaching the weapon to the hand
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack/Fire Behavior", Meta = (Tooltip = "Socket for attaching the weapon to the hand"))
	FName HandSocket{};
	
	// Rotation fix for the hand when using the weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack/Fire Behavior", Meta = (Tooltip = "Rotation fix for the hand when using the weapon"))
	FRotator HandRotatorFix{FRotator::ZeroRotator};
	
	// Array of socket names for holstering the weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack/Fire Behavior", Meta = (Tooltip = "Array of socket names for holstering the weapon"))
	TArray<FName> HolsterSocket{};
	
	// Rotation fix for holstering the weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack/Fire Behavior", Meta = (Tooltip = "Rotation fix for holstering the weapon"))
	FRotator HolsterRotatorFix{FRotator::ZeroRotator};
	
	// Socket for the muzzle of the weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack/Fire Behavior", Meta = (Tooltip = "Socket for the muzzle of the weapon"))
	FName MuzzleSocket{};

	// ----------- Fade Effects -----------
	// Should the weapon fade in/out when equipped/holstered?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fade Effects", Meta = (Tooltip = "Should the weapon fade in/out when equipped/holstered?"))
	bool bShouldFade{true};
	
	// Speed at which the weapon fades when holstered
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fade Effects", Meta = (Tooltip = "Speed at which the weapon fades when holstered"))
	float HolsterFadeSpeed{1.0f};
	
	// Speed at which the weapon fades when equipped
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fade Effects", Meta = (Tooltip = "Speed at which the weapon fades when equipped"))
	float EquipFadeSpeed{1.0f};

	// ----------- Sound Effects -----------
	// Sound played when the weapon is equipped
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Effects", Meta = (Tooltip = "Sound played when the weapon is equipped"))
	TSoftObjectPtr<USoundBase> EquipSound{nullptr};
	
	// Sound played when the weapon is holstered
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Effects", Meta = (Tooltip = "Sound played when the weapon is holstered"))
	TSoftObjectPtr<USoundBase> HolsterSound{nullptr};
};

USTRUCT(BlueprintType, Blueprintable)
struct FFireWeaponData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General")
	FGeneralWeaponData GeneralWeaponData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TSoftObjectPtr<UStaticMesh> WeaponMagazineMesh{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Projectile")
	FGameplayTag ProjectileMode{FscProjectileModeTags::Cartridge};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Projectile")
	FVector2D ProjectileVelocity{1.f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Projectile")
	float ProjectileGravity{0.5f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Projectile")
	TSoftObjectPtr<UMaterialInterface> ProjectileMaterial{nullptr};
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Behavior|Projectile")
	TSoftObjectPtr<UNiagaraSystem> ProjectileNiagaraParticle{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Ammo")
	FGameplayTag AmmoMode{FscAmmoTypeTags::Light};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Ammo")
	int32 InitialMagAmmo = 31;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Ammo")
	int32 MaxAmmoInMag = 31;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Ammo")
	float ReloadTime = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Ammo")
	float InspectionCameraLength = 50.0f;	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Fire")
	TEnumAsByte<EFireMode_PRAS> StartFireMode{Auto};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Fire")
	FGameplayTagContainer PossibleFireModes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Fire", DisplayName = "Delay Between Bullets (Burst mode Only)")
	float DelayBetweenBullets = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Fire")
	float FireRate = 600;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Fire")
    int BulletsPerShoot = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Fire")
	FVector2D SpreadRange = {-2.f, 5.f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Damage")
	FVector2D DamageRange = {20.f, 40.f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Damage")
	FVector2D HeadShotDamageRange = {1.f, 1.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Aiming")
	bool bShouldAds{false};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Aiming")
	float AimFOV{80.f};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Recoil", DisplayName = "Recoil Pitch Curve")
	UCurveFloat* RecoilPitchCurve{nullptr};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Recoil", DisplayName = "Recoil Yaw Curve")
	UCurveFloat* RecoilYawCurve{nullptr};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Recoil", DisplayName = "Recoil Velocity")
	float RecoilVelocity{0.04f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Recoil", DisplayName = "Recoil Velocity")
	float RecoilTimeVelocity{0.04f};
	
	// Effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TSoftObjectPtr<UParticleSystem> MuzzleFlashCascadeParticle{nullptr};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FVector MuzzleFlashCascadeParticleScale{FVector::One()};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TSoftObjectPtr<UNiagaraSystem> MuzzleFlashNiagaraParticle{nullptr};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FVector MuzzleFlashNiagaraParticleScale{FVector::One()};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FGameplayTagContainer MuzzleFlashMode {FscMuzzleFlashModeTags::UseCascade};

	// Sounds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	TSoftObjectPtr<USoundBase> FireSound{nullptr};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	TSoftObjectPtr<USoundBase> SilencedFireSound{nullptr};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds", Meta = (Tooltip = "Sound attenuation for firing the weapon"))
	TSoftObjectPtr<USoundAttenuation> FireSoundAttenuation{nullptr};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	TSoftObjectPtr<USoundBase> EmptyMagSound{nullptr};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	TSoftObjectPtr<USoundBase> MagInSound{nullptr};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	TSoftObjectPtr<USoundBase> MagOutSound{nullptr};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	TSoftObjectPtr<USoundBase> MagAuxSound{nullptr};

	// Character Animations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	TSoftObjectPtr<UAnimMontage> CharacterReloadMontage{nullptr};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	FRecoilAnimData CharacterFireAnim;
	
	// Weapon Animations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	TSoftObjectPtr<UAnimSequence> WeaponReloadAnim{nullptr};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	FName HandMagazineSocket{"MagazineSocket"};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	FName MagazineSocket{};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	FName MagazineEjectSocket{"EjectPoint"};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	FName AimingCameraSocket{"CameraOffsetSocket"};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow|Behavior")
	float MaxVelocitySpeed = 2500.0f;
};