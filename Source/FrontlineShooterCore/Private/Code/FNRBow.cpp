//  Wise Labs: Gameworks (c) 2020-2024

#include "Code/FNRBow.h"

#include "FrontlineShooterCore.h"
#include "Code/FNRWeaponComponent.h"
#include "GameFramework/Character.h"
#include "Logging/StructuredLog.h"

AFNRBow::AFNRBow()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFNRBow::BeginPlay()
{
	Super::BeginPlay();
}

void AFNRBow::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (bIsFiring)
	{
		Internal_FireWeaponData.ProjectileVelocity = FMath::FInterpConstantTo(Internal_FireWeaponData.ProjectileVelocity, Internal_FireWeaponData.MinMaxVelocity.Y,
			DeltaSeconds, Internal_FireWeaponData.MaxVelocitySpeed);
		
		FireStrength = UKismetMathLibrary::MapRangeClamped(Internal_FireWeaponData.ProjectileVelocity,
			Internal_FireWeaponData.MinMaxVelocity.X, Internal_FireWeaponData.MinMaxVelocity.Y,
			0.0f, 1.0f);
		
		UE_LOGFMT(LogFSC, Display, "Projectile Velocity = {0} | Fire Strength = {1}", Internal_FireWeaponData.ProjectileVelocity, FireStrength);
	}
	else
	{
		Internal_FireWeaponData.ProjectileVelocity = Internal_FireWeaponData.MinMaxVelocity.X;
	}
}

bool AFNRBow::Fire(const bool bFire)
{
	if (!CharacterOwner || !CharacterOwner->IsLocallyControlled() || BulletsInMag < 1)
	{
		WeaponSystem->ResetRecoil();
		Server_SetIsFiring(false);
		return false;
	}
	
	bWantsFire = bFire;
	
	if (WeaponState.HasTag(FscWeaponStateTags::CanFire) && !bIsReloading && HasAmmo())
	{
		if (bIsFiring && !bInFireRateDelay)
		{
			Server_SetIsFiring(false);
			
			bInFireRateDelay = true;

			--BulletsInMag;
					
			K2_Fire();
			FHitResult HitResult = FireTrace();
			FireProjectile(FTransform{
				GetSpread(UKismetMathLibrary::FindLookAtRotation(MeshComponent->GetSocketLocation(GetGeneralData().MuzzleSocket),
				HitResult.bBlockingHit ? HitResult.ImpactPoint : HitResult.TraceEnd)),
				MeshComponent->GetSocketLocation(GetGeneralData().MuzzleSocket), FVector::One()
			});

			UE_LOGFMT(LogFSC, Warning, "Fired Projectile Velocity = {0}", Internal_FireWeaponData.ProjectileVelocity);

			WeaponSystem->OnFire.Broadcast();

			if (Internal_FireWeaponData.FireAnimationMode.HasTag(FscFireAnimationModeTags::UseBakedFireAnimation))
			{
				WeaponSystem->PlayMontage_Server(LoadedFireCharacterMontage);
			}
			
			ApplyRecoil();

			MeshComponent->PlayAnimation(LoadedWeaponFireAnim, false);
			
			GetWorldTimerManager().SetTimer(InFireRateTimerHandle, [this]()
			{
				bInFireRateDelay = false;
				Server_SetIsFiring(false);
			}, 60 / Internal_FireWeaponData.FireRate, false);
		}
		else if (bWantsFire)
		{
			Server_SetIsFiring(true);
		}
		return true;
	}

	return false;
}
