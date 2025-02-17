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

void AFNRFireWeapon::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	RefreshRecoilOffset(DeltaSeconds);
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

void AFNRFireWeapon::RefreshRecoilOffset(const float DeltaTime)
{
    // Get the current recoil offset from the animation component
    RecoilOffset = RecoilAnimationComponent->GetOutput();

    // Check if recoil is being applied
    if (bApplyingRecoil)
    {
	    // Update detection timer
    	LastRecoilUpdate += DeltaTime;

    	// Perform detection check at defined intervals
    	if (LastRecoilUpdate >= RecoilUpdateDelay)
    	{
    		LastRecoilUpdate = 0.f;
    		RecoilTime += Internal_FireWeaponData.RecoilTimeVelocity;
    	}
    	
    	// Get the recoil values from the curves at the correct time
    	float BasePitchRecoil = Internal_FireWeaponData.RecoilPitchCurve->GetFloatValue(RecoilTime);
    	float BaseYawRecoil = Internal_FireWeaponData.RecoilYawCurve->GetFloatValue(RecoilTime);
    	
    	// Apply attachment multipliers (barrel, stock) to the recoil values
    	if (const auto& Barrel = AttachmentComponent->GetAttachmentByType(EAttachmentType::Barrel))
    	{
    		// Multiply the pitch and yaw recoil based on the barrel attachment's multipliers
    		BasePitchRecoil *= Barrel->Attachment->BarrelVRecoilMultiplier;
    		BaseYawRecoil *= Barrel->Attachment->BarrelHRecoilMultiplier;
    	}
    	if (const auto& Stock = AttachmentComponent->GetAttachmentByType(EAttachmentType::Stock))
    	{
    		// Multiply the pitch and yaw recoil based on the stock attachment's multipliers
    		BasePitchRecoil *= Stock->Attachment->BarrelVRecoilMultiplier;
    		BaseYawRecoil *= Stock->Attachment->BarrelHRecoilMultiplier;
    	}

    	// Smoothly interpolate the current pitch and yaw recoil towards the base recoil values
    	// Note: Do not excessively smooth the yaw recoil for a more natural feel
    	CurrentPitchRecoil = FMath::FInterpTo(CurrentPitchRecoil, BasePitchRecoil, DeltaTime, Internal_FireWeaponData.RecoilVelocity);
    	CurrentYawRecoil = FMath::FInterpTo(CurrentYawRecoil, BaseYawRecoil, DeltaTime, Internal_FireWeaponData.RecoilVelocity);

    	// Apply the recoil to the character's controller input
    	CharacterOwner->AddControllerPitchInput(CurrentPitchRecoil * GetWeaponComponentInterface()->Execute_GetSpreadMultiplier(CharacterOwner));
    	CharacterOwner->AddControllerYawInput(CurrentYawRecoil * GetWeaponComponentInterface()->Execute_GetSpreadMultiplier(CharacterOwner));
    }
}

bool AFNRFireWeapon::AutoFire()
{
    // Check if weapon is ready to fire
    if (bWantsFire && WeaponState.HasTag(FscWeaponStateTags::CanFire) && !bIsReloading && !bInFireRateDelay && HasAmmo())
    {
        Server_SetIsFiring(true);
        bInFireRateDelay = true;

        // If locally controlled, decrement the bullets in the magazine
        if (CharacterOwner->IsLocallyControlled())
        {
            --BulletsInMag;
        }

        // Fire multiple bullets (burst fire logic)
        for (int i = 0; i < Internal_FireWeaponData.BulletsPerShoot; i++)
        {
            K2_Fire();
            const FHitResult HitResult = FireTrace();
            const FVector MuzzleLocation = MeshComponent->GetSocketLocation(GetGeneralData().MuzzleSocket);
            const FRotator ProjectileRotation = GetSpread(UKismetMathLibrary::FindLookAtRotation(MuzzleLocation, HitResult.bBlockingHit ? HitResult.ImpactPoint : HitResult.TraceEnd), 0);

            FireProjectile(FTransform{ProjectileRotation, MuzzleLocation, FVector::One()});
            WeaponSystem->OnFire.Broadcast();
        }

        // Play firing animations
        MeshComponent->PlayAnimation(LoadedWeaponFireAnim, false);
        RecoilAnimationComponent->Play();

        // Handle baked fire animation if necessary
        if (Internal_FireWeaponData.FireAnimationMode.HasTag(FscFireAnimationModeTags::UseBakedFireAnimation))
        {
            WeaponSystem->PlayMontage_Server(LoadedFireCharacterMontage);
        }

        // Set a timer for the fire rate delay
        GetWorldTimerManager().SetTimer(InFireRateTimerHandle, [this]
        {
            bInFireRateDelay = false;
            AutoFire(); // Call AutoFire again if the timer completes
        }, GetFireRate(), false);

        // Set recoil applying to true when firing
        bApplyingRecoil = true;

        return true;
    }

    // If not firing or out of ammo, play empty magazine sound
    if (!bWantsFire && !WeaponState.HasTag(FscWeaponStateTags::CanFire) && !HasAmmo())
    {
        UGameplayStatics::PlaySoundAtLocation(this, LoadedEmptyMagSound, MeshComponent->GetSocketLocation(GetGeneralData().MuzzleSocket), FRotator::ZeroRotator, 0.5f);
    }

    // Stop firing and recoil animation
    bApplyingRecoil = false;
    RecoilTime = 0.0f;
    Server_SetIsFiring(false);
    RecoilAnimationComponent->Stop();
    GetWeaponComponentInterface()->Execute_OnWeaponFiredOnce(CharacterOwner, false);
    return false;
}

bool AFNRFireWeapon::SemiFire()
{
    // Check if weapon is ready to fire
    if (bWantsFire && WeaponState.HasTag(FscWeaponStateTags::CanFire) && !bIsReloading && !bInFireRateDelay && HasAmmo())
    {
        Server_SetIsFiring(true);
        bInFireRateDelay = true;

        // If locally controlled, decrement the bullets in the magazine
        if (CharacterOwner->IsLocallyControlled())
        {
            --BulletsInMag;
        }

        // Fire a single bullet (semi-fire logic)
        for (int i = 0; i < Internal_FireWeaponData.BulletsPerShoot; i++)
        {
            K2_Fire();
            const FHitResult HitResult = FireTrace();
            const FVector MuzzleLocation = MeshComponent->GetSocketLocation(GetGeneralData().MuzzleSocket);
            const FRotator ProjectileRotation = GetSpread(UKismetMathLibrary::FindLookAtRotation(MuzzleLocation, HitResult.bBlockingHit ? HitResult.ImpactPoint : HitResult.TraceEnd), 0);

            FireProjectile(FTransform{ProjectileRotation, MuzzleLocation, FVector::One()});
            WeaponSystem->OnFire.Broadcast();
        }

        // Play firing animations
        MeshComponent->PlayAnimation(LoadedWeaponFireAnim, false);
        RecoilAnimationComponent->Play();

        // Handle baked fire animation if necessary
        if (Internal_FireWeaponData.FireAnimationMode.HasTag(FscFireAnimationModeTags::UseBakedFireAnimation))
        {
            WeaponSystem->PlayMontage_Server(LoadedFireCharacterMontage);
        }

        // Set a timer for the fire rate delay
        GetWorldTimerManager().SetTimer(InFireRateTimerHandle, [this]()
        {
            bInFireRateDelay = false;
            Server_SetIsFiring(false);
        	bApplyingRecoil = false;
        }, GetFireRate(), false);

        // Set recoil applying to true when firing
        bApplyingRecoil = true;

        // Notify that firing occurred
        GetWeaponComponentInterface()->Execute_OnWeaponFiredOnce(CharacterOwner, true);
        return true;
    }

    // If not firing or out of ammo, play empty magazine sound
    if (!bWantsFire && !WeaponState.HasTag(FscWeaponStateTags::CanFire) && !HasAmmo())
    {
        UGameplayStatics::PlaySoundAtLocation(this, LoadedEmptyMagSound, MeshComponent->GetSocketLocation(GetGeneralData().MuzzleSocket));
    }

    // Stop firing and recoil animation
    Server_SetIsFiring(false);
    RecoilAnimationComponent->Stop();
    GetWeaponComponentInterface()->Execute_OnWeaponFiredOnce(CharacterOwner, false);
    bApplyingRecoil = false;
    RecoilTime = 0.0f;
    return false;
}

bool AFNRFireWeapon::BurstFire()
{
    // If a fire timer already exists, stop firing and reset recoil
    if (GetWorldTimerManager().TimerExists(FireWithDelayTimerHandle))
    {
        Server_SetIsFiring(false);
        GetWeaponComponentInterface()->Execute_OnWeaponFiredOnce(CharacterOwner, false);
        bApplyingRecoil = false;
        RecoilTime = 0.0f;
        return false;
    }

    // Start applying recoil when firing
    bApplyingRecoil = true;

    // Reduce bullets in the magazine if the player is local
    if (CharacterOwner->IsLocallyControlled())
    {
        --BulletsInMag;
    }

    // Trigger the weapon fire event
    GetWeaponComponentInterface()->Execute_OnWeaponFiredOnce(CharacterOwner, true);
    K2_Fire();
    
    // Perform a fire trace to determine the hit result
    const FHitResult HitResult = FireTrace();
    const FVector MuzzleLocation = MeshComponent->GetSocketLocation(GetGeneralData().MuzzleSocket);
    const FRotator ProjectileRotation = GetSpread(
        UKismetMathLibrary::FindLookAtRotation(MuzzleLocation, HitResult.bBlockingHit ? HitResult.ImpactPoint : HitResult.TraceEnd), 0);

    // Fire the projectile
    FireProjectile(FTransform{ProjectileRotation, MuzzleLocation, FVector::One()});

    // Broadcast the fire event
    WeaponSystem->OnFire.Broadcast();

    // Set the weapon to firing state
    Server_SetIsFiring(true);
    BulletsRemaining = Internal_FireWeaponData.BulletsPerShoot - 1;

    // Handle burst fire logic
    GetWorldTimerManager().SetTimer(FireWithDelayTimerHandle, [this]
    {
        if (BulletsInMag > 0 && BulletsRemaining > 0)
        {
            Server_SetIsFiring(true);
            GetWeaponComponentInterface()->Execute_OnWeaponFiredOnce(CharacterOwner, true);
            K2_Fire();

            // Perform another fire trace
            const FHitResult HitResult = FireTrace();
            const FVector MuzzleLocation = MeshComponent->GetSocketLocation(GetGeneralData().MuzzleSocket);
            const FRotator ProjectileRotation = GetSpread(
                UKismetMathLibrary::FindLookAtRotation(MuzzleLocation, HitResult.bBlockingHit ? HitResult.ImpactPoint : HitResult.TraceEnd), 0);

            // Fire the projectile
            FireProjectile(FTransform{ProjectileRotation, MuzzleLocation, FVector::One()});

            // Reduce bullets in the magazine if the player is local
            if (CharacterOwner->IsLocallyControlled())
            {
                --BulletsInMag;
            }

            // Broadcast fire event and update remaining bullets
            WeaponSystem->OnFire.Broadcast();
            --BulletsRemaining;
        }
        else
        {
            // Stop firing when out of bullets
            GetWorldTimerManager().ClearTimer(FireWithDelayTimerHandle);
            Server_SetIsFiring(false);
        }
    }, Internal_FireWeaponData.DelayBetweenBullets, true);

    // Play fire animation
    RecoilAnimationComponent->Play();
    if (Internal_FireWeaponData.FireAnimationMode.HasTag(FscFireAnimationModeTags::UseBakedFireAnimation))
    {
        WeaponSystem->PlayMontage_Server(LoadedFireCharacterMontage);
    }
    MeshComponent->PlayAnimation(LoadedWeaponFireAnim, false);

    // Set delay for fire rate control
    GetWorldTimerManager().SetTimer(InFireRateTimerHandle, [this]()
    {
        bInFireRateDelay = false;
    }, GetFireRate(), false);

    return true;
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
	bIsFiring = bFiring;
	Multicast_SetIsFiring(bFiring);
}

void AFNRFireWeapon::Multicast_SetIsFiring_Implementation(const bool bFiring)
{
	bIsFiring = bFiring;

	if (bIsFiring)
	{
		// Determine the scaling factor based on ADS (Aim Down Sight)
		const float MuzzleFlashScale = WeaponState.HasTag(FscWeaponStateTags::InAds) ? 0.5f : 1.0f;

		// Check if both Cascade and Niagara effects should be used
		if (Internal_FireWeaponData.MuzzleFlashMode.HasTag(FscMuzzleFlashModeTags::UseCascade) && 
			Internal_FireWeaponData.MuzzleFlashMode.HasTag(FscMuzzleFlashModeTags::UseNiagara))
		{
			// Spawn Cascade Particle if available
			if (LoadedCascadedParticle)
			{
				UGameplayStatics::SpawnEmitterAttached(
					LoadedCascadedParticle, MeshComponent, GetGeneralData().MuzzleSocket, FVector(ForceInit), FRotator::ZeroRotator,
					Internal_FireWeaponData.MuzzleFlashCascadeParticleScale * MuzzleFlashScale
				);
			}

			// Spawn Niagara Particle if available
			if (LoadedNiagaraParticle)
			{
				UNiagaraFunctionLibrary::SpawnSystemAttached(
					LoadedNiagaraParticle, MeshComponent, GetGeneralData().MuzzleSocket, FVector(0.f), FRotator(0.f),
					Internal_FireWeaponData.MuzzleFlashNiagaraParticleScale * MuzzleFlashScale,
					EAttachLocation::KeepRelativeOffset, true, ENCPoolMethod::AutoRelease
				);
			}
		}
		// Only Niagara
		else if (Internal_FireWeaponData.MuzzleFlashMode.HasTag(FscMuzzleFlashModeTags::UseNiagara))
		{
			if (LoadedNiagaraParticle)
			{
				UNiagaraFunctionLibrary::SpawnSystemAttached(
					LoadedNiagaraParticle, MeshComponent, GetGeneralData().MuzzleSocket, FVector(0.f), FRotator(0.f),
					Internal_FireWeaponData.MuzzleFlashNiagaraParticleScale * MuzzleFlashScale,
					EAttachLocation::KeepRelativeOffset, true, ENCPoolMethod::AutoRelease
				);
			}
		}
		// Only Cascade
		else if (Internal_FireWeaponData.MuzzleFlashMode.HasTag(FscMuzzleFlashModeTags::UseCascade))
		{
			if (LoadedCascadedParticle)
			{
				UGameplayStatics::SpawnEmitterAttached(
					LoadedCascadedParticle, MeshComponent, GetGeneralData().MuzzleSocket, FVector(ForceInit), FRotator::ZeroRotator,
					Internal_FireWeaponData.MuzzleFlashCascadeParticleScale * MuzzleFlashScale
				);
			}
		}

		// Play the correct firing sound
		UGameplayStatics::PlaySoundAtLocation(this, 
			AttachmentComponent->GetAttachmentByType(EAttachmentType::Suppressor) ? LoadedSilencedSound : LoadedFireSound, 
			MeshComponent->GetSocketLocation(GetGeneralData().MuzzleSocket), 
			FRotator::ZeroRotator, 0.2f);
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
		ProjectileRef->MuzzleVelocity = FMath::FRandRange(Internal_FireWeaponData.ProjectileVelocity.X, Internal_FireWeaponData.ProjectileVelocity.Y);
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
