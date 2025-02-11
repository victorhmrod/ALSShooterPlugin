//  Wise Labs: Gameworks (c) 2020-2024

#pragma once

#include "CoreMinimal.h"
#include "Code/FNRWeapon.h"
#include "Data/FNRMeleeWeaponData.h"
#include "FNRMeleeWeapon.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResetAttack);

UCLASS()
class FRONTLINESHOOTERCORE_API AFNRMeleeWeapon : public AFNRWeapon
{
	GENERATED_BODY()

#pragma region Variables
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	UFNRMeleeWeaponData* WeaponDataAsset{nullptr};
	
public:
	UPROPERTY(BlueprintReadOnly, Category = "Info")
	bool bAllowAttackByPastAnim = false;

	bool bAllowChangeWeapon = true;

	UPROPERTY(BlueprintReadWrite, Category = "Info")
	bool bDisableKnowback = false;
	
private:
	int32 NextAttackIndex = -1;
	
#pragma endregion

#pragma region Delegates
public:
	UPROPERTY(BlueprintAssignable)
	FOnResetAttack OnResetAttack;
	
#pragma endregion
	
#pragma region Functions
	#pragma region Unreal Defaults
public:
	// Sets default values for this actor's properties
	AFNRMeleeWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	#pragma endregion

	#pragma region CPP Only
	virtual FGeneralWeaponData GetGeneralData() const override;
	
	virtual bool Fire(const bool bFire) override;

	FTimerHandle AttackTimerHandle;
	UFUNCTION(BlueprintCallable, Category = "Essential")
	virtual bool Attack(const EAttackType AttackType);

	UFUNCTION(BlueprintCallable)
	bool PlayAttackAnimation(UAnimMontage* Animation, const bool bEnableRootMotion = true);

	UAnimMontage* FindHitReactionAnimation();

	UFUNCTION(BlueprintCallable, Category = "Essential")
	virtual bool JumpToEnemy(UAnimMontage* CustomAnimation = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Essential", DisplayName = "Hit Target")
	void OnHitTarget(const FHitResult HitResult, const FVector2D Damage);
	
	virtual void SetAimingStatus(bool bStatus) override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Info")
	bool EnemyIsFarAway(const AActor* Enemy) const;

	FTimerHandle SpecialAnimationTimerHandle;
	UFUNCTION(BlueprintCallable, Category = "Essential")
	float PlaySpecialAnimation(UAnimMontage* Montage, const bool bForceCancelExistentMontages = false);

	FTimerHandle ResetGravityTimerHandle;
	UFUNCTION(BlueprintCallable, Category = "Essential")
	void ThrowEnemyToUp(ACharacter* Enemy, const float TimeInAir, const float LaunchVelocity, const bool bLaunchCharacterAlso = true);
	
	#pragma endregion
	
#pragma endregion
};
