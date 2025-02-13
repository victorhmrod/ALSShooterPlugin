// All rights reserved Wise Labs �

#include "Code/FNRWeaponComponent.h"

#include "FrontlineShooterCore.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "RecoilAnimationComponent.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Code/FNRAttachment.h"
#include "Code/FNRAttachmentWeaponComponent.h"
#include "Code/FNRGrenade.h"
#include "Code/FNRFireWeapon.h"
#include "Code/FNRMeleeWeapon.h"
#include "Code/UI/FNRAttachmentSwitch.h"
#include "Data/Interfaces/FNRWeaponComponentInterface.h"
#include "Core/InteractableComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Components/SplineComponent.h"
#include "Components/WidgetComponent.h"
#include "Data/FNRAttachmentData.h"
#include "Data/FNRGrenadeWeaponData.h"
#include "Interactables/FNROblivion.h"
#include "Interactables/FNROblivionManager.h"
#include "Logging/StructuredLog.h"

#pragma region Unreal Defaults
UFNRWeaponComponent::UFNRWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UFNRWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	ComponentOwnerCharacter = CastChecked<AFNRPlayerCharacter>(GetOwner());
	ComponentOwnerPlayerController = Cast<APlayerController>(ComponentOwnerCharacter->GetController());
	LoadValues();
	if (ComponentOwnerCharacter->IsLocallyControlled())
	{
		PredictSplineComponent = Cast<USplineComponent>(ComponentOwnerCharacter->AddComponentByClass(USplineComponent::StaticClass(), false, FTransform(), false));
		PredictEffectComponent = Cast<UNiagaraComponent>(ComponentOwnerCharacter->AddComponentByClass(UNiagaraComponent::StaticClass(), true, FTransform(), true));
		PredictEffectComponent->SetAsset(GrenadePredictNiagaraEffect.LoadSynchronous());
		PredictEffectComponent->SetVariableActor("Spline", ComponentOwnerCharacter);
		PredictEffectComponent->AttachToComponent(PredictSplineComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
		PredictEffectComponent->SetHiddenInGame(true);
		ComponentOwnerCharacter->FinishAddComponent(PredictEffectComponent, true, FTransform());
	}
	if (ComponentOwnerCharacter->HasAuthority())
	{
		FTimerHandle RandomTimer;
		GetWorld()->GetTimerManager().SetTimer(RandomTimer, [this]
		{
			if (StartGrenades.Num() > 0)
			{
				for (int i = 0; i < StartGrenades.Num(); i++)
				{
					if (i >= MaxGrenadesNum) {break;}
				
					if (StartGrenades[i].IsValid())
					{
						AddWeapon(GetWorld()->SpawnActor<AFNRGrenade>(StartGrenades[i].LoadSynchronous(), ComponentOwnerCharacter->GetActorTransform()));
					}
				}
			}
			if (StartWeapons.Num() > 0)
			{
				for (int i = 0; i < StartWeapons.Num(); i++)
				{
					if (i >= MaxWeaponNum) {break;}
				
					if (StartWeapons[i].IsValid())
					{
						AddWeapon(GetWorld()->SpawnActor<AFNRWeapon>(StartWeapons[i].LoadSynchronous(), ComponentOwnerCharacter->GetActorTransform()));
					}
				}
			}
		}, 1.0f, false);
	}
}

void UFNRWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!IsValid(ComponentOwnerCharacter)) return;
	ResetRecoilRefresh(DeltaTime);
	PredictGrenade();
	if (IsValid(GetFocusedEnemy()) && IsValid(ComponentOwnerPlayerController))
	{
		if (LockedInEnemy && IsValid(ComponentOwnerPlayerController))
		{
			ComponentOwnerPlayerController->SetControlRotation(FMath::RInterpTo(ComponentOwnerPlayerController->GetControlRotation(), UKismetMathLibrary::FindLookAtRotation(ComponentOwnerCharacter->GetActorLocation(), GetFocusedEnemy()->GetActorLocation()), DeltaTime, 2.f));
		}
	}
	else
	{
		LockedInEnemy = false;
	}
	if (GetWeaponComponentInterface())
	{
		GetWeaponComponentInterface()->OnUpdateCombatMode(bInCombatMode);
	}
	if (bInCombatMode && DetectedEnemies.Num() > 0)
	{
		AActor* NearbyActor = nullptr;
		for (const auto& a : DetectedEnemies)
		{
			if (!IsValid(a))
			{
				continue;
			}
			if (!IsValid(NearbyActor) || a->GetDistanceTo(ComponentOwnerCharacter) < NearbyActor->GetDistanceTo(ComponentOwnerCharacter))
			{
				NearbyActor = a;
			}
		}
		if (IsValid(NearbyActor))
		{
			SetFocusEnemy(NearbyActor);
		}
	}
	else
	{
		SetFocusEnemy(nullptr);
	}
}

void UFNRWeaponComponent::ResetRecoilRefresh(float DeltaTime)
{
	if (!IsValid(ComponentOwnerCharacter)) return;
	if (EquippedFireWeapon && bIsInterpolatingRecoil && !bIsInspecting)
	{
		if (!EquippedFireWeapon->HasAmmo())
		{
			LastRecoilApplied = 0.0f;	
			bIsInterpolatingRecoil = false;
			return;
		}
		AController* Controller = ComponentOwnerCharacter->GetController();
		if (Controller)
		{
			const FRotator CurrentRotation = Controller->GetControlRotation();
			const FRotator TargetRotation = FRotator(OriginalPitch - LastRecoilApplied, CurrentRotation.Yaw, CurrentRotation.Roll);
			const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, RecoilInterpSpeed);
			Controller->SetControlRotation(NewRotation);
		}	
		LastRecoilApplied = FMath::FInterpTo(LastRecoilApplied, 0.0f, DeltaTime, RecoilInterpSpeed);
		if (FMath::IsNearlyZero(LastRecoilApplied, 1.0f))
		{
			LastRecoilApplied = 0.0f;
			bIsInterpolatingRecoil = false;
		}
	}
}

void UFNRWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UFNRWeaponComponent, Weapons);
	DOREPLIFETIME(UFNRWeaponComponent, CurrentWeapons);
	DOREPLIFETIME(UFNRWeaponComponent, EquippedWeapon);
	DOREPLIFETIME(UFNRWeaponComponent, EquippedGrenade);
	DOREPLIFETIME(UFNRWeaponComponent, EquippedFireWeapon);
	DOREPLIFETIME(UFNRWeaponComponent, EquippedMeleeWeapon);
	DOREPLIFETIME(UFNRWeaponComponent, EnergyAmmo);
	DOREPLIFETIME(UFNRWeaponComponent, HeavyAmmo);
	DOREPLIFETIME(UFNRWeaponComponent, ShotgunAmmo);
	DOREPLIFETIME(UFNRWeaponComponent, PrecisionAmmo);
	DOREPLIFETIME(UFNRWeaponComponent, LightAmmo);
	DOREPLIFETIME(UFNRWeaponComponent, WeaponsSockets);
	DOREPLIFETIME(UFNRWeaponComponent, CurrentWeaponIndex);	
}

FName UFNRWeaponComponent::GetFirstHolsterSocket(const AFNRWeapon* Weapon) const
{
	FName HolsterSocket = ""; 
	bool bSuccess = false;
	for (const auto& i : Weapon->GetGeneralData().HolsterSocket)
	{
		if (WeaponsSockets.Contains(i.ToString() + Weapon->GetName()))
		{
			UE_LOGFMT(LogFSC, Display, "Exactly founded in Weapon Sockets array");
			HolsterSocket = i;
			bSuccess = true;
			break;
		}
		UE_LOGFMT(LogFSC, Error, "Dont found {0} in Weapon Sockets Array", i.ToString() + Weapon->GetName());
	}
	if (!bSuccess)
	{
		for (const auto& i : Weapon->GetGeneralData().HolsterSocket)
		{
			for (const auto& j: WeaponsSockets)
			{
				if (!j.Contains(i.ToString()))
				{
					UE_LOGFMT(LogFSC, Display, "{0} dont contains in {1}", i.ToString(), j);
					bSuccess = true;
					break;
				}
				UE_LOGFMT(LogFSC, Display, "{0} contains in {1}", i.ToString(), j);
			}
			
			if (bSuccess)
			{
				HolsterSocket = i;
				break;
			}
		}
	}
	return HolsterSocket;
}

#pragma endregion Unreal Defaults

#pragma region CPP Only
void UFNRWeaponComponent::StartSpawnGrenade()
{
	if (IsValid(EquippedGrenade) && bCanThrow)
	{
		UE_LOGFMT(LogFSC, Display, "Start Predicting");

		GetWeaponComponentInterface()->OnStartThrowGrenade();
		bPredictingGrenade = true;
	}
}

void UFNRWeaponComponent::FinishSpawnGrenade(const bool bCancelGrenade)
{
	if (bCanThrow && bPredictingGrenade)
	{
		ComponentOwnerCharacter->GetWorldTimerManager().ClearTimer(GrenadeTimerHandle);
		GetWeaponComponentInterface()->OnEndThrowGrenade(!bCancelGrenade);
		bPredictingGrenade = false;
		if (bCancelGrenade)
		{
			PredictSplineComponent->ClearSplinePoints();
		}
		else
		{
			if (IsValid(PredictSplineComponent))
			{
				PredictSplineComponent->ClearSplinePoints();
			}			
			PredictEffectComponent->SetHiddenInGame(true);
			if (UAnimMontage* ThrowGrenadeMontage = EquippedGrenade->GrenadeData->General.CharacterFireMontage.LoadSynchronous())
			{
				PlayMontage_Server(ThrowGrenadeMontage);
			}
			else
			{
				ThrowGrenade();
			}
		}
		OnEquipOrUnEquipWeapon.Broadcast(EquippedWeapon);
	}
}

void UFNRWeaponComponent::EquipAnotherWeapon(UAnimMontage* AnimMontage)
{
	OnEndAnyMontage.RemoveDynamic(this, &UFNRWeaponComponent::EquipAnotherWeapon);
	if (WeaponToEquipIndex >= 0)
	{
		if (DelayToEquipAnotherWeapon <= 0.0f)
		{
		    ComponentOwnerCharacter->GetWorldTimerManager().SetTimerForNextTick([this] 
		    {
		    	EquipWeapon(WeaponToEquipIndex);
		    	WeaponToEquipIndex = -1;
		    });
		}
		else
		{
            ComponentOwnerCharacter->GetWorldTimerManager().SetTimer(EquipAnotherWeaponTimerHandle, [this]		
            {
                EquipWeapon(WeaponToEquipIndex);
                WeaponToEquipIndex = -1;
            }, DelayToEquipAnotherWeapon, false);
		}
	}
}

FRotator UFNRWeaponComponent::GetPredictedGrenadeRotation() const
{
	if (!IsValid(ComponentOwnerCharacter))
	{
		return FRotator::ZeroRotator;
	}
	if (IsValid(EquippedGrenade))
	{
		const FTransform CameraTransform = GetWeaponComponentInterface()->GetCameraTransform();
		return UKismetMathLibrary::FindLookAtRotation(ComponentOwnerCharacter->GetMesh()->GetSocketLocation(GrenadeSpawn), CameraTransform.GetLocation() + CameraTransform.GetRotation().GetForwardVector() * GrenadeOffset.X + CameraTransform.GetRotation().GetUpVector() * GrenadeOffset.Z + CameraTransform.GetRotation().GetRightVector() * GrenadeOffset.Y);
	}
	return FRotator::ZeroRotator;
}

void UFNRWeaponComponent::ResetRecoil()
{
	bIsInterpolatingRecoil = true;
}

void UFNRWeaponComponent::SetCombatMode(const bool bCombatModeEnabled)
{
	GetWorld()->GetTimerManager().ClearTimer(DetectEnemyTimerHandle);
	DetectedEnemies.Empty();
	SetFocusEnemy(nullptr);
	if (EquippedMeleeWeapon)
	{
		bInCombatMode = bCombatModeEnabled;
		UpdateDetectedEnemies();

		GetWorld()->GetTimerManager().SetTimer(DetectEnemyTimerHandle, [this]
		{
			UpdateDetectedEnemies();
		}, 0.2f, true);
	}
	else
	{
		bInCombatMode = false;
	}
	UE_LOGFMT(LogFSC, Display, "Weapon Component Interface called in {0}", ComponentOwnerCharacter->GetName());
}
#pragma endregion CPP Only

#pragma region Blueprint Exposed
void UFNRWeaponComponent::DestroyAllWeapons()
{
	Server_DestroyAllWeapons();
	OnEquipOrUnEquipWeapon.Broadcast(nullptr);
}

void UFNRWeaponComponent::Server_DestroyAllWeapons_Implementation()
{
	for (AFNRWeapon* Weapon : GetAllWeapons())
	{
		if (!IsValid(Weapon))
		{
			continue;
		}

		// Check if it's a fire weapon and destroy its attachments first
		const AFNRFireWeapon* FoundFireWeapon = Cast<AFNRFireWeapon>(Weapon);
		if (FoundFireWeapon)
		{
			for (AFNRAttachment* Attachment : FoundFireWeapon->AttachmentComponent->GetAttachments())
			{
				if (IsValid(Attachment))
				{
					Attachment->Destroy(true);
				}
			}
		}

		// Destroy the weapon itself
		Weapon->Destroy(true);
	}

	// Broadcast the weapon unequip event
	OnEquipOrUnEquipWeapon.Broadcast(nullptr);
}

bool UFNRWeaponComponent::PickupWeapon(AFNRWeapon* Weapon)
{
	if (!Weapon->IsA<AFNROblivion>())
	{
		if (AFNRGrenade* Grenade = Cast<AFNRGrenade>(Weapon))
		{
			if (GetGrenades(static_cast<EGrenadeFilter>(Grenade->GrenadeData->GrenadeType)).Num() > MaxGrenadesNum - 1)
			{
				return false;
			}
		}
		else
		{
			if (GetWeapons().Num() > MaxWeaponNum - 1)
			{
				if (IsValid(EquippedWeapon))
				{
					DropCurrentWeapon();
				}
				else
				{
					return false;
				}
			}
		}
	}
	//? Verify if the weapon reference is valid, if player don't have the weapon, and the weapon don't have owner
	if (Weapon && Weapon->CharacterOwner == nullptr)
	{
		AddWeapon(Weapon);
		Weapon->OnPickuped.Broadcast();
		return true;
	}
	return false;
}

void UFNRWeaponComponent::EquipWeapon(const int WeaponIndex)
{
	Internal_EquipWeapon(WeaponIndex, false);
}

void UFNRWeaponComponent::EquipOblivion()
{
	Internal_EquipWeapon(AFNROblivionManager::GetOblivion(this));
}

void UFNRWeaponComponent::EquipGrenade(const EGrenadeFilter GrenadeFilter)
{
	Internal_EquipWeapon(GetGrenades(GrenadeFilter).Num() - 1, true, GrenadeFilter);
}

void UFNRWeaponComponent::UnEquipWeapon(const bool bForceUnequip)
{
	if (!IsValid(EquippedWeapon)) return;
	
	if (bForceUnequip || !bHasMontageRunning)
	{
		Fire(false);
		if (IsValid(EquippedFireWeapon))
		{
			InspectWeapon(false);
			EquippedFireWeapon->GetWorldTimerManager().ClearTimer(EquippedFireWeapon->ReloadTimerHandle);
			EquippedFireWeapon->GetWorldTimerManager().ClearTimer(EquippedFireWeapon->FireWithDelayTimerHandle);
		}
		else if (IsValid(EquippedMeleeWeapon) && !EquippedMeleeWeapon->bAllowChangeWeapon)
		{
			return;
		}
		OnEquipOrUnEquipWeapon.Broadcast(nullptr);
		UnEquipWeaponServer(bForceUnequip);
	}
}

void UFNRWeaponComponent::DropWeapon(AFNRWeapon* WeaponToDrop, const bool bSimulatePhysics)
{
	if (!WeaponToDrop) return;
	if (!bHasMontageRunning)
	{
		if (WeaponToDrop == EquippedWeapon)
		{
			UnEquipWeapon(true);
		}
		DropWeapon_Server(WeaponToDrop, WeaponToDrop->IsA<AFNRFireWeapon>() ? Cast<AFNRFireWeapon>(WeaponToDrop)->BulletsInMag : 0, bSimulatePhysics);
		OnEquipOrUnEquipWeapon.Broadcast(nullptr);
	}
}

void UFNRWeaponComponent::DropCurrentWeapon(const bool bSimulatePhysics)
{
	if (bHasMontageRunning) return;
	if (EquippedFireWeapon)
	{
		EquippedFireWeapon->GetWorldTimerManager().ClearTimer(EquippedFireWeapon->ReloadTimerHandle);
		EquippedFireWeapon->GetWorldTimerManager().ClearTimer(EquippedFireWeapon->FireWithDelayTimerHandle);
		DropWeapon_Server(EquippedFireWeapon, EquippedFireWeapon->BulletsInMag, bSimulatePhysics);
		OnEquipOrUnEquipWeapon.Broadcast(nullptr);
		return;
	}
	if (EquippedGrenade)
	{
		FinishSpawnGrenade(true);
	}
	else if (EquippedMeleeWeapon && !EquippedMeleeWeapon->bAllowChangeWeapon) {return;}
	DropWeapon_Server(EquippedWeapon, 0, bSimulatePhysics);
	OnEquipOrUnEquipWeapon.Broadcast(nullptr);
}

void UFNRWeaponComponent::DropAllWeapons(const bool bIncludeGrenades, const bool bSimulatePhysics)
{
	if (Weapons.Num() <= 0) return;

	for (int i = 0; i < Weapons.Num() - 1; ++i)
	{		
		const auto& LocallyWeapon = Weapons[i];
		if (!IsValid(LocallyWeapon))
		{
			continue;
		}
		const auto& LocallyGrenade = Cast<AFNRGrenade>(LocallyWeapon);
		if (!bIncludeGrenades && IsValid(LocallyGrenade))
		{
			continue;
		}
		DropWeapon(LocallyWeapon, bSimulatePhysics);
		OnEquipOrUnEquipWeapon.Broadcast(nullptr);
	}
}

void UFNRWeaponComponent::LockCameraInTarget(const bool bLocked)
{
	if (EquippedMeleeWeapon)
	{
		LockedInEnemy = bLocked;
	}
}

void UFNRWeaponComponent::NextFireMode() const
{
	if (!IsValid(EquippedFireWeapon) || !IsValid(ComponentOwnerCharacter))
	{
		return;
	}

	
	if (!EquippedFireWeapon || !ComponentOwnerCharacter) return;

	TArray<EFireMode_PRAS> AvailableFireModes;
	
	if (EquippedFireWeapon->Internal_FireWeaponData.PossibleFireModes.HasTag(FscFireModeTags::CanAuto)) AvailableFireModes.Add(Auto);
	if (EquippedFireWeapon->Internal_FireWeaponData.PossibleFireModes.HasTag(FscFireModeTags::CanBurst)) AvailableFireModes.Add(Burst);
	if (EquippedFireWeapon->Internal_FireWeaponData.PossibleFireModes.HasTag(FscFireModeTags::CanSingle)) AvailableFireModes.Add(Semi);

	int CurrentIndex = AvailableFireModes.IndexOfByKey(EquippedFireWeapon->CurrentFireMode);
	if (CurrentIndex == INDEX_NONE) return;

	int NextIndex = (CurrentIndex + 1) % AvailableFireModes.Num();
	EquippedFireWeapon->SetFireMode(AvailableFireModes[NextIndex]);
	OnChangeFireMode.Broadcast(AvailableFireModes[NextIndex]);
}

void UFNRWeaponComponent::Fire(const bool bFire)
{
	if (bIsInspecting) return;

	if (EquippedGrenade)
	{
		if (bHasMontageRunning)
		{
			FinishSpawnGrenade(true);
			return;
		}
		if (bFire)
		{
			if (UAnimMontage* StartGrenadeMontage = EquippedGrenade->LoadedEquipCharacterMontage)
			{
				PlayMontage_Server(StartGrenadeMontage);
			}
			else
			{
				StartSpawnGrenade();
			}
		}
		else
		{
			FinishSpawnGrenade(false);
		}
	}
	else if (EquippedWeapon)
	{
		if (EquippedWeapon->GetWeaponState().HasTag(FscWeaponStateTags::CanFire))
		{
			if (bFire)
			{
				if (!bHasMontageRunning && EquippedWeapon->Fire(true))
				{
					GetWeaponComponentInterface()->OnCharacterToggleWeaponFire(true);
					
					if (EquippedWeapon->HasAmmo())
					{			
						bIsInterpolatingRecoil = false;

						const AController* Controller = ComponentOwnerCharacter->GetController();
						if (Controller)
						{
							OriginalPitch = Controller->GetControlRotation().Pitch;
						}
					}
					else
					{
						ResetRecoil();
					}
				}
			}
			else
			{
				EquippedWeapon->Fire(false);
				
				ResetRecoil();

				GetWeaponComponentInterface()->OnCharacterToggleWeaponFire(false);
			}
		}
	}
}

void UFNRWeaponComponent::AIFire(const bool bFire, const FVector TargetLocation, const float Precision)
{
	if (EquippedGrenade)
	{
		if (bHasMontageRunning)
		{
			FinishSpawnGrenade(true);
			return;
		}
		if (UAnimMontage* StartGrenadeMontage = EquippedGrenade->LoadedEquipCharacterMontage)
		{
			PlayMontage_Server(StartGrenadeMontage);
		}
		else
		{
			StartSpawnGrenade();
			FinishSpawnGrenade(false);
		}
	}
	else if (EquippedWeapon)
	{
		if (EquippedWeapon->GetWeaponState().HasTag(FscWeaponStateTags::CanFire))
		{
			if (bFire)
			{
				if (!bHasMontageRunning && EquippedWeapon->AIFire(true, TargetLocation, Precision))
				{
					GetWeaponComponentInterface()->OnCharacterToggleWeaponFire(true);
					
					if (EquippedWeapon->HasAmmo())
					{			
						bIsInterpolatingRecoil = false;

						const AController* Controller = ComponentOwnerCharacter->GetController();
						if (Controller)
						{
							OriginalPitch = Controller->GetControlRotation().Pitch;
						}
					}
					else
					{
						ResetRecoil();
					}
				}
			}
			else
			{
				EquippedWeapon->AIFire(false, TargetLocation, Precision);
				
				ResetRecoil();

				GetWeaponComponentInterface()->OnCharacterToggleWeaponFire(false);
			}
		}
	}
}

bool UFNRWeaponComponent::ThrowGrenade()
{
	if (bCanThrow && IsValid(EquippedGrenade))
	{
		bPredictingGrenade = false;
		if (IsValid(PredictSplineComponent))
		{
			PredictSplineComponent->ClearSplinePoints();
		}
		ThrowGrenade_Server();
		return true;
	}
	return false;
}

void UFNRWeaponComponent::PredictGrenade() const
{
    if (!ComponentOwnerCharacter->IsLocallyControlled()) return;

    if (bPredictingGrenade && bCanThrow && IsValid(EquippedGrenade) && PredictEffectComponent && PredictSplineComponent)
    {
        PredictEffectComponent->SetHiddenInGame(false);
        PredictSplineComponent->ClearSplinePoints();
        FPredictProjectilePathParams PredictParams;
        PredictParams.StartLocation = ComponentOwnerCharacter->GetMesh()->GetSocketLocation(GrenadeSpawn);
        PredictParams.LaunchVelocity = GetPredictedGrenadeRotation().Vector() * EquippedGrenade->GrenadeData->GrenadeSpeed;
        UE_LOG(LogFSC, Display, TEXT("Predicting Grenade with Launch Velocity: X=%f, Y=%f, Z=%f"), 
               PredictParams.LaunchVelocity.X, PredictParams.LaunchVelocity.Y, PredictParams.LaunchVelocity.Z);
        PredictParams.ProjectileRadius = EquippedGrenade->GrenadeData->GrenadeRadius;
        PredictParams.bTraceWithChannel = true;
        PredictParams.bTraceWithCollision = true;
        PredictParams.TraceChannel = EquippedGrenade->MeshComponent->GetCollisionObjectType();
        PredictParams.ActorsToIgnore.Add(ComponentOwnerCharacter);
    	PredictParams.MaxSimTime = EquippedGrenade->GrenadeData->GrenadeRadius;
		#if WITH_EDITOR
        PredictParams.DrawDebugType = IsValid(GrenadePredictNiagaraEffect.LoadSynchronous()) ? EDrawDebugTrace::None : EDrawDebugTrace::ForDuration;
        PredictParams.DrawDebugTime = GetWorld()->GetDeltaSeconds();
		#endif
        FPredictProjectilePathResult Result;
        UGameplayStatics::PredictProjectilePath(this, PredictParams, Result);
        if (Result.PathData.Num() > 0)
        {
            for (const auto& Pd : Result.PathData)
            {
                PredictSplineComponent->AddSplinePoint(Pd.Location, ESplineCoordinateSpace::World);
            }
        }
    }
    else if (IsValid(PredictEffectComponent))
    {
        PredictEffectComponent->SetHiddenInGame(true);
    }
}

IFNRWeaponComponentInterface* UFNRWeaponComponent::GetWeaponComponentInterface() const
{
	if (ComponentOwnerCharacter->Implements<UFNRWeaponComponentInterface>())
	{
		return Cast<IFNRWeaponComponentInterface>(ComponentOwnerCharacter);
	}
	return nullptr;
}

void UFNRWeaponComponent::Internal_EquipWeapon(const int WeaponIndex, const bool& bGetGrenades, const EGrenadeFilter& GrenadeType)
{
	if (bHasMontageRunning) return;
	WeaponToEquipIndex = WeaponIndex;
	AFNRWeapon* WeaponToEquip;
	if (bGetGrenades)
	{
		if (GetGrenades(GrenadeType).IsValidIndex(WeaponIndex) && GetGrenades(GrenadeType)[WeaponIndex])
		{
			WeaponToEquip = GetGrenades(GrenadeType)[WeaponIndex];
			UE_LOGFMT(LogFSC, Display, "Grenade To Equip: {0}", WeaponToEquip->GetName());
		}
		else
		{
			UE_LOGFMT(LogFSC, Warning, "Is a invalid index");
			return;
		}
	}
	else if (GetWeapons().IsValidIndex(WeaponIndex) && GetWeapons()[WeaponIndex])
	{
		WeaponToEquip = GetWeapons()[WeaponIndex];
	}
	else
	{
		return;
	}
	Internal_EquipWeapon(WeaponToEquip);
}

void UFNRWeaponComponent::Internal_EquipWeapon(AFNRWeapon* WeaponToEquip)
{	
	if (bHasMontageRunning || (!Weapons.Contains(WeaponToEquip) || !WeaponToEquip)) {return;}
	WeaponToEquipIndex = Weapons.Find(WeaponToEquip);
	if (IsValid(EquippedWeapon))
	{
		if (WeaponToEquip == EquippedWeapon && bHolstOnEquipSameWeapon)
		{
			UnEquipWeapon(false);
		}
		else
		{
			OnEndAnyMontage.AddUniqueDynamic(this, &UFNRWeaponComponent::EquipAnotherWeapon);
			UnEquipWeapon(false);
		}
	}
	else
	{
		EquipWeaponServer(WeaponToEquip);
		OnEquipOrUnEquipWeapon.Broadcast(WeaponToEquip);
		
		if (ComponentOwnerCharacter->IsLocallyControlled())
		{
			UGameplayStatics::SpawnSoundAtLocation(this, WeaponToEquip->GetGeneralData().EquipSound.LoadSynchronous(), ComponentOwnerCharacter->GetActorLocation());
		}
	}
}

void UFNRWeaponComponent::SetFocusEnemy(AActor* NewFocusedEnemy)
{
	if (EnemyInFocus != NewFocusedEnemy)
	{
		OnUpdateFocusedEnemy.Broadcast(EnemyInFocus, NewFocusedEnemy);
	}
	EnemyInFocus = NewFocusedEnemy;
}

void UFNRWeaponComponent::UpdateDetectedEnemies()
{
	if (bInCombatMode)
	{
		const TArray<AActor*> ActorsToIgnore = {ComponentOwnerCharacter, EquippedMeleeWeapon};
		TArray<FHitResult> HitResult;
		DetectedEnemies.Empty();
		const TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes = {UEngineTypes::ConvertToObjectType(ECC_Pawn)};
		if (UKismetSystemLibrary::SphereTraceMultiForObjects(this, ComponentOwnerCharacter->GetActorLocation(), ComponentOwnerCharacter->GetActorLocation(), CombatModeRadius, ObjectTypes, false, ActorsToIgnore, EDrawDebugTrace::None, HitResult, false, FLinearColor::Red, FLinearColor::Green, 0.011f))
		{
			for (const auto& h : HitResult)
			{
				DetectedEnemies.AddUnique(h.GetActor());
			}
		}
	}
}

void UFNRWeaponComponent::Reload() const
{
	if (EquippedFireWeapon)
	{
		EquippedFireWeapon->Reload();
	}
}

void UFNRWeaponComponent::ReloadByAnim(const int Quantity) const
{
	if (EquippedFireWeapon)
	{
		EquippedFireWeapon->ReloadByAnim(Quantity);
	}
}

void UFNRWeaponComponent::InspectWeapon(const bool bInspect)
{
	if (!IsValid(ComponentOwnerPlayerController))
	{
		ComponentOwnerPlayerController = Cast<APlayerController>(ComponentOwnerCharacter->GetController());
	}
	
	if (!IsValid(ComponentOwnerPlayerController) || !IsValid(EquippedFireWeapon)) return;
	if (bInspect)
	{
		ComponentOwnerPlayerController->bShowMouseCursor = true;
		UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(ComponentOwnerPlayerController, nullptr, EMouseLockMode::LockInFullscreen, false, true);
		bIsInspecting = true;
		if (AttachmentsWidget)
		{
			AttachmentsWidget->RemoveFromParent();
		}
		Fire(false);
		AttachmentsWidget = CreateWidget<UFNRAttachmentSwitch>(ComponentOwnerPlayerController, AvailableAttachmentsWidgetClass);
		AttachmentsWidget->UpdateInfo(this, EAttachmentType::Other);
		AttachmentsWidget->AddToViewport();
		ComponentOwnerPlayerController->bShowMouseCursor = true;
	}
	else
	{
		bIsInspecting = false;
		ComponentOwnerPlayerController->bShowMouseCursor = false;
		UWidgetBlueprintLibrary::SetInputMode_GameOnly(ComponentOwnerPlayerController);
		if (AttachmentsWidget)
		{
			AttachmentsWidget->RemoveFromParent();
		}
	}
	OnInspect.Broadcast(bIsInspecting);
}

void UFNRWeaponComponent::AttachWeapon(AFNRWeapon* Weapon, const bool bHolster)
{
	if (!Weapon || !ComponentOwnerCharacter)
	{
		return;
	}
	if (ComponentOwnerCharacter->IsLocallyControlled() && IsValid(EquippedWeapon) && EquippedWeapon->GetGeneralData().HolsterSound.LoadSynchronous() && bHolster)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, EquippedWeapon->GetGeneralData().HolsterSound.Get(), ComponentOwnerCharacter->GetActorLocation());
	}
	AttachWeapon_Server(Weapon, bHolster);
}


TArray<AFNRWeapon*> UFNRWeaponComponent::GetWeapons() const
{
	TArray<AFNRWeapon*> OnlyWeapons;
	if (Weapons.Num() <= 0) {return OnlyWeapons;}
	for (const auto& W : Weapons)
	{
		if (!W) {continue;}

		if (W->IsA<AFNRFireWeapon>() || W->IsA<AFNRMeleeWeapon>())
		{
			OnlyWeapons.Add(W);
		}
	}
	return OnlyWeapons;
}

TArray<AFNRFireWeapon*> UFNRWeaponComponent::GetFireWeapons() const
{
	TArray<AFNRFireWeapon*> FireWeapons;
	for (const auto& W : Weapons)
	{
		if (!W) {continue;}

		if (AFNRFireWeapon* FireWeapon = Cast<AFNRFireWeapon>(W))
		{
			FireWeapons.Add(FireWeapon);
		}
	}
	return FireWeapons;
}

TArray<AFNRMeleeWeapon*> UFNRWeaponComponent::GetMeleeWeapons() const
{
	TArray<AFNRMeleeWeapon*> MeleeWeapons;
	for (const auto& W : Weapons)
	{
		if (!W) {continue;}

		if (AFNRMeleeWeapon* MeleeWeapon = Cast<AFNRMeleeWeapon>(W))
		{
			MeleeWeapons.Add(MeleeWeapon);
		}
	}
	return MeleeWeapons;
}

TArray<FName> UFNRWeaponComponent::GetWeaponsNames(const bool bAllWeaponsName) const
{
	TArray<FName> WeaponsNames;
	if (Weapons.Num() <= 0) {return WeaponsNames;}
	for (const auto& Wc : Weapons)
	{
		if (!Wc) {continue;}
		if (bAllWeaponsName)
		{
			WeaponsNames.Add(Wc->GetGeneralData().WeaponName);
		}
		else if (Wc->IsA<AFNRFireWeapon>() || Wc->IsA<AFNRMeleeWeapon>())
		{
			WeaponsNames.Add(Wc->GetGeneralData().WeaponName);
		}
	}
	return WeaponsNames;
}

TArray<TSubclassOf<AFNRWeapon>> UFNRWeaponComponent::GetWeaponsClasses() const
{
	TArray<TSubclassOf<AFNRWeapon>> WeaponsClasses;
	if (Weapons.Num() <= 0) {return WeaponsClasses;}
	for (const auto& W : Weapons)
	{
		if (W->IsA<AFNRFireWeapon>() || W->IsA<AFNRMeleeWeapon>())
		{
			WeaponsClasses.Add(W->GetClass());
		}
	}
	return WeaponsClasses;
}

TArray<AFNRGrenade*> UFNRWeaponComponent::GetGrenades(EGrenadeFilter GrenadeFilter)
{
	TArray<AFNRGrenade*> Grenades;
	if (Weapons.Num() <= 0)
	{
		return Grenades;
	}
	for (const auto& G : Weapons)
	{
		AFNRGrenade* Grenade = Cast<AFNRGrenade>(G);
		if (Grenade && (GrenadeFilter == EGrenadeFilter::All || static_cast<uint8>(Grenade->GrenadeData->GrenadeType) == static_cast<uint8>(GrenadeFilter)))
		{
			Grenades.Add(Grenade);
		}
	}
	return Grenades;
}

int32 UFNRWeaponComponent::GetAmmoByType(const FGameplayTag WeaponAmmo) const
{
	if (WeaponAmmo == FscAmmoTypeTags::Energy)
	{
		return EnergyAmmo;
	}
	if (WeaponAmmo == FscAmmoTypeTags::Heavy)
	{
		return HeavyAmmo;
	}
	if (WeaponAmmo == FscAmmoTypeTags::ShotgunAmmo)
	{
		return ShotgunAmmo;
	}
	if (WeaponAmmo == FscAmmoTypeTags::Precision)
	{
		return PrecisionAmmo;
	}
	if (WeaponAmmo == FscAmmoTypeTags::Light)
	{
		return LightAmmo;
	}
	return 0;
}

void UFNRWeaponComponent::SetCanFire(const bool bCanFireNow) const
{
	if (!IsValid(EquippedWeapon)) return;
	if (bCanFireNow)
	{
		EquippedWeapon->GetWeaponState().AddTag(FscWeaponStateTags::CanFire);
	}
	else
	{
		EquippedWeapon->GetWeaponState().RemoveTag(FscWeaponStateTags::CanFire);
	}
}

#pragma endregion Blueprint Exposed

#pragma region Replicated Functions
void UFNRWeaponComponent::ThrowGrenade_Server_Implementation()
{
	if (IsValid(EquippedGrenade)) 
	{
		EquippedGrenade->SetActorRotation(GetPredictedGrenadeRotation());
		EquippedGrenade->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		EquippedGrenade->Fire(true);

		GetWeaponComponentInterface()->OnCharacterThrowGrenade();
		Weapons.Remove(EquippedGrenade);
		CurrentWeapons.Remove(EquippedGrenade->GetClass());
		EquippedWeapon = nullptr;
		EquippedGrenade = nullptr;
	}
}

void UFNRWeaponComponent::PlayMontage_Server_Implementation(UAnimMontage* AnimMontage)
{
	PlayMontage_Multicast(AnimMontage);
}

void UFNRWeaponComponent::AddAmmoByType_Implementation(const FGameplayTag WeaponAmmo, const int AmmoQuantity)
{
	if (WeaponAmmo == FscAmmoTypeTags::Energy)
	{
		EnergyAmmo += AmmoQuantity;
	}
	else if (WeaponAmmo == FscAmmoTypeTags::Heavy)
	{
		HeavyAmmo += AmmoQuantity;
	}
	if (WeaponAmmo == FscAmmoTypeTags::ShotgunAmmo)
	{
		ShotgunAmmo += AmmoQuantity;
	}
	if (WeaponAmmo == FscAmmoTypeTags::Precision)
	{
		PrecisionAmmo += AmmoQuantity;
	}
	if (WeaponAmmo == FscAmmoTypeTags::Light)
	{
		LightAmmo += AmmoQuantity;
	}
}

void UFNRWeaponComponent::PlayMontage_Multicast_Implementation(UAnimMontage* AnimMontage)
{
	if (AnimMontage)
	{
		bHasMontageRunning = true;
		ComponentOwnerCharacter->GetMesh()->GetAnimInstance()->Montage_Play(AnimMontage);
		FTimerHandle HasMontageRunningTimerHandle;
		GetWorld()->GetTimerManager().ClearTimer(HasMontageRunningTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(HasMontageRunningTimerHandle, [this, AnimMontage]
		{
			bHasMontageRunning = false;
			OnEndAnyMontage.Broadcast(AnimMontage);
		}, AnimMontage->GetPlayLength(), false);
	}
}

void UFNRWeaponComponent::AttachWeapon_Server_Implementation(AFNRWeapon* Weapon, const bool bHolster)
{
	if (!Weapon) return;
	SetWeaponFade(Weapon, bHolster);
	Weapon->MeshComponent->SetSimulatePhysics(false);
	Weapon->MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (bHolster)
	{
		FName HolsterSocket = "";
		if (Weapon->GetGeneralData().HolsterSocket.Num() > 0)
		{
			if (Weapon->IsA<AFNROblivion>())
			{
				HolsterSocket = Weapon->GetGeneralData().HolsterSocket[0];
			}
			else if (Weapon->IsA<AFNRFireWeapon>() || Weapon->IsA<AFNRMeleeWeapon>())
			{
				if (WeaponsSockets.Num() == 0)
				{
					HolsterSocket = Weapon->GetGeneralData().HolsterSocket[0];
				}
				else
				{
					HolsterSocket = GetFirstHolsterSocket(Weapon);
				}
				WeaponsSockets.AddUnique(HolsterSocket.ToString() + Weapon->GetName());
			}
		}
		Weapon->AttachToComponent(ComponentOwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, HolsterSocket);
		Weapon->SetActorRelativeRotation(FRotator::ZeroRotator);
		Weapon->SetActorHiddenInGame(IsValid(Cast<AFNRGrenade>(Weapon)) ? true : bHideWeaponOnHolst);
		const AFNRFireWeapon* FireWeapon = Cast<AFNRFireWeapon>(Weapon);
		if (FireWeapon && FireWeapon->AttachmentComponent->GetAttachments().Num() > 0)
		{
			for (const auto& ArrayElements: FireWeapon->AttachmentComponent->GetAttachments())
			{
				if (ArrayElements->Attachment->AttachToWeapon)
				{
					ArrayElements->SetActorHiddenInGame(Weapon->IsHidden());
				}
				else
				{
					ArrayElements->SetActorHiddenInGame(true);
				}
			}
		}
	}
	else
	{
		UE_LOGFMT(LogFSC, Display, "Grenade Attached in {0} socket in Weapon Mesh, and is physics is {1}", GrenadeSpawn, Weapon->MeshComponent->IsAnySimulatingPhysics());
		Weapon->AttachToComponent(ComponentOwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, IsValid(Cast<AFNRGrenade>(Weapon)) ? GrenadeSpawn : Weapon->GetGeneralData().HandSocket);
		Weapon->SetActorHiddenInGame(false);
		Weapon->SetActorRelativeRotation(Weapon->GetGeneralData().HandRotatorFix);
		if (AFNRFireWeapon* FireWeapon = Cast<AFNRFireWeapon>(Weapon); IsValid(FireWeapon) && FireWeapon->AttachmentComponent->GetAttachments().Num() > 0)
		{
			for (const auto& ArrayElements: FireWeapon->AttachmentComponent->GetAttachments())
			{
				ArrayElements->SetActorHiddenInGame(!ArrayElements->Attachment->AttachToWeapon);
			}
		}
	}
}

void UFNRWeaponComponent::SetWeaponFade_Implementation(AFNRWeapon* Weapon, const bool bHide)
{
	if (!Weapon->GetGeneralData().bShouldFade) {return;}
	if (ComponentOwnerCharacter->IsLocallyControlled())
	{
		Weapon->SetFade(bHide);
	}
}

void UFNRWeaponComponent::AddWeapon_Implementation(AFNRWeapon* Weapon)
{
	if (!Weapon) return;
	UE_LOGFMT(LogFSC, Display, "Weapon: {0}", Weapon->GetName());
	Weapons.Add(Weapon);
	CurrentWeapons.Add(Weapon->GetClass());
	Weapon->SetCharacterOwner(ComponentOwnerCharacter);
	Weapon->InteractableComponent->SetInteractionActive(false);
	AttachWeapon(Weapon, true);
	GetWeaponComponentInterface()->OnCharacterPickupWeapon();
}

void UFNRWeaponComponent::EquipWeaponServer_Implementation(AFNRWeapon* Reference)
{
	if (!Reference) return;
	EquippedWeapon = Reference;
	CurrentWeaponIndex = GetWeapons().Find(Reference);
	EquippedGrenade = Cast<AFNRGrenade>(EquippedWeapon);
	EquippedFireWeapon = Cast<AFNRFireWeapon>(EquippedWeapon);
	EquippedMeleeWeapon = Cast<AFNRMeleeWeapon>(EquippedWeapon);
	if (IsValid(EquippedMeleeWeapon))
	{
		SetCombatMode(true);
	}
	UE_LOGFMT(LogFSC, Display, "Equipped Grenade: {0}", IsValid(EquippedGrenade) ? EquippedGrenade->GetName() : "Invalid");
	if (!Reference->LoadedUnEquipCharacterMontage)
	{
		AttachWeapon(Reference, false);
	}
	else
	{
		PlayMontage_Multicast(Reference->LoadedEquipCharacterMontage);
	}
	OnEquipOrUnEquipWeapon.Broadcast(EquippedWeapon);
}

void UFNRWeaponComponent::UnEquipWeaponServer_Implementation(const bool bForceUnequip)
{
	if (!IsValid(EquippedWeapon)) return;
	if (!EquippedWeapon->LoadedUnEquipCharacterMontage || bForceUnequip)
	{
		AttachWeapon(EquippedWeapon, true);
		ClearEquippedWeapon();
	}
	else
	{
		PlayMontage_Multicast(EquippedWeapon->LoadedUnEquipCharacterMontage);
		OnEquipOrUnEquipWeapon.Broadcast(nullptr);
	}
}

void UFNRWeaponComponent::ClearEquippedWeapon()
{
	Server_ClearEquippedWeapon();
	EquippedWeapon = nullptr;
	CurrentWeaponIndex = -1;
	EquippedGrenade = nullptr;
	EquippedFireWeapon = nullptr;
	if (EquippedMeleeWeapon)
	{
		EquippedMeleeWeapon->bAllowAttackByPastAnim = false;
		SetCombatMode(false);
	}
	EquippedMeleeWeapon = nullptr;
	OnEquipOrUnEquipWeapon.Broadcast(nullptr);
}

void UFNRWeaponComponent::Server_ClearEquippedWeapon_Implementation()
{
	EquippedWeapon = nullptr;
	CurrentWeaponIndex = -1;
	EquippedGrenade = nullptr;
	EquippedFireWeapon = nullptr;
	if (EquippedMeleeWeapon)
	{
		EquippedMeleeWeapon->bAllowAttackByPastAnim = false;
		SetCombatMode(false);
	}
	EquippedMeleeWeapon = nullptr;
	OnEquipOrUnEquipWeapon.Broadcast(nullptr);
}

void UFNRWeaponComponent::DropWeapon_Server_Implementation(AFNRWeapon* WeaponToDrop, const int& LocalBulletsInMag, const bool bSimulatePhysics)
{
	if (!WeaponToDrop) {return;}
	// Check if is a fire weapon and local bullets in mag is equal 0
	if (WeaponToDrop->IsA<AFNRFireWeapon>() && LocalBulletsInMag == 0) {return;}
	GetWeaponComponentInterface()->OnCharacterDropWeapon();
	// Unequip weapon if weapon to drop is equal to equipped weapon
	if (WeaponToDrop == EquippedWeapon)
	{
		UnEquipWeapon(true);
	}
	AFNRFireWeapon* FireWeaponToDrop = Cast<AFNRFireWeapon>(WeaponToDrop);
	if (FireWeaponToDrop)
	{
		// Update bullets in weapon by local bullets in mag parameter
		UpdateBullets(FireWeaponToDrop, LocalBulletsInMag);

		/*
		// Drop attachments in weapon
		if (FireWeaponToDrop->AttachmentComponent->GetAttachments().Num() > 0)
		{
			const TArray<AFNRAttachment*> AttachmentsInWeapon = FireWeaponToDrop->AttachmentComponent->GetAttachments();
			for (const auto& a: AttachmentsInWeapon)
			{
				FireWeaponToDrop->AttachmentComponent->RemoveAttachment(a, true);
			}
		}*/
	}
	// Check if weapons array contains this weapon
	if (Weapons.Contains(WeaponToDrop))
	{
		// Clear socket in weapons sockets list (to can equip another weapon in the place)
		FString WeaponSocket;
		for (const auto& s : WeaponsSockets)
		{
			if (s.Contains(WeaponToDrop->GetName(), ESearchCase::CaseSensitive, ESearchDir::FromStart))
			{
				WeaponSocket = s;
				UE_LOGFMT(LogFSC, Display, "Successful removed {0} in WeaponSockets", WeaponSocket);
				break;
			}
		}

		// Clear references in arrays
		Weapons.Remove(WeaponToDrop);
		WeaponsSockets.Remove(WeaponSocket);
		CurrentWeapons.Remove(WeaponToDrop->GetClass());
	}
	// Drop
	WeaponToDrop->SetCharacterOwner(nullptr);
	WeaponToDrop->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	WeaponToDrop->MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponToDrop->MeshComponent->SetSimulatePhysics(bSimulatePhysics);
	WeaponToDrop->InteractableComponent->SetInteractionActive(true);
	WeaponToDrop->SetActorHiddenInGame(false);
	SetWeaponFade(WeaponToDrop, false);
}

void UFNRWeaponComponent::LoadValues()
{
}

void UFNRWeaponComponent::UpdateBullets_Implementation(AFNRWeapon* Weapon, const int& LocalBulletsInMag)
{
	if (Cast<AFNRFireWeapon>(Weapon))
	{
		Cast<AFNRFireWeapon>(Weapon)->BulletsInMag = LocalBulletsInMag;
		Weapon->InteractableComponent->SetTooltipText(FString::FromInt(LocalBulletsInMag));
	}
}

#pragma endregion Replicated Functions
