// All rights reserved Wise Labs ®

#pragma once

#include "CoreMinimal.h"
#include "Core/InteractorComponent.h"
#include "Data/FSCTypes.h"
#include "FNRWeaponComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipOrUnEquipWeapon, class AFNRWeapon*, WeaponBase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChangeFireMode, EFireMode_PRAS, NewFireMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFire);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttack, AActor*, Enemy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInspect, bool, bIsInspecting);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndAnyMontage, UAnimMontage*, AnimMontage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUpdateFocusedEnemy, AActor*, OldFocusedEnemy, AActor*, NewFocusedEnemy);

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class FRONTLINESHOOTERCORE_API UFNRWeaponComponent : public UInteractorComponent
{
	GENERATED_BODY()

#pragma region Variables
public:
	UPROPERTY(BlueprintReadOnly, Category = "References")
	TObjectPtr<class AFNRPlayerCharacter> ComponentOwnerCharacter = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "References")
	TObjectPtr<APlayerController> ComponentOwnerPlayerController = nullptr;

	// This property is public because if u want to add a character than has "weapon domain", you can modify this in runtime or in editor
	UPROPERTY(BlueprintReadWrite, Category = "Weapons|Info", EditAnywhere, meta = (UIMin = 0.01f))
	float DelayToEquipAnotherWeapon = 0.0f;

	// If max weapon num is <= 0 the player can't pick up any weapon
	UPROPERTY(BlueprintReadWrite, Category = "Weapons|Info", EditAnywhere)
	int MaxWeaponNum = 3;
	
	UPROPERTY(BlueprintReadWrite, Category = "Grenades|Info", EditAnywhere)
	int MaxGrenadesNum = 3;
	
	UPROPERTY(BlueprintReadOnly, Category = "Weapons|Info", EditAnywhere)
	bool bHideWeaponOnHolst = true;

	UPROPERTY(BlueprintReadWrite, Category = "Weapons|Info", EditAnywhere)
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;
	
	UPROPERTY(BlueprintReadOnly, Category = "Weapons|Info", VisibleInstanceOnly)
	bool bIsInspecting = false;
	
	UPROPERTY(EditAnywhere, Category = "Weapons|Info", DisplayName = "Attachments Widget Class")
	TSubclassOf<class UFNRAttachmentSwitch> AvailableAttachmentsWidgetClass;

	// The socket that the grenade spawns
	UPROPERTY(BlueprintReadWrite, Category = "Grenades|Info", EditDefaultsOnly)
	FName GrenadeSpawn = "hand_l";

	// X = Forward Distance, Y = Right Distance, Z = Up Distance
	UPROPERTY(BlueprintReadWrite, Category = "Grenades|Info", EditDefaultsOnly)
	FVector GrenadeOffset = FVector(150000.0f, 80.0f, -80.0f);

	UPROPERTY(BlueprintReadWrite, Category = "Grenades|Info")
	bool bCanThrow = true; 

	UPROPERTY(BlueprintReadOnly, Category = "Grenades|Input")
	bool bPredictingGrenade = false;

	UPROPERTY(BlueprintReadWrite, Category = "Weapons|Input", EditAnywhere)
	bool bHolstOnEquipSameWeapon = true;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Grenades|Info")
	TArray<TSoftClassPtr<class AFNRGrenade>> StartGrenades;

	UPROPERTY(EditDefaultsOnly, Category = "Weapons|Info")
	TArray<TSoftClassPtr<AFNRWeapon>> StartWeapons;
	
	UPROPERTY(Replicated)
	TArray<TSubclassOf<class AFNRWeapon>> CurrentWeapons;
	
	UPROPERTY(Replicated)
	TArray<class AFNRWeapon*> Weapons;

public:
	UPROPERTY(BlueprintReadOnly, Category = "References", Replicated)
	int32 CurrentWeaponIndex{-1};
	
	UPROPERTY(BlueprintReadOnly, Category = "References", Replicated)
	TObjectPtr<class AFNRWeapon> EquippedWeapon{nullptr};

	UPROPERTY(BlueprintReadOnly, Category = "References", Replicated)
	TObjectPtr<class AFNRFireWeapon> EquippedFireWeapon{nullptr};

	UPROPERTY(BlueprintReadOnly, Category = "References", Replicated)
	TObjectPtr<class AFNRMeleeWeapon> EquippedMeleeWeapon{nullptr};

	UPROPERTY(BlueprintReadOnly, Category = "References", Replicated)
	TObjectPtr<class AFNRGrenade> EquippedGrenade{nullptr};

	UPROPERTY(BlueprintReadOnly, Category = "References")
	TObjectPtr<class USplineComponent> PredictSplineComponent{nullptr};

	UPROPERTY(EditAnywhere, Category = "Weapons|Recoil")
	float RecoilInterpSpeed{65.0f};

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Weapons|Recoil")
	float LastRecoilApplied{0.0f};

protected:
	UPROPERTY(EditAnywhere, Replicated, Category = "Weapons|Ammo")
	int LightAmmo = 60;

	UPROPERTY(EditAnywhere, Replicated, Category = "Weapons|Ammo")
	int HeavyAmmo = 30;
	
	UPROPERTY(EditAnywhere, Replicated, Category = "Weapons|Ammo")
	int ShotgunAmmo= 24;
	
	UPROPERTY(EditAnywhere, Replicated, Category = "Weapons|Ammo")
	int PrecisionAmmo = 15;

	UPROPERTY(EditAnywhere, Replicated, Category = "Weapons|Ammo")
	int EnergyAmmo = 120;

	UPROPERTY(Category = "Grenades|Predict", EditDefaultsOnly)
	TSoftObjectPtr<UNiagaraSystem> GrenadePredictNiagaraEffect;
	
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> PredictEffectComponent;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Combat")
	float CombatModeRadius{1300.0f};

	#pragma region CPP Only
public:
	bool bHasMontageRunning = false;
	
private:
	bool bIsInterpolatingRecoil = false;

	int WeaponToEquipIndex = -1;
	
	UPROPERTY(Replicated)
	TArray<FString> WeaponsSockets;

	// Only used if using PredictMesh mode
	UPROPERTY()
	TArray<class USplineMeshComponent*> SplineMeshComponents;

	UPROPERTY()
	UFNRAttachmentSwitch* AttachmentsWidget;

	FTimerHandle GrenadeTimerHandle, DetectEnemyTimerHandle;

public:
	float OriginalPitch{};

	#pragma region Combat System
private:
	bool bInCombatMode = false;

	bool bAutoFind = false;

	UPROPERTY()
	TArray<AActor*> DetectedEnemies = {};

	UPROPERTY()
	AActor* EnemyInFocus = nullptr;
	
	#pragma endregion
	
	#pragma endregion CPP Only
	
#pragma endregion Variables

#pragma region Functions
#pragma region Delegates
public:
	UPROPERTY(BlueprintAssignable)
	FOnEquipOrUnEquipWeapon OnEquipOrUnEquipWeapon;
	
	UPROPERTY(BlueprintAssignable)
	FOnFire OnFire;

	UPROPERTY(BlueprintAssignable)
	FOnEndAnyMontage OnEndAnyMontage;

	// *Frytas
	UPROPERTY(BlueprintAssignable)
	FOnUpdateFocusedEnemy OnUpdateFocusedEnemy;
	// Frytas*

	UPROPERTY(BlueprintAssignable)
	FOnChangeFireMode OnChangeFireMode;

	UPROPERTY(BlueprintAssignable)
	FOnAttack OnAttack;
	
	UPROPERTY(BlueprintAssignable)
	FOnInspect OnInspect;
	
#pragma endregion Delegates

#pragma region Unreal Defaults
	// Sets default values for this component's properties
	UFNRWeaponComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ResetRecoilRefresh(float DeltaTime);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
#pragma endregion Unreal Defaults

#pragma region CPP Only
public:
	FName GetFirstHolsterSocket(const AFNRWeapon* Weapon) const;
	
	UFUNCTION(BlueprintCallable, Category = "Grenades|Essential")
	void StartSpawnGrenade();

	UFUNCTION(BlueprintCallable, Category = "Grenades|Essential")
	void FinishSpawnGrenade(const bool bCancelGrenade);
private:

	FTimerHandle EquipAnotherWeaponTimerHandle;
	UFUNCTION()
	void EquipAnotherWeapon(UAnimMontage* AnimMontage);
	
	FRotator GetPredictedGrenadeRotation() const;
	
	void PredictGrenade() const;

public:
	class IFNRWeaponComponentInterface* GetWeaponComponentInterface() const;

private:
	void Internal_EquipWeapon(const int WeaponIndex = 0, const bool& bGetGrenades = false, const EGrenadeFilter& GrenadeType = EGrenadeFilter::All);

	void Internal_EquipWeapon(AFNRWeapon* WeaponToEquip);
	
	void SetFocusEnemy(AActor* NewFocusedEnemy);
	
	void UpdateDetectedEnemies();

public:
	void ResetRecoil();
	
#pragma endregion CPP Only

#pragma region Blueprint Exposed
private:
	void SetCombatMode(const bool bCombatModeEnabled = false);
	
	UFUNCTION(Server, Reliable)
	void Server_DestroyAllWeapons();

public:
	UFUNCTION(BlueprintCallable, Category = "Weapons|Essential")
	void DestroyAllWeapons();
	
	UFUNCTION(BlueprintCallable, Category = "Weapons|Essential")
	bool PickupWeapon(AFNRWeapon* Weapon = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Weapons|Essential")
	void EquipWeapon(const int WeaponIndex = 0);

	UFUNCTION(BlueprintCallable, Category = "Weapons|Essential")
	void EquipOblivion();

	UFUNCTION(BlueprintCallable, Category = "Weapons|Essential")
	void EquipGrenade(const EGrenadeFilter GrenadeFilter = EGrenadeFilter::All);
	
	UFUNCTION(BlueprintCallable, Category = "Weapons|Essential")
	void UnEquipWeapon(const bool bForceUnequip = false);

	UFUNCTION(BlueprintCallable, Category = "Weapons|Essential")
	void DropWeapon(AFNRWeapon* WeaponToDrop, const bool bSimulatePhysics = true);

	UFUNCTION(BlueprintCallable, Category = "Weapons|Essential")
	void DropCurrentWeapon(const bool bSimulatePhysics = true);

	UFUNCTION(BlueprintCallable, Category = "Weapons|Essential")
	void DropAllWeapons(const bool bIncludeGrenades = false, const bool bSimulatePhysics = true);

	UFUNCTION(BlueprintCallable, Category = "Weapons|Essential")
	void LockCameraInTarget(const bool bLocked);

	UPROPERTY()
	bool LockedInEnemy;

	UFUNCTION(BlueprintCallable, Category = "Weapons|Essential")
	void NextFireMode() const;

	FTimerHandle BackToLastRecoilPositionTimerHandle;
	UFUNCTION(BlueprintCallable, Category = "Weapons|Essential")
	void Fire(const bool bFire);
	
	UFUNCTION(BlueprintCallable, Category = "Weapons|Essential")
	void AIFire(const bool bFire = true, const FVector TargetLocation = FVector(0.0f, 0.0f, 0.0f), const float Precision = 60.0f);

	// Try throw grenade
	UFUNCTION(BlueprintCallable, Category = "Weapons|Essential", DisplayName = "Try Throw Grenade")
	bool ThrowGrenade();
	
	UFUNCTION(BlueprintCallable, Category = "Weapons|Essential")
	void Reload() const;

	UFUNCTION(BlueprintCallable, Category = "Weapons|Essential")
	void ReloadByAnim(const int Quantity) const;
	
	UFUNCTION(BlueprintCallable, Category = "Weapons|Essential")
	void InspectWeapon(const bool bInspect);

#pragma region Set/Get Functions

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapons|Info")
	FORCEINLINE bool HasFCSMontageRunning() const
	{
		return bHasMontageRunning;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapons|Info")
	FORCEINLINE bool InCombatMode(AActor*& FocusedEnemy) const
	{
		FocusedEnemy = EnemyInFocus;
		return bInCombatMode;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapons|Info")
	FORCEINLINE TArray<AActor*> GetDetectedEnemies() const
	{
		return DetectedEnemies;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapons|Info")
	FORCEINLINE AActor* GetFocusedEnemy() const
	{
		return EnemyInFocus;
	}
	
	UFUNCTION(BlueprintCallable, Category = "Weapons|Info")
	void AttachWeapon(AFNRWeapon* Weapon, const bool bHolster);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapons|Info")
	FORCEINLINE TArray<AFNRWeapon*> GetAllWeapons() const
	{
		return Weapons;
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapons|Info")
	FORCEINLINE TArray<TSubclassOf<AFNRWeapon>> GetAllWeaponsClasses() const
	{
		return  CurrentWeapons;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapons|Info")
	TArray<AFNRWeapon*> GetWeapons() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapons|Info")
	TArray<AFNRFireWeapon*> GetFireWeapons() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapons|Info")
	TArray<AFNRMeleeWeapon*> GetMeleeWeapons() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapons|Info")
	TArray<TSubclassOf<AFNRWeapon>> GetWeaponsClasses() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapons|Info")
	TArray<FName> GetWeaponsNames(const bool bAllWeaponsName = false) const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapons|Info")
	TArray<AFNRGrenade*> GetGrenades(EGrenadeFilter GrenadeFilter = EGrenadeFilter::All);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapons|Info")
	int32 GetAmmoByType(const FGameplayTag WeaponAmmo) const;

	UFUNCTION(BlueprintCallable, Category = "Weapons|Essential", Server, Unreliable)
	void AddAmmoByType(const FGameplayTag WeaponAmmo, const int AmmoQuantity);

	UFUNCTION(BlueprintCallable, Category = "Weapons|Essential")
	void SetCanFire(const bool bCanFireNow) const;

	#pragma endregion Set/Get Functions
	
	#pragma endregion Blueprint Exposed

	#pragma region Replicated Functions
public:
    UFUNCTION(Server, Reliable)
    void ThrowGrenade_Server();

	UFUNCTION(Server, Reliable)
	void PlayMontage_Server(UAnimMontage* AnimMontage);

private:
	UFUNCTION(Server, Reliable)
	void AttachWeapon_Server(AFNRWeapon* Weapon, const bool bHolster);

	UFUNCTION(Client, Unreliable)
	void SetWeaponFade(AFNRWeapon* Weapon, const bool bHide);
	
	UFUNCTION(NetMulticast, Reliable)
	void PlayMontage_Multicast(UAnimMontage* AnimMontage);
	
	UFUNCTION(Server, Reliable)
	void AddWeapon(AFNRWeapon* Weapon);

private:
	UFUNCTION(Server, Reliable)
	void EquipWeaponServer(AFNRWeapon* Reference);

public:
	FTimerHandle CleanEquippedWeaponTimerHandle;
	UFUNCTION(Server, Reliable)
	void UnEquipWeaponServer(const bool bForceUnequip = false);

private:
	UFUNCTION(Server, Reliable)
	void DropWeapon_Server(AFNRWeapon* WeaponToDrop, const int& LocalBulletsInMag, const bool bSimulatePhysics = true);

	UFUNCTION(NetMulticast, Unreliable)
	void UpdateBullets(AFNRWeapon* Weapon, const int& LocalBulletsInMag);

	void LoadValues();

public:
	UFUNCTION()
	void ClearEquippedWeapon();

private:
	UFUNCTION(Server, Reliable)
	void Server_ClearEquippedWeapon();
	
	#pragma endregion Replicated Functions
	
#pragma endregion Functions
};
