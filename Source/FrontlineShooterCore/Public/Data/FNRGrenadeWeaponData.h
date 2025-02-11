//  Wise Labs: Gameworks (c) 2020-2024

#pragma once

#include "CoreMinimal.h"
#include "FSCTypes.h"
#include "Engine/DataAsset.h"
#include "FNRGrenadeWeaponData.generated.h"

UCLASS()
class FRONTLINESHOOTERCORE_API UFNRGrenadeWeaponData : public UDataAsset
{
	GENERATED_BODY()

public:
	UFNRGrenadeWeaponData();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	FGeneralWeaponData General;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	EGrenadeType GrenadeType = EGrenadeType::Fragmentation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	float GrenadeSpeed = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	float GrenadeRadius = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	float TimeToExplode = 3.0f;
};