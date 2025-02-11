//  Wise Labs: Gameworks (c) 2020-2024

#pragma once

#include "CoreMinimal.h"
#include "FSCTypes.h"
#include "Engine/DataAsset.h"
#include "Utils/RbsTypes.h"
#include "FNRAttachmentData.generated.h"

UENUM()
enum ECompatiblity
{
	EC_ByAmmoMode UMETA(DisplayName = "Ammo Mode"),
	EC_ByWeaponName UMETA(DisplayName = "Weapon Name"),
};

UCLASS(DisplayName="AttachmentData")
class FRONTLINESHOOTERCORE_API UFNRAttachmentData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	FName DisplayName{"Attachment"};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	FName TooltipName{"Attachment ToolTip"};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	FSlateBrush Icon;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	TSoftObjectPtr<class UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info|Sounds")
	TSoftObjectPtr<USoundBase> EquipSound{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info|Sounds")
	TSoftObjectPtr<USoundBase> UnequipSound{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compatibility")
	bool AttachToWeapon{true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compatibility")
	TEnumAsByte<ECompatiblity> CompatibilityMode;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compatibility")
	TArray<FGameplayTag> CompatibleWeaponAmmoModes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compatibility")
	FName CompatibleWeaponName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	FName WeaponSocketToAttach;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	EAttachmentType AttachmentType = EAttachmentType::Other;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim Attachment")
	FTransform CameraOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim Attachment")
	float AimFOV{80.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo Attachment")
	int AddToMaxBullets = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo Attachment")
	int AmmoMeshIndex = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrel Attachment")
	bool bIsSuppressed = false;

	// Multiply base damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrel Attachment")
	float DamageMultiplier = 1.0f;

	// Multiply base recoil
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrel Attachment", DisplayName="Barrel Recoil Vertical Multiplier")
	float BarrelVRecoilMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrel Attachment", DisplayName="Barrel Recoil Horizontal Multiplier")
	float BarrelHRecoilMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stock Attachment", DisplayName="Stock Recoil Vertical Multiplier")
	float StockVRecoilMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stock Attachment", DisplayName="Stock Recoil Horizontal Multiplier")
	float StockHRecoilMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bolt Attachment")
	float FireRateMultiplier = 1.0f;

	//Defines the reference of the item that will be added to the inventory
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Inventory")
	TSubclassOf<UFNRInventoryItem> ItemClass{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment Table")
	float SilverRequired = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment Table")
	float GoldRequired = 1.0f;
};
