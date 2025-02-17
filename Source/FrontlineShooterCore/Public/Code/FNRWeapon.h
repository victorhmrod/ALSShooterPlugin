// All rights reserved Wise Labs ®

#pragma once

#include "CoreMinimal.h"
#include "Character/FNRPlayerCharacter.h"
#include "Data/FSCTypes.h"
#include "GameFramework/Actor.h"
#include "FNRWeapon.generated.h"

class UFNRFireWeaponData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPickuped);

UCLASS()
class FRONTLINESHOOTERCORE_API AFNRWeapon : public AActor
{
	GENERATED_BODY()

#pragma region Components
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> MeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UInteractableComponent> InteractableComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UNiagaraComponent> GlowNiagaraComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UBillboardComponent> BillboardComponent;

#pragma endregion Components

#pragma region Variables
public:
	UPROPERTY(BlueprintAssignable)
	FOnPickuped OnPickuped;
	
	UPROPERTY(BlueprintReadOnly, Category = "Info", Replicated, ReplicatedUsing="OnRep_bIsFiring")
	bool bIsFiring;

	UFUNCTION()
	virtual void OnRep_bIsFiring();
	
	UPROPERTY(BlueprintReadOnly, Category = "References", Replicated)
	TObjectPtr<class AFNRPlayerCharacter> CharacterOwner = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "References", Replicated)
	class UFNRWeaponComponent* WeaponSystem = nullptr;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Info")
	FRarity RarityColors;

protected:
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Info")
	FGameplayTagContainer WeaponState;

public:
	UFUNCTION()
	FGameplayTagContainer GetWeaponState();
protected:
	bool bInFireRateDelay;

	#pragma region Loaded
public:
    UPROPERTY()
    UAnimMontage* LoadedUnEquipCharacterMontage;
    
    UPROPERTY()
    UAnimMontage* LoadedEquipCharacterMontage;

    UPROPERTY()
    UAnimMontage* LoadedFireCharacterMontage;
    
    UPROPERTY()
    UAnimSequence* LoadedWeaponUnEquipAnim;
    
    UPROPERTY()
    UAnimSequence* LoadedWeaponEquipAnim;
    
    UPROPERTY()
    UAnimSequence* LoadedWeaponFireAnim;

	UPROPERTY()
	USkeletalMesh* LoadedWeaponMesh;
	
	#pragma endregion

#pragma endregion Variables

#pragma region Functions
	
	#pragma region Unreal Defaults
public:	
	// Sets default values for this actor's properties
	AFNRWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void ReadValues();

	virtual void PreInitializeComponents() override;

	IFNRWeaponComponentInterface* GetWeaponComponentInterface() const;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual TArray<class UMeshComponent*> GetGlowableMeshes() const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	#pragma endregion Unreal Defaults

	#pragma region CPP Only
	
	float CurrentValue;
	FTimerHandle FadeTimerHandle;
	void SetFade(const bool bHolster);

	UFUNCTION()
	virtual void RefreshCPPOnly();
	
	#pragma endregion CPP Only
	
	#pragma region Binded Functions
protected:
	UFUNCTION()
	virtual void OnInteract(class UInteractorComponent* Interactor, UInteractableComponent* Interactable);

	#pragma endregion Binded Functions

	#pragma region Blueprint Exposed
public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Essential")
	virtual FGeneralWeaponData GetGeneralData() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Essential")
	virtual void GetFactors(float& SpreadFactor, float& DamageFactor, float& RecoilFactor) const;
	
	// Return the value of dispersion applied, 0.0f == not fire 
	UFUNCTION(BlueprintCallable, Category = "Essential")
	virtual bool Fire(const bool bFire);
	
	UFUNCTION(BlueprintCallable, Category = "Essential")
	virtual bool AIFire(const bool bFire, const FVector TargetLocation, const float Precision);

	UFUNCTION(BlueprintCallable, Category = "Essential")
	virtual bool HasAmmo();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="Data")
	void RefreshData();

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Essential")
	virtual void SetAimingStatus(bool bStatus);
	
	#pragma endregion Blueprint Exposed

	#pragma region Replicated Functions
public:
	UFUNCTION(Server, Reliable)
	void SetCharacterOwner(AFNRPlayerCharacter* NewCharacterOwner);
protected:
	virtual void Internal_SetCharacterOwner(AFNRPlayerCharacter* NewCharacterOwner);	
	
private:
	UFUNCTION(NetMulticast, Unreliable)
	void RefreshGlow() const;

	#pragma endregion Replicated Funtions
	
#pragma endregion Functions
	
};
