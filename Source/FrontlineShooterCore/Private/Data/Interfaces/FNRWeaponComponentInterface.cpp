// All rights reserved Wise Labs �


#include "Data/Interfaces/FNRWeaponComponentInterface.h"

FTransform IFNRWeaponComponentInterface::GetCameraTransform()
{
	return FTransform::Identity;
}

float IFNRWeaponComponentInterface::GetSpreadMultiplier()
{
	return 0;
}

float IFNRWeaponComponentInterface::GetDamageMultiplier()
{
	return 0;
}

float IFNRWeaponComponentInterface::GetRecoilMultiplier()
{
	return 0;
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
