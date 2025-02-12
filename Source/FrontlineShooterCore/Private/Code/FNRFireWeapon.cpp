// All rights reserved Wise Labs �

#include "Code/FNRFireWeapon.h"

#include "FrontlineShooterCore.h"
#include "Code/FNRWeaponComponent.h"
#include "Data/Interfaces/FNRWeaponComponentInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Code/FNRAttachment.h"
#include "Code/FNRAttachmentWeaponComponent.h"
#include "Code/FNRCartridgeProjectile.h"
#include "Code/FNRRocketProjectile.h"
#include "Components/ArrowComponent.h"
#include "Core/InteractableComponent.h"
#include "Data/FNRAttachmentData.h"
#include "Data/FNRFireWeaponData.h"
#include "Net/Core/PushModel/PushModel.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Logging/StructuredLog.h"

#pragma region Unreal Defaults

AFNRFireWeapon::AFNRFireWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	AttachmentComponent = CreateDefaultSubobject<UFNRAttachmentWeaponComponent>(FName{TEXTVIEW("AttachmentComponent")});
	MagazineComponent = CreateDefaultSubobject<UStaticMeshComponent>(FName{TEXTVIEW("MagazineMesh")});
	MagazineComponent->SetIsReplicated(true);
	MagazineComponent->SetupAttachment(MeshComponent);
	MagazineComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MagazineComponent->SetReceivesDecals(false);
	IronsightLocationComponent = CreateDefaultSubobject<UArrowComponent>(FName{TEXTVIEW("IronsightCamera")});
	IronsightLocationComponent->SetupAttachment(MeshComponent);
	RecoilAnimationComponent = CreateDefaultSubobject<URecoilAnimationComponent>(FName{TEXTVIEW("RecoilAnimationComponent")});
	RotatingMovementComponent = CreateDefaultSubobject<URotatingMovementComponent>(FName{TEXTVIEW("RotatingMovementComponent")});
	RotatingMovementComponent->RotationRate = FRotator(0, 90.f, 0);
	RotatingMovementComponent->SetAutoActivate(false);
}

void AFNRFireWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	RefreshRecoilOffset();
}

TArray<UMeshComponent*> AFNRFireWeapon::GetGlowableMeshes() const
{
	TArray<UMeshComponent*> FoundArray;
	FoundArray.Add(MagazineComponent);
	FoundArray.Append(Super::GetGlowableMeshes());
	return FoundArray;
}

void AFNRFireWeapon::BeginPlay()
{
	Super::BeginPlay();
	if (!IsValid(WeaponDataAsset))
	{
		#if WITH_EDITOR
			UKismetSystemLibrary::PrintString(this, "You forgot to put the data asset on this weapon", true, true, FLinearColor::Green, 25.f);
		#endif
		UE_LOGFMT(LogFSC, Display, "You forgot to put the data asset on this weapon");
	}
	if (HasAuthority())
	{
		MeshComponent->SetSimulatePhysics(SimulatePhysics);
	}
	if (WeaponState.HasTag(FscWeaponStateTags::InAds))
	{
		WeaponState.RemoveTag(FscWeaponStateTags::InAds);
	}
	CurrentFireMode = Internal_FireWeaponData.StartFireMode;
	BulletsInMag = FMath::Min(Internal_FireWeaponData.InitialMagAmmo,Internal_FireWeaponData.MaxAmmoInMag);
	RecoilAnimationComponent->Init(Internal_FireWeaponData.CharacterFireAnim, Internal_FireWeaponData.FireRate, Internal_FireWeaponData.BulletsPerShoot);
	RecoilAnimationComponent->SetFireMode(Internal_FireWeaponData.StartFireMode);
	RotatingMovementComponent->SetActive(bEnableRotation, false);
	InteractableComponent->SetTooltipText(FString::FromInt(BulletsInMag));
}

void AFNRFireWeapon::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void AFNRFireWeapon::ReadValues()
{
	Super::ReadValues();
	
	if (!IsValid(WeaponDataAsset)) return;
	Internal_FireWeaponData = WeaponDataAsset->AsStructure(WeaponDataAsset);
	IronsightLocationComponent->AttachToComponent(MeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, Internal_FireWeaponData.AimingCameraSocket);
	MagazineComponent->AttachToComponent(MeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, Internal_FireWeaponData.MagazineSocket);
	if (IsValid(Internal_FireWeaponData.FireSound.LoadSynchronous()))
	{
		LoadedFireSound = Internal_FireWeaponData.FireSound.Get();
	}
	if (IsValid(Internal_FireWeaponData.WeaponMagazineMesh.LoadSynchronous()))
	{
		MagazineComponent->SetStaticMesh(Internal_FireWeaponData.WeaponMagazineMesh.Get());
	}
	if (IsValid(Internal_FireWeaponData.ProjectileMaterial.LoadSynchronous()))
	{
		LoadedProjectileMaterial = Internal_FireWeaponData.ProjectileMaterial.Get();
	}
	if (IsValid(Internal_FireWeaponData.ProjectileNiagaraParticle.LoadSynchronous()))
	{
		LoadedProjectileNiagaraParticle = Internal_FireWeaponData.ProjectileNiagaraParticle.Get();
	}
	if (IsValid(Internal_FireWeaponData.ProjectileMaterial.LoadSynchronous()))
	{
		LoadedProjectileMaterial = Internal_FireWeaponData.ProjectileMaterial.Get();
	}
	if (IsValid(CartridgeProjectileClass.LoadSynchronous()))
	{
		LoadedCartridgeProjectileClass = CartridgeProjectileClass.Get();
	}
	if (IsValid(RocketProjectileClass.LoadSynchronous()))
	{
		LoadedRocketProjectileClass = RocketProjectileClass.Get();
	}
	if (IsValid(Internal_FireWeaponData.CharacterReloadMontage.LoadSynchronous()))
	{
		LoadedReloadCharacterMontage = Internal_FireWeaponData.CharacterReloadMontage.Get();
	}
	if (IsValid(Internal_FireWeaponData.EmptyMagSound.LoadSynchronous()))
	{
		LoadedEmptyMagSound = Internal_FireWeaponData.EmptyMagSound.Get();
	}
	if (IsValid(Internal_FireWeaponData.SilencedFireSound.LoadSynchronous()))
	{
		LoadedSilencedSound = Internal_FireWeaponData.SilencedFireSound.Get();
	}
	if (IsValid(Internal_FireWeaponData.MuzzleFlashCascadeParticle.LoadSynchronous()))
	{
		LoadedCascadedParticle = Internal_FireWeaponData.MuzzleFlashCascadeParticle.Get();
	}
	if (IsValid(Internal_FireWeaponData.WeaponMagazineMesh.LoadSynchronous()))
	{
		LoadedMagazineMesh = Internal_FireWeaponData.WeaponMagazineMesh.Get();
	}
	if (IsValid(Internal_FireWeaponData.WeaponReloadAnim.LoadSynchronous()))
	{
		LoadedWeaponReloadAnim = Internal_FireWeaponData.WeaponReloadAnim.Get();
	}
	if (IsValid(Internal_FireWeaponData.MuzzleFlashNiagaraParticle.LoadSynchronous()))
	{
		LoadedNiagaraParticle = Internal_FireWeaponData.MuzzleFlashNiagaraParticle.Get();
	}
}

void AFNRFireWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFNRFireWeapon, CharacterOwner);
	DOREPLIFETIME(AFNRFireWeapon, WeaponSystem);
	DOREPLIFETIME(AFNRFireWeapon, SimulatePhysics);
	DOREPLIFETIME(AFNRFireWeapon, RecoilOffset);
	DOREPLIFETIME(AFNRFireWeapon, WeaponState);
	FDoRepLifetimeParams BulletParameters;
	BulletParameters.bIsPushBased = true;
	BulletParameters.Condition = COND_SkipOwner;
	BulletParameters.RepNotifyCondition = REPNOTIFY_Always;
	DOREPLIFETIME_WITH_PARAMS_FAST(AFNRFireWeapon, CartridgeSpawnData, BulletParameters);
	DOREPLIFETIME_WITH_PARAMS_FAST(AFNRFireWeapon, RocketSpawnData, BulletParameters);
}

#pragma endregion Unreal Defaults

#pragma region CPP Only
void AFNRFireWeapon::RefreshRecoilOffset()
{
	RecoilOffset = RecoilAnimationComponent->GetOutput();
}

bool AFNRFireWeapon::AutoFire()
{
	if (bWantsFire && WeaponState.HasTag(FscWeaponStateTags::CanFire) && !bIsReloading && !bInFireRateDelay && HasAmmo())
	{
		Server_SetIsFiring(true);
		bInFireRateDelay = true;
		if (CharacterOwner->IsLocallyControlled())
		{
			--BulletsInMag;
		}
		for (int i = 0; i < Internal_FireWeaponData.BulletsPerShoot; i++)
		{
			K2_Fire();
			const FHitResult HitResult = FireTrace();
			FRotator ProjectileRotation = GetSpread(UKismetMathLibrary::FindLookAtRotation(MeshComponent->GetSocketLocation(GetGeneralData().MuzzleSocket), HitResult.bBlockingHit ? HitResult.ImpactPoint : HitResult.TraceEnd), 0);
			FireProjectile(FTransform{ProjectileRotation, MeshComponent->GetSocketLocation(GetGeneralData().MuzzleSocket), FVector::One()});
			WeaponSystem->OnFire.Broadcast();
		}
		ApplyRecoil();
		MeshComponent->PlayAnimation(LoadedWeaponFireAnim, false);
		RecoilAnimationComponent->Play();
		GetWeaponComponentInterface()->Execute_OnWeaponFiredOnce(CharacterOwner, true);
		if (Internal_FireWeaponData.FireAnimationMode.HasTag(FscFireAnimationModeTags::UseBakedFireAnimation))
		{
			WeaponSystem->PlayMontage_Server(LoadedFireCharacterMontage);
		}
		GetWorldTimerManager().SetTimer(InFireRateTimerHandle, [this]
		{
			bInFireRateDelay = false;
			AutoFire();
		}, GetFireRate(), false);
		return true;
	}
	if (!bWantsFire && !WeaponState.HasTag(FscWeaponStateTags::CanFire) && !HasAmmo())
	{
		UGameplayStatics::PlaySoundAtLocation(this, LoadedEmptyMagSound, MeshComponent->GetSocketLocation(GetGeneralData().MuzzleSocket), FRotator::ZeroRotator, 0.5f);
	}
	Server_SetIsFiring(false);
	RecoilAnimationComponent->Stop();
	GetWeaponComponentInterface()->Execute_OnWeaponFiredOnce(CharacterOwner, false);
	return false;
}

bool AFNRFireWeapon::SemiFire()
{
	if (bWantsFire && WeaponState.HasTag(FscWeaponStateTags::CanFire) && !bIsReloading && !bInFireRateDelay && HasAmmo())
	{
		Server_SetIsFiring(true);
		bInFireRateDelay = true;
		if (CharacterOwner->IsLocallyControlled())
		{
			--BulletsInMag;
		}
		for (int i = 0; i < Internal_FireWeaponData.BulletsPerShoot; i++)
		{
			K2_Fire();
			const FHitResult HitResult = FireTrace();
			FRotator ProjectileRotation = GetSpread(UKismetMathLibrary::FindLookAtRotation(MeshComponent->GetSocketLocation(GetGeneralData().MuzzleSocket), HitResult.bBlockingHit ? HitResult.ImpactPoint : HitResult.TraceEnd), 0);
			FireProjectile(FTransform{ProjectileRotation, MeshComponent->GetSocketLocation(GetGeneralData().MuzzleSocket), FVector::One()});
			WeaponSystem->OnFire.Broadcast();
		}
		ApplyRecoil();
		RecoilAnimationComponent->Play();
		MeshComponent->PlayAnimation(LoadedWeaponFireAnim, false);
		if (Internal_FireWeaponData.FireAnimationMode.HasTag(FscFireAnimationModeTags::UseBakedFireAnimation))
		{
			WeaponSystem->PlayMontage_Server(LoadedFireCharacterMontage);
		}
		GetWorldTimerManager().SetTimer(InFireRateTimerHandle, [this]()
		{
			bInFireRateDelay = false;
			Server_SetIsFiring(false);
		}, GetFireRate(), false);
		GetWeaponComponentInterface()->Execute_OnWeaponFiredOnce(CharacterOwner, true);
		return true;
	}
	UGameplayStatics::PlaySoundAtLocation(this, LoadedEmptyMagSound, MeshComponent->GetSocketLocation(GetGeneralData().MuzzleSocket));
	Server_SetIsFiring(false);
	RecoilAnimationComponent->Stop();
	GetWeaponComponentInterface()->Execute_OnWeaponFiredOnce(CharacterOwner, false);
	return false;
}

bool AFNRFireWeapon::BurstFire()
{
    if (GetWorldTimerManager().TimerExists(FireWithDelayTimerHandle))
    {
        Server_SetIsFiring(false);
    	GetWeaponComponentInterface()->Execute_OnWeaponFiredOnce(CharacterOwner, false);
        return false;
    }
	if (CharacterOwner->IsLocallyControlled())
	{
		--BulletsInMag;
	}
	GetWeaponComponentInterface()->Execute_OnWeaponFiredOnce(CharacterOwner, true);
	K2_Fire();
	const FHitResult HitResult = FireTrace();
	const FRotator ProjectileRotation = GetSpread(UKismetMathLibrary::FindLookAtRotation(MeshComponent->GetSocketLocation(GetGeneralData().MuzzleSocket), HitResult.bBlockingHit ? HitResult.ImpactPoint : HitResult.TraceEnd), 0);
	FireProjectile(FTransform{ProjectileRotation, MeshComponent->GetSocketLocation(GetGeneralData().MuzzleSocket), FVector::One()});
	ApplyRecoil();
	WeaponSystem->OnFire.Broadcast();
    Server_SetIsFiring(true);
    BulletsRemaining = Internal_FireWeaponData.BulletsPerShoot - 1;
    GetWorldTimerManager().SetTimer(FireWithDelayTimerHandle, [this]
    {
        if (BulletsInMag > 0 && BulletsRemaining > 0)
        {
        	Server_SetIsFiring(true);
        	GetWeaponComponentInterface()->Execute_OnWeaponFiredOnce(CharacterOwner, true);
        	K2_Fire();
        	const FHitResult HitResult = FireTrace();
        	const FRotator ProjectileRotation = GetSpread(UKismetMathLibrary::FindLookAtRotation(MeshComponent->GetSocketLocation(GetGeneralData().MuzzleSocket), HitResult.bBlockingHit ? HitResult.ImpactPoint : HitResult.TraceEnd), 0);
        	FireProjectile(FTransform{ProjectileRotation, MeshComponent->GetSocketLocation(GetGeneralData().MuzzleSocket), FVector::One()});
        	ApplyRecoil();
	        if (CharacterOwner->IsLocallyControlled())
	        {
		        --BulletsInMag;
	        }
        	WeaponSystem->OnFire.Broadcast();
        	--BulletsRemaining;
        }
        else
        {
            GetWorldTimerManager().ClearTimer(FireWithDelayTimerHandle);
            Server_SetIsFiring(false);
        }
    }, Internal_FireWeaponData.DelayBetweenBullets, true);
	RecoilAnimationComponent->Play();
    if (Internal_FireWeaponData.FireAnimationMode.HasTag(FscFireAnimationModeTags::UseBakedFireAnimation))
    {
        WeaponSystem->PlayMontage_Server(LoadedFireCharacterMontage);
    }
    MeshComponent->PlayAnimation(LoadedWeaponFireAnim, false);
    GetWorldTimerManager().SetTimer(InFireRateTimerHandle, [this]()
    {
        bInFireRateDelay = false;
    }, GetFireRate(), false);
    return true;
}

void AFNRFireWeapon::ApplyRecoil()
{
	LastTimeCurve += Internal_FireWeaponData.RecoilVelocity;
	float TargetPitchRecoil = Internal_FireWeaponData.RecoilPitchCurve->GetFloatValue(LastTimeCurve) * GetWeaponComponentInterface()->Execute_GetRecoilMultiplier(CharacterOwner);
	float TargetYawRecoil = Internal_FireWeaponData.RecoilYawCurve->GetFloatValue(LastTimeCurve) * GetWeaponComponentInterface()->Execute_GetRecoilMultiplier(CharacterOwner);

	if (const auto& Barrel = AttachmentComponent->GetAttachmentByType(EAttachmentType::Barrel))
	{
		TargetPitchRecoil *= Barrel->Attachment->BarrelVRecoilMultiplier;
		TargetYawRecoil *= Barrel->Attachment->BarrelHRecoilMultiplier;
	}
	if (const auto& Stock = AttachmentComponent->GetAttachmentByType(EAttachmentType::Stock))
	{
		TargetPitchRecoil *= Stock->Attachment->BarrelVRecoilMultiplier;
		TargetYawRecoil *= Stock->Attachment->BarrelHRecoilMultiplier;
	}
	WeaponSystem->LastRecoilApplied += TargetPitchRecoil;
	CharacterOwner->AddControllerPitchInput(TargetPitchRecoil);
	CharacterOwner->AddControllerYawInput(TargetYawRecoil);
	GetWorldTimerManager().ClearTimer(ResetRecoilTimerHandle);
	GetWorldTimerManager().SetTimer(ResetRecoilTimerHandle, [this]
	{
		LastTimeCurve = 0.0f;
	}, Internal_FireWeaponData.TimeToResetRecoil, false);
}

FHitResult AFNRFireWeapon::FireTrace()
{
	if (!IsValid(CharacterOwner))
	{
		return{};
	}
	FVector CameraLocation{};
	FRotator CameraRotation{};
	if (CharacterOwner->IsBotControlled() || !CharacterOwner->IsPlayerControlled())
	{
		CharacterOwner->GetActorEyesViewPoint(CameraLocation, CameraRotation);
	}
	else
	{
		CameraLocation = GetWeaponComponentInterface()->GetCameraTransform().GetLocation();
		CameraRotation = GetWeaponComponentInterface()->GetCameraTransform().Rotator();
	}
	const FVector StartLocation = CameraLocation + CameraRotation.Vector() * (CameraLocation - MeshComponent->GetSocketLocation(GetGeneralData().MuzzleSocket)).Length();
	const FVector EndLocation = StartLocation + CameraRotation.Vector() * 100000.0f;
	FHitResult HitResult;
	const TArray<AActor*> ActorsToIgnore{this, CharacterOwner};
	UKismetSystemLibrary::LineTraceSingle(this, StartLocation, EndLocation, UEngineTypes::ConvertToTraceType(WeaponSystem->TraceChannel), false, ActorsToIgnore, WeaponSystem->GetDrawDebugType(), HitResult, true);
	return HitResult;
}

FRotator AFNRFireWeapon::GetSpread(const FRotator& CurrentRotation, const float& Precision) const
{
	if (!IsValid(CharacterOwner)) return FRotator::ZeroRotator;
	const FRotator SpreadVector = FRotator{
		CurrentRotation.Pitch + FMath::RandRange(Internal_FireWeaponData.SpreadRange.X / 10, Internal_FireWeaponData.SpreadRange.Y / 10) * GetWeaponComponentInterface()->Execute_GetSpreadMultiplier(CharacterOwner),
		CurrentRotation.Yaw + FMath::RandRange(Internal_FireWeaponData.SpreadRange.X / 10, Internal_FireWeaponData.SpreadRange.Y / 10) * GetWeaponComponentInterface()->Execute_GetSpreadMultiplier(CharacterOwner),
		CurrentRotation.Roll + FMath::RandRange(Internal_FireWeaponData.SpreadRange.X / 10, Internal_FireWeaponData.SpreadRange.Y / 10) * GetWeaponComponentInterface()->Execute_GetSpreadMultiplier(CharacterOwner)};
	return CharacterOwner->IsPlayerControlled() ? SpreadVector : SpreadVector * (FMath::GetMappedRangeValueUnclamped(FVector2D(0.0f, 100.0f), FVector2D(2.0f, 1.0f), Precision));
}

void AFNRFireWeapon::RefreshCPPOnly()
{
	Super::RefreshCPPOnly();
	if (IsValid(LoadedMagazineMesh))
	{
		MagazineComponent->SetStaticMesh(LoadedMagazineMesh);
	}
	if (IsValid(MagazineComponent))
	{
		MagazineComponent->AttachToComponent(MeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, Internal_FireWeaponData.MagazineSocket);
	}
	if (IsValid(IronsightLocationComponent))
	{
		IronsightLocationComponent->AttachToComponent(MeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, Internal_FireWeaponData.AimingCameraSocket);
	}
	InteractableComponent->SetTooltipText(FString::FromInt(BulletsInMag));
}

#pragma endregion CPP Only

#pragma region Blueprint Exposed
bool AFNRFireWeapon::HasAmmo()
{
	return BulletsInMag > 0;
}

void AFNRFireWeapon::Server_SetIsFiring_Implementation(const bool bFiring)
{
	//bIsFiring = bFiring;
	Multicast_SetIsFiring(bFiring);
}

void AFNRFireWeapon::Multicast_SetIsFiring_Implementation(const bool bFiring)
{
	bIsFiring = bFiring;
	if (bIsFiring)
	{
		if (Internal_FireWeaponData.MuzzleFlashMode.HasTag(FscMuzzleFlashModeTags::UseCascade) && Internal_FireWeaponData.MuzzleFlashMode.HasTag(FscMuzzleFlashModeTags::UseNiagara))
		{
			if (LoadedCascadedParticle)
			{
				UGameplayStatics::SpawnEmitterAttached(
					LoadedCascadedParticle, MeshComponent, GetGeneralData().MuzzleSocket, FVector(ForceInit), FRotator::ZeroRotator,
					WeaponState.HasTag(FscWeaponStateTags::InAds) ? Internal_FireWeaponData.MuzzleFlashCascadeParticleScale * 0.5f : Internal_FireWeaponData.MuzzleFlashCascadeParticleScale
				);
			}
			if (LoadedNiagaraParticle)
			{
				UNiagaraFunctionLibrary::SpawnSystemAttached(
					LoadedNiagaraParticle, MeshComponent, GetGeneralData().MuzzleSocket, FVector(0.f), FRotator(0.f),
					WeaponState.HasTag(FscWeaponStateTags::InAds) ? Internal_FireWeaponData.MuzzleFlashNiagaraParticleScale * 0.5f : Internal_FireWeaponData.MuzzleFlashNiagaraParticleScale,
					EAttachLocation::KeepRelativeOffset, true, ENCPoolMethod::AutoRelease
				);
			}
		}
		else if (Internal_FireWeaponData.MuzzleFlashMode.HasTag(FscMuzzleFlashModeTags::UseNiagara))
		{
			if (LoadedNiagaraParticle)
			{
				UNiagaraFunctionLibrary::SpawnSystemAttached(
					LoadedNiagaraParticle, MeshComponent, GetGeneralData().MuzzleSocket, FVector(0.f), FRotator(0.f),
					WeaponState.HasTag(FscWeaponStateTags::InAds) ? Internal_FireWeaponData.MuzzleFlashNiagaraParticleScale * 0.5f : Internal_FireWeaponData.MuzzleFlashNiagaraParticleScale,
					EAttachLocation::KeepRelativeOffset, true, ENCPoolMethod::AutoRelease
				);
			}
		}
		else if (Internal_FireWeaponData.MuzzleFlashMode.HasTag(FscMuzzleFlashModeTags::UseCascade))
		{
			if (LoadedCascadedParticle)
			{
				UGameplayStatics::SpawnEmitterAttached(
					LoadedCascadedParticle, MeshComponent, GetGeneralData().MuzzleSocket, FVector(ForceInit), FRotator::ZeroRotator,
					WeaponState.HasTag(FscWeaponStateTags::InAds) ? Internal_FireWeaponData.MuzzleFlashCascadeParticleScale * 0.5f : Internal_FireWeaponData.MuzzleFlashCascadeParticleScale
				);
			}
		}

		UGameplayStatics::PlaySoundAtLocation(this, AttachmentComponent->GetAttachmentByType(EAttachmentType::Supressor) ? LoadedSilencedSound : LoadedFireSound, MeshComponent->GetSocketLocation(GetGeneralData().MuzzleSocket), FRotator::ZeroRotator, 0.2f);
	}
	else
	{
		WeaponSystem->ResetRecoil();
	}
}

bool AFNRFireWeapon::Fire(const bool bFire)
{
	if (!CharacterOwner || !CharacterOwner->IsPlayerControlled() || BulletsInMag < 1)
	{
		Server_SetIsFiring(false);
		RecoilAnimationComponent->Stop();
		return false;
	}
	bWantsFire = bFire;
	RecoilAnimationComponent->Stop();
	Server_SetIsFiring(false);
	if (bWantsFire && WeaponState.HasTag(FscWeaponStateTags::CanFire) && !bIsReloading && !bInFireRateDelay)
	{
		switch (CurrentFireMode)
		{
		default: case Auto:
			{
				return AutoFire();
			}
			case Semi:
			{
				return SemiFire();
			}
		case Burst:
			{
				return BurstFire();
			}
		}
	}
	Server_SetIsFiring(false);
	RecoilAnimationComponent->Stop();
	return false;
}

bool AFNRFireWeapon::AIFire(const bool bFire, const FVector TargetLocation, const float Precision)
{
	if (!IsValid(CharacterOwner) || (IsValid(CharacterOwner) && CharacterOwner->IsPlayerControlled()))
	{
		Server_SetIsFiring(false);
		RecoilAnimationComponent->Stop();
		return false;
	}
	if (BulletsInMag < 1)
	{
		Reload();
		return false;
	}
	bWantsFire = bFire;
	RecoilAnimationComponent->Stop();
	Server_SetIsFiring(false);
	if (bWantsFire && WeaponState.HasTag(FscWeaponStateTags::CanFire) && !bIsReloading && !bInFireRateDelay)
	{
		switch (CurrentFireMode)
		{
		default: case Auto:
			{
				return AutoFire();
			}
		case Semi:
			{
				return SemiFire();
			}
		case Burst:
			{
				return BurstFire();
			}
		}
	}
	Server_SetIsFiring(false);
	RecoilAnimationComponent->Stop();
	return false;
}

float AFNRFireWeapon::GetFireRate() const
{
	return AttachmentComponent->GetAttachmentByType(EAttachmentType::Bolt) ? 60 / (Internal_FireWeaponData.FireRate * AttachmentComponent->GetAttachmentByType(EAttachmentType::Bolt)->Attachment->FireRateMultiplier) : 60 / Internal_FireWeaponData.FireRate;
}

void AFNRFireWeapon::Reload()
{
	if (WeaponSystem && BulletsInMag < Internal_FireWeaponData.MaxAmmoInMag && !bIsReloading && WeaponSystem->GetAmmoByType(Internal_FireWeaponData.AmmoMode) > 0)
	{
		GetWeaponComponentInterface()->OnCharacterStartReload();
		if (bIsFiring)
		{
			Fire(false);
		}
		bIsReloading = true;
		MeshComponent->PlayAnimation(LoadedWeaponReloadAnim, false);
		if (Internal_FireWeaponData.CharacterReloadMontage)
		{
			WeaponSystem->PlayMontage_Server(LoadedReloadCharacterMontage);
			GetWorldTimerManager().SetTimer(ReloadTimerHandle, [this]
			{
				EndReload();
				bIsReloading = false;
			}, Internal_FireWeaponData.CharacterReloadMontage->GetPlayLength(), false);
		}
		else
		{
			GetWorldTimerManager().SetTimer(ReloadTimerHandle, [this]
			{
				EndReload();
				bIsReloading = false;
			}, Internal_FireWeaponData.ReloadTime, false);
		}
	}
}

void AFNRFireWeapon::ReloadByAnim(const int& Quantity)
{
	if (WeaponSystem && BulletsInMag < Internal_FireWeaponData.MaxAmmoInMag && bIsReloading && WeaponSystem->GetAmmoByType(Internal_FireWeaponData.AmmoMode) > 0)
	{
		EndReloadByAnim(Quantity);
	}
}

void AFNRFireWeapon::SetAimingStatus(const bool bStatus)
{
	Super::SetAimingStatus(bStatus);
	RecoilAnimationComponent->SetAimingStatus(bStatus);
}

EFireMode_PRAS AFNRFireWeapon::GetCurrentFireMode() const
{
	return CurrentFireMode;
}

void AFNRFireWeapon::FireProjectile(const FTransform& ProjectileTransform)
{
	if (Internal_FireWeaponData.ProjectileMode == FscProjectileModeTags::Rocket)
	{
		SpawnRocketProjectile(ProjectileTransform);
		Server_SpawnRocket(ProjectileTransform);
	}
	else
	{
		SpawnCartridgeProjectile(ProjectileTransform);
		Server_SpawnCartridge(ProjectileTransform);
	}
}

void AFNRFireWeapon::SetFireMode(const EFireMode_PRAS& NewFireMode)
{
	CurrentFireMode = NewFireMode;
	RecoilAnimationComponent->SetFireMode(NewFireMode);
}

void AFNRFireWeapon::Server_SpawnRocket_Implementation(const FTransform& ProjectileTransform)
{
	RocketSpawnData = FBulletSpawnData(ProjectileTransform);
	OnRep_RocketSpawnData();
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, RocketSpawnData, this);
}

void AFNRFireWeapon::SpawnCartridgeProjectile(const FTransform& ProjectileTransform) const
{
	if (!IsValid(LoadedCartridgeProjectileClass)) return;
	if (AFNRCartridgeProjectile* ProjectileRef = GetWorld()->SpawnActorDeferred<AFNRCartridgeProjectile>(LoadedCartridgeProjectileClass, ProjectileTransform, CharacterOwner, CharacterOwner))
	{
		ProjectileRef->Damage = FMath::FRandRange(Internal_FireWeaponData.DamageRange.X, Internal_FireWeaponData.DamageRange.Y);
		if (const auto& Barrel = AttachmentComponent->GetAttachmentByType(EAttachmentType::Barrel))
		{
			ProjectileRef->Damage *= Barrel->Attachment->DamageMultiplier;
		}
		ProjectileRef->MuzzleVelocity = Internal_FireWeaponData.ProjectileVelocity;
		ProjectileRef->ProjectileMaterial = LoadedProjectileMaterial;
		ProjectileRef->ShrapnelFlameFX = LoadedProjectileNiagaraParticle;
		UGameplayStatics::FinishSpawningActor(ProjectileRef, ProjectileTransform);
	}
}

void AFNRFireWeapon::SpawnRocketProjectile(const FTransform& ProjectileTransform) const
{
	if (!IsValid(LoadedCartridgeProjectileClass)) return;
	if (AFNRRocketProjectile* ProjectileRef = GetWorld()->SpawnActorDeferred<AFNRRocketProjectile>(LoadedRocketProjectileClass, ProjectileTransform, CharacterOwner, CharacterOwner))
	{
		ProjectileRef->Damage = FMath::FRandRange(Internal_FireWeaponData.DamageRange.X, Internal_FireWeaponData.DamageRange.Y);
		if (const auto& Barrel = AttachmentComponent->GetAttachmentByType(EAttachmentType::Barrel))
		{
			ProjectileRef->Damage *= Barrel->Attachment->DamageMultiplier;
		}
		UGameplayStatics::FinishSpawningActor(ProjectileRef, ProjectileTransform);
	}
}

void AFNRFireWeapon::Server_SpawnCartridge_Implementation(const FTransform& ProjectileTransform)
{
	CartridgeSpawnData = FBulletSpawnData(ProjectileTransform);
	OnRep_CartridgeSpawnData();
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, CartridgeSpawnData, this);
}

void AFNRFireWeapon::OnRep_CartridgeSpawnData() const
{
	if (!IsValid(CharacterOwner)) return;
	if (!CharacterOwner->IsLocallyControlled())
	{
		SpawnCartridgeProjectile(CartridgeSpawnData.ProjectileTransform);
	}
}

void AFNRFireWeapon::OnRep_RocketSpawnData() const
{
	if (IsValid(CharacterOwner))
	{
		if (!CharacterOwner->IsLocallyControlled())
		{
			SpawnRocketProjectile(RocketSpawnData.ProjectileTransform);
		}
	}
}

#pragma endregion Blueprint Exposed

#pragma region Replicated Functions

void AFNRFireWeapon::EndReload()
{
	const int StoredAmmo = WeaponSystem->GetAmmoByType(Internal_FireWeaponData.AmmoMode);
	const AFNRAttachment* AmmoAttachment = AttachmentComponent->GetAttachmentByType(EAttachmentType::Ammo);
	const int MaxAmmoInMag = AmmoAttachment ? Internal_FireWeaponData.MaxAmmoInMag + AmmoAttachment->Attachment->AddToMaxBullets : Internal_FireWeaponData.MaxAmmoInMag;
	const int BulletsToAdd = StoredAmmo < MaxAmmoInMag ? FMath::Min(StoredAmmo, MaxAmmoInMag - BulletsInMag) :
	MaxAmmoInMag - BulletsInMag;
	BulletsInMag += BulletsToAdd;
	WeaponSystem->AddAmmoByType(Internal_FireWeaponData.AmmoMode, -BulletsToAdd);
	if (GetWeaponComponentInterface())
	{
		GetWeaponComponentInterface()->OnCharacterFinishReload();
	}
}

void AFNRFireWeapon::EndReloadByAnim(const int Quantity)
{
	const int StoredAmmo = WeaponSystem->GetAmmoByType(Internal_FireWeaponData.AmmoMode);
	const AFNRAttachment* AmmoAttachment = AttachmentComponent->GetAttachmentByType(EAttachmentType::Ammo);
	const int MaxAmmoInMag = AmmoAttachment ? Internal_FireWeaponData.MaxAmmoInMag + AmmoAttachment->Attachment->AddToMaxBullets : Internal_FireWeaponData.MaxAmmoInMag;
	const int BulletsToAdd = StoredAmmo < Quantity ? FMath::Min(StoredAmmo, MaxAmmoInMag - BulletsInMag) :
	FMath::Min(Quantity, MaxAmmoInMag - BulletsInMag);
	BulletsInMag += BulletsToAdd;
	WeaponSystem->AddAmmoByType(Internal_FireWeaponData.AmmoMode, -BulletsToAdd);
	GetWeaponComponentInterface()->OnCharacterFinishReload();
}

#pragma endregion Replicated Functions