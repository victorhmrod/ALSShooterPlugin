//  Wise Labs: Gameworks (c) 2020-2024

#pragma once

#include "CoreMinimal.h"
#include "FSCTypes.h"
#include "NiagaraSystem.h"
#include "Engine/DataAsset.h"
#include "FNRMeleeWeaponData.generated.h"

UCLASS()
class FRONTLINESHOOTERCORE_API UFNRMeleeWeaponData : public UDataAsset
{
	GENERATED_BODY()

public:
	UFNRMeleeWeaponData();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeleeWeapon")
	FGeneralWeaponData General;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeleeWeapon")
	float DistanceToDash{300.0f};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeleeWeapon|Animations")
	TMap<TSoftObjectPtr<UAnimMontage>, TSoftObjectPtr<UAnimMontage>> CharacterLightAttackMontages{};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeleeWeapon|Animations")
	TMap<TSoftObjectPtr<UAnimMontage>, TSoftObjectPtr<UAnimMontage>> CharacterHeavyAttackMontages{};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeleeWeapon|Animations")
	TMap<TSoftObjectPtr<UAnimMontage>, TSoftObjectPtr<UAnimMontage>> CharacterEnemyFarAttackMontages{};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeleeWeapon|Animations")
	TMap<TSoftObjectPtr<UAnimMontage>, TSoftObjectPtr<UAnimMontage>> CharacterInAirAttackMontages{};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeleeWeapon|VFX")
	TSoftObjectPtr<UNiagaraSystem> MeleeWeaponTrailNiagaraEffect{nullptr};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeleeWeapon|VFX")
	TSoftObjectPtr<UNiagaraSystem> MeleeWeaponHitNiagaraEffect{nullptr};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeleeWeapon|SFX")
	TSoftObjectPtr<USoundBase> MeleeWeaponHitSound{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeleeWeapon|SFX")
	TSoftObjectPtr<USoundBase> DashSound{nullptr};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeleeWeapon|Special 1")
	TSoftObjectPtr<UAnimMontage> Special1Montage{nullptr};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeleeWeapon|Special 1")
	FVector2D Special1Damage{1.0f, 1.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeleeWeapon|Special 1")
	TSoftObjectPtr<UNiagaraSystem> Special1PlayerEffect{nullptr};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeleeWeapon|Special 2")
	TSoftObjectPtr<UAnimMontage> Special2Montage{nullptr};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeleeWeapon|Special 2")
	FVector2D Special2Damage{1.0f, 1.0f};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeleeWeapon|Special 3")
	TMap<TSoftObjectPtr<UAnimMontage>, TSoftObjectPtr<UAnimMontage>> Special3Montage{{nullptr, nullptr}};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeleeWeapon|Special 3")
	FVector2D Special3Damage{1.0f, 1.0f};
};