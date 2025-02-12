// All rights reserved Wise Labs �


#include "Data/Interfaces/FNRWeaponComponentInterface.h"

FTransform IFNRWeaponComponentInterface::GetCameraTransform()
{
	return FTransform::Identity;
}

void IFNRWeaponComponentInterface::OnStartThrowGrenade()
{
}

void IFNRWeaponComponentInterface::OnEndThrowGrenade(const bool bSuccess)
{
}

void IFNRWeaponComponentInterface::OnCharacterStartReload()
{
}

void IFNRWeaponComponentInterface::OnCharacterFinishReload()
{
}

void IFNRWeaponComponentInterface::OnCharacterDropWeapon()
{
}

void IFNRWeaponComponentInterface::OnCharacterPickupWeapon()
{
}

void IFNRWeaponComponentInterface::OnCharacterThrowGrenade()
{
}

void IFNRWeaponComponentInterface::OnCharacterToggleWeaponFire(const bool NewState)
{
}

void IFNRWeaponComponentInterface::OnUpdateCombatMode(const bool bCombatModeIsEnabled)
{
}

void IFNRWeaponComponentInterface::OnFocusedEnemyIsTooFar(AActor* CurrentEnemyInFocus)
{
}
