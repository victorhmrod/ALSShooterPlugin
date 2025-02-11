// All rights reserved Wise Labs �

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FNRWeaponComponentInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UFNRWeaponComponentInterface : public UInterface
{
	GENERATED_BODY()
};


class FRONTLINESHOOTERCORE_API IFNRWeaponComponentInterface
{
	GENERATED_BODY()

public:

	virtual FTransform GetCameraTransform();
	
	virtual float GetSpreadMultiplier();

	virtual float GetDamageMultiplier();

	virtual float GetRecoilMultiplier();
	
	virtual void OnStartThrowGrenade();

	virtual void OnEndThrowGrenade(const bool bSuccess);
	
	virtual void OnCharacterStartReload();

	virtual void OnCharacterFinishReload();

	virtual void OnCharacterDropWeapon();

	virtual void OnCharacterPickupWeapon();

	virtual void OnCharacterThrowGrenade();

	virtual void OnCharacterToggleWeaponFire(const bool NewState);

	virtual void OnUpdateCombatMode(const bool bCombatModeIsEnabled);

	virtual void OnFocusedEnemyIsTooFar(AActor* CurrentEnemyInFocus);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Frontline|Weapon")
	void OnWeaponFiredOnce(const bool bFiring);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Frontline|Weapon")
	UDecalComponent* GetFocusedEnemyDecal();
};
