// All rights reserved Wise Labs �

#include "Data/FSCTypes.h"

namespace FscWeaponStateTags
{
	UE_DEFINE_GAMEPLAY_TAG(CanFire, FName{TEXTVIEW("Frontline.WeaponState.CanFire")})

	UE_DEFINE_GAMEPLAY_TAG(InAds, FName{TEXTVIEW("Frontline.WeaponState.InAds")})
}

namespace FscMuzzleFlashModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(UseNiagara, FName{TEXTVIEW("Frontline.MuzzleFlashMode.UseNiagara")})

	UE_DEFINE_GAMEPLAY_TAG(UseCascade, FName{TEXTVIEW("Frontline.MuzzleFlashMode.UseCascade")})
}

namespace FscWeaponTypeTags
{
	UE_DEFINE_GAMEPLAY_TAG(AssaultRifle, FName{TEXTVIEW("Frontline.WeaponType.AssaultRifle")})
	
	UE_DEFINE_GAMEPLAY_TAG(Sword, FName{TEXTVIEW("Frontline.WeaponType.Sword")})

	UE_DEFINE_GAMEPLAY_TAG(PrecisionRifle, FName{TEXTVIEW("Frontline.WeaponType.PrecisionRifle")})

	UE_DEFINE_GAMEPLAY_TAG(Pistol, FName{TEXTVIEW("Frontline.WeaponType.Pistol")})

	UE_DEFINE_GAMEPLAY_TAG(SMG, FName{TEXTVIEW("Frontline.WeaponType.SMG")})

	UE_DEFINE_GAMEPLAY_TAG(Grenade, FName{TEXTVIEW("Frontline.WeaponType.Grenade")})

	UE_DEFINE_GAMEPLAY_TAG(Shotgun, FName{TEXTVIEW("Frontline.WeaponType.Shotgun")})
}

namespace FscFireModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(CanSingle, FName{TEXTVIEW("Frontline.FireMode.Single")})

	UE_DEFINE_GAMEPLAY_TAG(CanAuto, FName{TEXTVIEW("Frontline.FireMode.Auto")})

	UE_DEFINE_GAMEPLAY_TAG(CanBurst, FName{TEXTVIEW("Frontline.FireMode.Burst")})
}

namespace FscAmmoTypeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Light, FName{TEXTVIEW("Frontline.AmmoMode.Light")})

	UE_DEFINE_GAMEPLAY_TAG(Heavy, FName{TEXTVIEW("Frontline.AmmoMode.Heavy")})

	UE_DEFINE_GAMEPLAY_TAG(Precision, FName{TEXTVIEW("Frontline.AmmoMode.Precision")})

	UE_DEFINE_GAMEPLAY_TAG(Energy, FName{TEXTVIEW("Frontline.AmmoMode.Energy")})
	
	UE_DEFINE_GAMEPLAY_TAG(ShotgunAmmo, FName{TEXTVIEW("Frontline.AmmoMode.Shotgun")})
}

namespace FscProjectileModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Rocket, FName{TEXTVIEW("Frontline.ProjectileType.Rocket")})
	
	UE_DEFINE_GAMEPLAY_TAG(Cartridge, FName{TEXTVIEW("Frontline.ProjectileType.Cartridge")})	
}

namespace FscRarityType
{
	UE_DEFINE_GAMEPLAY_TAG(Commom, FName{TEXTVIEW("Frontline.RarityMode.Commom")})

	UE_DEFINE_GAMEPLAY_TAG(Uncommom, FName{TEXTVIEW("Frontline.RarityMode.Uncommom")})

	UE_DEFINE_GAMEPLAY_TAG(Rare, FName{TEXTVIEW("Frontline.RarityMode.Rare")})

	UE_DEFINE_GAMEPLAY_TAG(UltraRare, FName{TEXTVIEW("Frontline.RarityMode.UltraRare")})

	UE_DEFINE_GAMEPLAY_TAG(Divine, FName{TEXTVIEW("Frontline.RarityMode.Divine")})

	UE_DEFINE_GAMEPLAY_TAG(Mythic, FName{TEXTVIEW("Frontline.RarityMode.Mythic")})
}
