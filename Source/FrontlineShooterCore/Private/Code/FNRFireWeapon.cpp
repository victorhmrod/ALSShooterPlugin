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
#include "Code/FNRAttachmentComponent.h"
#include "Code/FNRCartridgeProjectile.h"
#include "Code/FNRRocketProjectile.h"
#include "Components/ArrowComponent.h"
#include "Core/InteractableComponent.h"
#include "Data/FNRAttachmentDataAsset.h"
#include "Data/FNRFireWeaponData.h"
#include "Net/Core/PushModel/PushModel.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Logging/StructuredLog.h"

#pragma region Unreal Defaults

// Constructor for the AFNRFireWeapon class
AFNRFireWeapon::AFNRFireWeapon()
{
    // Enable ticking for the actor (it will be updated every frame)
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize the attachment component (for weapon attachments like scopes, etc.)
    AttachmentComponent = CreateDefaultSubobject<UFNRAttachmentComponent>(FName{TEXTVIEW("AttachmentComponent")});
    
    // Initialize the magazine mesh component (for the visual representation of the magazine)
    MagazineComponent = CreateDefaultSubobject<UStaticMeshComponent>(FName{TEXTVIEW("MagazineMesh")});
    
    // Set up the magazine component replication, so it's synchronized across clients
    MagazineComponent->SetIsReplicated(true);
    
    // Attach the magazine component to the main mesh component (the body of the weapon)
    MagazineComponent->SetupAttachment(MeshComponent);
    
    // Disable collisions for the magazine component (it won't interact with the environment physically)
    MagazineComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    // Disable decals for the magazine component (it won’t receive any decals like bullet holes)
    MagazineComponent->SetReceivesDecals(false);
    
    // Initialize the ironsight location component (for camera alignment when aiming)
    IronsightLocationComponent = CreateDefaultSubobject<UArrowComponent>(FName{TEXTVIEW("IronsightCamera")});
    
    // Attach the ironsight location component to the weapon's mesh
    IronsightLocationComponent->SetupAttachment(MeshComponent);
    
    // Initialize the recoil animation component (for managing recoil effects)
    RecoilAnimationComponent = CreateDefaultSubobject<URecoilAnimationComponent>(FName{TEXTVIEW("RecoilAnimationComponent")});
    
    // Initialize the rotating movement component (for rotating the weapon)
    RotatingMovementComponent = CreateDefaultSubobject<URotatingMovementComponent>(FName{TEXTVIEW("RotatingMovementComponent")});
    
    // Set the rotation speed for the rotating movement component (90 degrees per second)
    RotatingMovementComponent->RotationRate = FRotator(0, 90.f, 0);
    
    // Initially disable the rotating movement component (it won't rotate unless activated)
    RotatingMovementComponent->SetAutoActivate(false);
}

// Called every frame to update the weapon (for example, to apply recoil)
void AFNRFireWeapon::Tick(const float DeltaSeconds)
{
    // Call the parent class's Tick function
    Super::Tick(DeltaSeconds);
    
    // Update recoil offset based on the time that has passed
    RefreshRecoilOffset(DeltaSeconds);
}

// Called when the weapon is first created or spawned
void AFNRFireWeapon::BeginPlay()
{
    // Call the parent class's BeginPlay function
    Super::BeginPlay();

    // Check if the weapon data asset is set (this holds essential data for the weapon)
    if (!IsValid(WeaponDataAsset))
    {
#if WITH_EDITOR
        // If running in the editor, display an error message if the weapon data asset is missing
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("You forgot to put the data asset on this weapon: %s"), *GetName()), true, true, FLinearColor::Green, 25.f);
#endif

        // Log an error message if the weapon data asset is missing
        UE_LOGFMT(LogFSC, Display, "You forgot to put the data asset on this weapon: %s", *GetName());
    }
    else
    {
    	// If running in the editor, update every time the strucure
#if WITH_EDITOR

    	// Load the internal fire weapon data from the weapon data asset
    	Internal_FireWeaponData = WeaponDataAsset->AsStructure(WeaponDataAsset);
#endif
    }

    // If the weapon is running on the server (i.e., authority), enable physics simulation on the mesh
    if (HasAuthority())
    {
        MeshComponent->SetSimulatePhysics(SimulatePhysics);
    }

    // If the weapon is currently in the "In Ads" state, remove that state (likely means not aiming down sights)
    if (WeaponState.HasTag(FscWeaponStateTags::InAds))
    {
        WeaponState.RemoveTag(FscWeaponStateTags::InAds);
    }

    // Set the initial fire mode based on the data asset
    CurrentFireMode = Internal_FireWeaponData.StartFireMode;

    // Initialize the number of bullets in the magazine, ensuring it doesn’t exceed the maximum allowed
    BulletsInMag = FMath::Min(Internal_FireWeaponData.InitialMagAmmo, Internal_FireWeaponData.MaxAmmoInMag);

    // Initialize the recoil animation component with relevant data (e.g., character animation, fire rate, etc.)
    RecoilAnimationComponent->Init(
        Internal_FireWeaponData.CharacterFireAnim, 
        Internal_FireWeaponData.FireRate, 
        Internal_FireWeaponData.BulletsPerShoot
    );

    // Set the fire mode for the recoil animation component
    RecoilAnimationComponent->SetFireMode(Internal_FireWeaponData.StartFireMode);

    // If rotation is not enabled, deactivate the rotating movement component
    RotatingMovementComponent->SetActive(bEnableRotation, false);

    // Update the tooltip for the interactable component (displaying the number of bullets in the magazine)
    InteractableComponent->SetTooltipText(FString::FromInt(BulletsInMag));
}

// Pre-initialize components before the game starts
void AFNRFireWeapon::PreInitializeComponents()
{
    // Call the parent class's pre-initialize function
    Super::PreInitializeComponents();
}

// Set up replication for networked gameplay (e.g., synchronizing weapon properties between server and clients)
void AFNRFireWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    // Call the parent class's function to replicate properties
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    // Replicate relevant properties for network synchronization
    DOREPLIFETIME(AFNRFireWeapon, CharacterOwner);
    DOREPLIFETIME(AFNRFireWeapon, WeaponSystem);
    DOREPLIFETIME(AFNRFireWeapon, SimulatePhysics);
    DOREPLIFETIME(AFNRFireWeapon, RecoilOffset);
    DOREPLIFETIME(AFNRFireWeapon, WeaponState);
    
    // Replicate the bullet data with specific conditions for better network performance
    FDoRepLifetimeParams BulletParameters;
    BulletParameters.bIsPushBased = true;  // Use push-based replication
    BulletParameters.Condition = COND_SkipOwner;  // Don’t replicate to the owner
    BulletParameters.RepNotifyCondition = REPNOTIFY_Always;  // Always notify of changes
    DOREPLIFETIME_WITH_PARAMS_FAST(AFNRFireWeapon, CartridgeSpawnData, BulletParameters);
    DOREPLIFETIME_WITH_PARAMS_FAST(AFNRFireWeapon, RocketSpawnData, BulletParameters);
}

#pragma endregion Unreal Defaults

#pragma region CPP Only

// Get all meshes that should be glowable (e.g., highlighted in certain states)
TArray<UMeshComponent*> AFNRFireWeapon::GetGlowableMeshes() const
{
    // Initialize an array to store glowable meshes
    TArray<UMeshComponent*> FoundArray;
    
    // Add the magazine component (it could be glowable in certain contexts)
    FoundArray.Add(MagazineComponent);
    
    // Append glowable meshes from the parent class
    FoundArray.Append(Super::GetGlowableMeshes());
    
    // Return the complete array of glowable meshes
    return FoundArray;
}

// Read values from the weapon data asset and apply them to the weapon components
void AFNRFireWeapon::ReadValues()
{
    // Call the base class's ReadValues function to read any common values
    Super::ReadValues();
    
    // If the WeaponDataAsset is invalid, exit early
    if (!IsValid(WeaponDataAsset)) return;

    // Load the internal fire weapon data from the weapon data asset
    Internal_FireWeaponData = WeaponDataAsset->AsStructure(WeaponDataAsset);

    // Attach the ironsight location component to the mesh at the specified aiming socket
    IronsightLocationComponent->AttachToComponent(MeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, Internal_FireWeaponData.AimingCameraSocket);
    
    // Attach the magazine component to the mesh at the specified magazine socket
    MagazineComponent->AttachToComponent(MeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, Internal_FireWeaponData.MagazineSocket);

    // Check if the fire sound is valid, and if so, load it into the LoadedFireSound variable
    if (IsValid(Internal_FireWeaponData.FireSound.LoadSynchronous()))
    {
        LoadedFireSound = Internal_FireWeaponData.FireSound.Get();
    }

    // Check if the magazine mesh is valid, and if so, set it on the MagazineComponent
    if (IsValid(Internal_FireWeaponData.WeaponMagazineMesh.LoadSynchronous()))
    {
        MagazineComponent->SetStaticMesh(Internal_FireWeaponData.WeaponMagazineMesh.Get());
    }

    if (IsValid(Internal_FireWeaponData.FireSoundAttenuation.LoadSynchronous()))
    {
    	LoadedAttenuationSound =Internal_FireWeaponData.FireSoundAttenuation.Get();
    }

    // Check if the projectile material is valid, and if so, load it into LoadedProjectileMaterial
    if (IsValid(Internal_FireWeaponData.ProjectileMaterial.LoadSynchronous()))
    {
        LoadedProjectileMaterial = Internal_FireWeaponData.ProjectileMaterial.Get();
    }

    // Check if the Niagara particle system for the projectile is valid, and if so, load it
    if (IsValid(Internal_FireWeaponData.ProjectileNiagaraParticle.LoadSynchronous()))
    {
        LoadedProjectileNiagaraParticle = Internal_FireWeaponData.ProjectileNiagaraParticle.Get();
    }

    // Load the projectile material again if it's valid (this is a duplicate check)
    if (IsValid(Internal_FireWeaponData.ProjectileMaterial.LoadSynchronous()))
    {
        LoadedProjectileMaterial = Internal_FireWeaponData.ProjectileMaterial.Get();
    }

    // Check if the cartridge projectile class is valid, and if so, load it
    if (IsValid(CartridgeProjectileClass.LoadSynchronous()))
    {
        LoadedCartridgeProjectileClass = CartridgeProjectileClass.Get();
    }

    // Check if the rocket projectile class is valid, and if so, load it
    if (IsValid(RocketProjectileClass.LoadSynchronous()))
    {
        LoadedRocketProjectileClass = RocketProjectileClass.Get();
    }

    // Check if the reload animation for the character is valid, and if so, load it
    if (IsValid(Internal_FireWeaponData.CharacterReloadMontage.LoadSynchronous()))
    {
        LoadedReloadCharacterMontage = Internal_FireWeaponData.CharacterReloadMontage.Get();
    }

    // Check if the empty magazine sound is valid, and if so, load it
    if (IsValid(Internal_FireWeaponData.EmptyMagSound.LoadSynchronous()))
    {
        LoadedEmptyMagSound = Internal_FireWeaponData.EmptyMagSound.Get();
    }

    // Check if the silenced fire sound is valid, and if so, load it
    if (IsValid(Internal_FireWeaponData.SilencedFireSound.LoadSynchronous()))
    {
        LoadedSilencedSound = Internal_FireWeaponData.SilencedFireSound.Get();
    }

    // Check if the muzzle flash particle system (cascade) is valid, and if so, load it
    if (IsValid(Internal_FireWeaponData.MuzzleFlashCascadeParticle.LoadSynchronous()))
    {
        LoadedCascadedParticle = Internal_FireWeaponData.MuzzleFlashCascadeParticle.Get();
    }

    // Check if the weapon magazine mesh is valid (again), and if so, load it
    if (IsValid(Internal_FireWeaponData.WeaponMagazineMesh.LoadSynchronous()))
    {
        LoadedMagazineMesh = Internal_FireWeaponData.WeaponMagazineMesh.Get();
    }

    // Check if the reload animation for the weapon is valid, and if so, load it
    if (IsValid(Internal_FireWeaponData.WeaponReloadAnim.LoadSynchronous()))
    {
        LoadedWeaponReloadAnim = Internal_FireWeaponData.WeaponReloadAnim.Get();
    }

    // Check if the muzzle flash particle system (Niagara) is valid, and if so, load it
    if (IsValid(Internal_FireWeaponData.MuzzleFlashNiagaraParticle.LoadSynchronous()))
    {
        LoadedNiagaraParticle = Internal_FireWeaponData.MuzzleFlashNiagaraParticle.Get();
    }
}

void AFNRFireWeapon::RefreshRecoilOffset(const float DeltaTime)
{
    // Get the current recoil offset from the animation component
    RecoilOffset = RecoilAnimationComponent->GetOutput();

    // Check if recoil is being applied
    if (bApplyingRecoil)
    {
    	RecoilTime += Internal_FireWeaponData.RecoilTimeVelocity;
        
        // Get the recoil values from the curves at the correct time
        float BasePitchRecoil = Internal_FireWeaponData.RecoilPitchCurve->GetFloatValue(RecoilTime);
        float BaseYawRecoil = Internal_FireWeaponData.RecoilYawCurve->GetFloatValue(RecoilTime);
        
        // Apply attachment multipliers (barrel, stock) to the recoil values
        const auto& Barrel = AttachmentComponent->GetAttachmentByType(EAttachmentType::Barrel);
        if (Barrel)
        {
            // Multiply the pitch and yaw recoil based on the barrel attachment's multipliers
            BasePitchRecoil *= Barrel->AttachmentDataAsset->BarrelVRecoilMultiplier;
            BaseYawRecoil *= Barrel->AttachmentDataAsset->BarrelHRecoilMultiplier;
        }
        const auto& Stock = AttachmentComponent->GetAttachmentByType(EAttachmentType::Stock);
        if (Stock)
        {
            // Multiply the pitch and yaw recoil based on the stock attachment's multipliers
            BasePitchRecoil *= Stock->AttachmentDataAsset->BarrelVRecoilMultiplier;
            BaseYawRecoil *= Stock->AttachmentDataAsset->BarrelHRecoilMultiplier;
        }

        // Apply recoil multipliers for the character's weapon component interface (affects both pitch and yaw)
        const float RecoilMultiplier = GetWeaponComponentInterface()->Execute_GetRecoilMultiplier(CharacterOwner);
        
        // Smoothly interpolate the current pitch recoil with standard velocity
        CurrentPitchRecoil = FMath::FInterpTo(CurrentPitchRecoil, BasePitchRecoil, DeltaTime, Internal_FireWeaponData.RecoilVelocity);
        
        // Avoid excessive smoothing on yaw recoil to follow the curve more closely
        // Apply yaw recoil directly to avoid over-smoothing
        CurrentYawRecoil = BaseYawRecoil; // Direct application of recoil from the curve

        // Apply the recoil to the character's controller input (multiplied by recoil multipliers)
        CharacterOwner->AddControllerPitchInput(CurrentPitchRecoil * RecoilMultiplier);
        CharacterOwner->AddControllerYawInput(CurrentYawRecoil * RecoilMultiplier);
    }
    else
    {
        // Reset recoil time when no recoil is being applied
        RecoilTime = 0.f;
    	CurrentPitchRecoil = 0.f;
    	CurrentYawRecoil = 0.f;
    }
}

// Auto-fire function for continuous fire
bool AFNRFireWeapon::AutoFire()
{
    // Check if the weapon is ready to fire
    if (bWantsFire && WeaponState.HasTag(FscWeaponStateTags::CanFire) && !bIsReloading && !bInFireRateDelay && HasAmmo())
    {
        // Notify server that the weapon is firing
        Server_SetIsFiring(true);
        bInFireRateDelay = true;

        // If locally controlled, decrement the bullets in the magazine
        if (CharacterOwner->IsLocallyControlled())
        {
            --BulletsInMag;
        }

        // Fire multiple bullets for burst fire
        for (int i = 0; i < Internal_FireWeaponData.BulletsPerShoot; i++)
        {
            K2_Fire();
            const FHitResult HitResult = FireTrace();  // Trace the shot
            const FVector MuzzleLocation = MeshComponent->GetSocketLocation(GetGeneralData().MuzzleSocket);
            const FRotator ProjectileRotation = GetSpread(UKismetMathLibrary::FindLookAtRotation(MuzzleLocation, HitResult.bBlockingHit ? HitResult.ImpactPoint : HitResult.TraceEnd), 0);

            FireProjectile(FTransform{ProjectileRotation, MuzzleLocation, FVector::One()});  // Fire the projectile
            WeaponSystem->OnFire.Broadcast();  // Trigger fire event
        }

        // Play firing animations
        MeshComponent->PlayAnimation(LoadedWeaponFireAnim, false);
        RecoilAnimationComponent->Play();

        // Set a timer for the fire rate delay
        GetWorldTimerManager().SetTimer(InFireRateTimerHandle, [this]
        {
            bInFireRateDelay = false;
            AutoFire();  // Call AutoFire again after delay
        }, GetFireRate(), false);

        // Enable recoil during firing
        bApplyingRecoil = true;

        return true;
    }

    // If not firing or out of ammo, play empty magazine sound
    if (!bWantsFire && !WeaponState.HasTag(FscWeaponStateTags::CanFire) && !HasAmmo())
    {
        UGameplayStatics::PlaySoundAtLocation(this, LoadedEmptyMagSound, MeshComponent->GetSocketLocation(GetGeneralData().MuzzleSocket), FRotator::ZeroRotator, 0.5f);
    }

    // Stop firing and recoil animations
    bApplyingRecoil = false;
    RecoilTime = 0.0f;
    Server_SetIsFiring(false);
    RecoilAnimationComponent->Stop();
    GetWeaponComponentInterface()->Execute_OnWeaponFiredOnce(CharacterOwner, false);

    return false;
}

// Semi-fire function for single shots
bool AFNRFireWeapon::SemiFire()
{
    // Check if the weapon is ready to fire
    if (bWantsFire && WeaponState.HasTag(FscWeaponStateTags::CanFire) && !bIsReloading && !bInFireRateDelay && HasAmmo())
    {
        // Notify server that the weapon is firing
        Server_SetIsFiring(true);
        bInFireRateDelay = true;

        // If locally controlled, decrement the bullets in the magazine
        if (CharacterOwner->IsLocallyControlled())
        {
            --BulletsInMag;
        }

        // Fire a single bullet for semi-auto fire
        for (int i = 0; i < Internal_FireWeaponData.BulletsPerShoot; i++)
        {
            K2_Fire();
            const FHitResult HitResult = FireTrace();  // Trace the shot
            const FVector MuzzleLocation = MeshComponent->GetSocketLocation(GetGeneralData().MuzzleSocket);
            const FRotator ProjectileRotation = GetSpread(UKismetMathLibrary::FindLookAtRotation(MuzzleLocation, HitResult.bBlockingHit ? HitResult.ImpactPoint : HitResult.TraceEnd), 0);

            FireProjectile(FTransform{ProjectileRotation, MuzzleLocation, FVector::One()});  // Fire the projectile
            WeaponSystem->OnFire.Broadcast();  // Trigger fire event
        }

        // Play firing animations
        MeshComponent->PlayAnimation(LoadedWeaponFireAnim, false);
        RecoilAnimationComponent->Play();

        // Set a timer for the fire rate delay
        GetWorldTimerManager().SetTimer(InFireRateTimerHandle, [this]()
        {
            bInFireRateDelay = false;
            Server_SetIsFiring(false);
            bApplyingRecoil = false;
        }, GetFireRate(), false);

        // Enable recoil during firing
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

    // Stop firing and recoil animations
    Server_SetIsFiring(false);
    RecoilAnimationComponent->Stop();
    GetWeaponComponentInterface()->Execute_OnWeaponFiredOnce(CharacterOwner, false);
    bApplyingRecoil = false;
    RecoilTime = 0.0f;

    return false;
}

// Burst-fire function for rapid-fire shots
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

    // Handle burst fire logic with a timer
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
    MeshComponent->PlayAnimation(LoadedWeaponFireAnim, false);

    // Set delay for fire rate control
    GetWorldTimerManager().SetTimer(InFireRateTimerHandle, [this]()
    {
        bInFireRateDelay = false;
    }, GetFireRate(), false);

    return true;
}

// Perform a trace to detect hits from the weapon's fire
FHitResult AFNRFireWeapon::FireTrace()
{
	FHitResult HitResult;
	// Check if the CharacterOwner is valid
	if (!IsValid(CharacterOwner))
	{
		return HitResult;
	}

	FVector CameraLocation{};
	FRotator CameraRotation{};

	// Get the view point based on the character's control type (bot or player)
	if (CharacterOwner->IsBotControlled() || !CharacterOwner->IsPlayerControlled())
	{
		CharacterOwner->GetActorEyesViewPoint(CameraLocation, CameraRotation);
	}
	else
	{
		CameraLocation = GetWeaponComponentInterface()->GetCameraTransform().GetLocation();
		CameraRotation = GetWeaponComponentInterface()->GetCameraTransform().Rotator();
	}

	// Calculate the distance between the origin point (weapon muzzle) and the end point of the trace line
	const FVector StartLocation = CameraLocation + CameraRotation.Vector() * (CameraLocation - MeshComponent->GetSocketLocation(GetGeneralData().MuzzleSocket)).Length();
	const FVector EndLocation = StartLocation + CameraRotation.Vector() * 100000.0f;

	// Define the actors to ignore during the trace (like the character and weapon)
	const TArray<AActor*> ActorsToIgnore{this, CharacterOwner};

	// Set the trace channel and debug options
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActors(ActorsToIgnore);
	QueryParams.bTraceComplex = true; // Perform more detailed collision tracing, if needed
	QueryParams.bReturnPhysicalMaterial = false;

	// Perform the line trace
	UKismetSystemLibrary::LineTraceSingle(
		this, StartLocation, EndLocation, 
		UEngineTypes::ConvertToTraceType(WeaponSystem->TraceChannel), 
		false, ActorsToIgnore, 
		WeaponSystem->GetDrawDebugType(), 
		HitResult, true
	);
	
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
			FRotator::ZeroRotator, 1.f, 1.f, 0.f, LoadedAttenuationSound ? LoadedAttenuationSound : nullptr);
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
	return AttachmentComponent->GetAttachmentByType(EAttachmentType::Bolt) ? 60 / (Internal_FireWeaponData.FireRate * AttachmentComponent->GetAttachmentByType(EAttachmentType::Bolt)->AttachmentDataAsset->FireRateMultiplier) : 60 / Internal_FireWeaponData.FireRate;
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
			ProjectileRef->Damage *= Barrel->AttachmentDataAsset->DamageMultiplier;
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
			ProjectileRef->Damage *= Barrel->AttachmentDataAsset->DamageMultiplier;
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
	const int MaxAmmoInMag = AmmoAttachment ? Internal_FireWeaponData.MaxAmmoInMag + AmmoAttachment->AttachmentDataAsset->AddToMaxBullets : Internal_FireWeaponData.MaxAmmoInMag;
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
	const int MaxAmmoInMag = AmmoAttachment ? Internal_FireWeaponData.MaxAmmoInMag + AmmoAttachment->AttachmentDataAsset->AddToMaxBullets : Internal_FireWeaponData.MaxAmmoInMag;
	const int BulletsToAdd = StoredAmmo < Quantity ? FMath::Min(StoredAmmo, MaxAmmoInMag - BulletsInMag) :
	FMath::Min(Quantity, MaxAmmoInMag - BulletsInMag);
	BulletsInMag += BulletsToAdd;
	WeaponSystem->AddAmmoByType(Internal_FireWeaponData.AmmoMode, -BulletsToAdd);
	GetWeaponComponentInterface()->OnCharacterFinishReload();
}

#pragma endregion Replicated Functions
