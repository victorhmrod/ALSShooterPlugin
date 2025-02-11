//  Wise Labs: Gameworks (c) 2020-2024

#include "Code/FNRMeleeWeapon.h"

#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Code/FNRWeaponComponent.h"
#include "Code/Notifies/FNRAnimNotify_Attack.h"
#include "Code/Notifies/FNRAnimNotify_ResetAttack.h"
#include "Components/CapsuleComponent.h"
#include "Data/Interfaces/FNRWeaponComponentInterface.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"

AFNRMeleeWeapon::AFNRMeleeWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFNRMeleeWeapon::BeginPlay()
{
	Super::BeginPlay();

#if WITH_EDITOR
	if (!IsValid(WeaponDataAsset))
	{		
		UKismetSystemLibrary::PrintString(this, "You forgot to put the data asset on this weapon", true, true, FLinearColor::Green, 25.f);
	}
#endif
}

void AFNRMeleeWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

FGeneralWeaponData AFNRMeleeWeapon::GetGeneralData() const
{
	if (!WeaponDataAsset) {return FGeneralWeaponData();}
	return WeaponDataAsset->General;
}

bool AFNRMeleeWeapon::Fire(const bool bFire)
{
	if (bFire)
	{
		return Attack(EAttackType::Light);
	}
	return false;
}

bool AFNRMeleeWeapon::Attack(const EAttackType AttackType)
{
	if (!CharacterOwner || !WeaponSystem)
	{
		return false;
	}

	if ((bAllowAttackByPastAnim || !WeaponSystem->HasFCSMontageRunning()) && WeaponState.HasTag(FscWeaponStateTags::CanFire))
	{
		TArray<TSoftObjectPtr<UAnimMontage>> LightAttackArray;
		WeaponDataAsset->CharacterLightAttackMontages.GenerateKeyArray(LightAttackArray);
		TArray<TSoftObjectPtr<UAnimMontage>> HeavyAttackArray;
		WeaponDataAsset->CharacterHeavyAttackMontages.GenerateKeyArray(HeavyAttackArray);
		TArray<TSoftObjectPtr<UAnimMontage>> AttackAnims = AttackType == EAttackType::Light ? LightAttackArray : HeavyAttackArray;
		
		if (CharacterOwner->GetMovementComponent()->IsFalling())
		{
			TArray<TSoftObjectPtr<UAnimMontage>> InAirAttackArray;
			WeaponDataAsset->CharacterInAirAttackMontages.GenerateKeyArray(InAirAttackArray);
			AttackAnims = InAirAttackArray;
		}

		if (AttackAnims.Num() > 0)
		{
			if (NextAttackIndex < 0)
			{
				NextAttackIndex = FMath::RandRange(0, AttackAnims.Num() - 1);
			}
			else
			{
				NextAttackIndex += 1;
				if (NextAttackIndex >= AttackAnims.Num())
				{
					NextAttackIndex = 0;
				}
			}
			
			UAnimMontage* AttackMontage = AttackAnims[NextAttackIndex].LoadSynchronous();
			if (AttackMontage)
			{				
				PlayAttackAnimation(AttackMontage);
				return true;
			}
		}
	}
	
	return false;
}

// *Frytas
bool AFNRMeleeWeapon::PlayAttackAnimation(UAnimMontage* Animation, const bool bEnableRootMotion)
{
	if (!Animation)
	{
		return false;
	}

	bAllowChangeWeapon = false;
	WeaponSystem->ComponentOwnerPlayerController->SetIgnoreMoveInput(true);
	CharacterOwner->GetMesh()->GetAnimInstance()->SetRootMotionMode(bEnableRootMotion ? ERootMotionMode::Type::RootMotionFromMontagesOnly :
		ERootMotionMode::Type::IgnoreRootMotion);

	const float AnimTime = CharacterOwner->GetMesh()->GetAnimInstance()->Montage_Play(Animation);
	if (AnimTime <= 0.0f)
	{
		CharacterOwner->GetMesh()->GetAnimInstance()->SetRootMotionMode(ERootMotionMode::Type::RootMotionFromMontagesOnly);
		WeaponSystem->ComponentOwnerPlayerController->SetIgnoreMoveInput(false);
		return false;
	}
	
	bAllowAttackByPastAnim = false;
	WeaponSystem->bHasMontageRunning = true;
	
	bool bCallDispatcher = true;
	if (Animation->Notifies.Num() > 0)
	{
		for (const auto& n : Animation->Notifies)
		{
			if (!n.NotifyStateClass && !n.Notify)
			{
				continue;
			}
			
			if (const UFNRAttack_AnimNotify* AttackNotify = Cast<UFNRAttack_AnimNotify>(n.NotifyStateClass))
			{
				if (AttackNotify->bResetOnEnd)
				{
					bCallDispatcher = false;
					break;
				}
			}
			else if (Cast<UFNRAnimNotify_ResetAttack>(n.Notify))
			{
				bCallDispatcher = false;
				break;
			}
		}
	}
	
	GetWorldTimerManager().ClearTimer(AttackTimerHandle);
	GetWorldTimerManager().SetTimer(AttackTimerHandle, [this, bCallDispatcher, Animation]
	{
		if (Animation)
		{
			NextAttackIndex = -1;
			bAllowChangeWeapon = true;
			WeaponSystem->bHasMontageRunning = false;
			WeaponSystem->OnEndAnyMontage.Broadcast(Animation);
			if (bCallDispatcher)
			{
				OnResetAttack.Broadcast();
			}
			CharacterOwner->GetMesh()->GetAnimInstance()->SetRootMotionMode(ERootMotionMode::Type::RootMotionFromMontagesOnly);
			WeaponSystem->ComponentOwnerPlayerController->SetIgnoreMoveInput(false);
		}
	}, AnimTime, false);

	return true;
}
// Frytas*

UAnimMontage* AFNRMeleeWeapon::FindHitReactionAnimation()
{
	if (WeaponDataAsset->CharacterInAirAttackMontages.Find(WeaponSystem->ComponentOwnerCharacter->GetMesh()->GetAnimInstance()->GetCurrentActiveMontage()))
	{
		return WeaponDataAsset->CharacterInAirAttackMontages.Find(WeaponSystem->ComponentOwnerCharacter->GetMesh()->GetAnimInstance()->GetCurrentActiveMontage())->LoadSynchronous();	
	}
	if (WeaponDataAsset->CharacterLightAttackMontages.Find(WeaponSystem->ComponentOwnerCharacter->GetMesh()->GetAnimInstance()->GetCurrentActiveMontage()))
	{
		return WeaponDataAsset->CharacterLightAttackMontages.Find(WeaponSystem->ComponentOwnerCharacter->GetMesh()->GetAnimInstance()->GetCurrentActiveMontage())->LoadSynchronous();
	}
	if (WeaponDataAsset->CharacterHeavyAttackMontages.Find(WeaponSystem->ComponentOwnerCharacter->GetMesh()->GetAnimInstance()->GetCurrentActiveMontage()))
	{
		return WeaponDataAsset->CharacterHeavyAttackMontages.Find(WeaponSystem->ComponentOwnerCharacter->GetMesh()->GetAnimInstance()->GetCurrentActiveMontage())->LoadSynchronous();
	}
	if (WeaponDataAsset->CharacterEnemyFarAttackMontages.Find(WeaponSystem->ComponentOwnerCharacter->GetMesh()->GetAnimInstance()->GetCurrentActiveMontage()))
	{
		return WeaponDataAsset->CharacterEnemyFarAttackMontages.Find(WeaponSystem->ComponentOwnerCharacter->GetMesh()->GetAnimInstance()->GetCurrentActiveMontage())->LoadSynchronous();
	}
	return nullptr;
}

bool AFNRMeleeWeapon::JumpToEnemy(UAnimMontage* CustomAnimation)
{
	if (!CharacterOwner || !WeaponSystem || bDisableKnowback)
	{
		return false;
	}

	if ((bAllowAttackByPastAnim || !WeaponSystem->HasFCSMontageRunning()) && WeaponState.HasTag(FscWeaponStateTags::CanFire) &&
		WeaponSystem->GetDetectedEnemies().Num() > 0)
	{
		UAnimMontage* AttackMontage = CustomAnimation;
		AActor* EnemyInFocus = WeaponSystem->GetDetectedEnemies()[FMath::RandRange(0, WeaponSystem->GetDetectedEnemies().Num() - 1)];
		if (IsValid(EnemyInFocus))
		{
			WeaponSystem->OnAttack.Broadcast(EnemyInFocus);
			
			if (!AttackMontage)
			{
				const int32 AnimIndex = FMath::RandRange(0, WeaponDataAsset->CharacterEnemyFarAttackMontages.Num() - 1);
				TArray<TSoftObjectPtr<UAnimMontage>> FarAttackArray;
				WeaponDataAsset->CharacterEnemyFarAttackMontages.GenerateKeyArray(FarAttackArray);
			
				if (FarAttackArray[AnimIndex].LoadSynchronous())
				{
					AttackMontage = FarAttackArray[AnimIndex].Get();
				}
				else
				{
					return false;
				}
			}
				
			WeaponSystem->GetWeaponComponentInterface()->OnFocusedEnemyIsTooFar(EnemyInFocus);
			
			PlayAttackAnimation(AttackMontage, false);
			
			/*const FRotator TargetRotation = FRotator(0.0f, UKismetMathLibrary::FindLookAtRotation(CharacterOwner->GetActorLocation(), EnemyInFocus->GetActorLocation()).Yaw, 0.0f);
            			
            CharacterOwner->SetActorRotation(FMath::RInterpTo(CharacterOwner->GetActorRotation(), TargetRotation, 0.1, 1.0f));*/

			return true;
		}
	}
	return false;
}

void AFNRMeleeWeapon::OnHitTarget(const FHitResult HitResult, const FVector2D Damage)
{
	if (HitResult.GetActor() == CharacterOwner)
	{
		return;
	}

	if (Damage.X <= 0.0f && Damage.Y <= 0.0f)
	{
		return;
	}
	
	UGameplayStatics::ApplyDamage(HitResult.GetActor(), FMath::RandRange(Damage.X, Damage.Y),
				CharacterOwner->GetInstigatorController(), CharacterOwner->GetOwner(),
				UDamageType::StaticClass());
	
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, WeaponDataAsset->MeleeWeaponHitNiagaraEffect.LoadSynchronous(),
		HitResult.ImpactPoint, FRotator::ZeroRotator);

	UGameplayStatics::PlaySoundAtLocation(this, WeaponDataAsset->MeleeWeaponHitSound.LoadSynchronous(),
				HitResult.ImpactPoint);


	// *VH
	const ACharacter* LocallyEnemyCharacter = Cast<ACharacter>(HitResult.GetActor());
	if (LocallyEnemyCharacter && FindHitReactionAnimation())
	{
		LocallyEnemyCharacter->GetMesh()->GetAnimInstance()->Montage_Play(FindHitReactionAnimation());
	}
}

void AFNRMeleeWeapon::SetAimingStatus(const bool bStatus)
{
	Super::SetAimingStatus(bStatus);
}

bool AFNRMeleeWeapon::EnemyIsFarAway(const AActor* Enemy) const
{
	if (Enemy)
	{
		return CharacterOwner->GetDistanceTo(Enemy) > WeaponDataAsset->DistanceToDash;
	}
	return false;
}

float AFNRMeleeWeapon::PlaySpecialAnimation(UAnimMontage* Montage, const bool bForceCancelExistentMontages)
{
	if (bForceCancelExistentMontages)
	{
		GetWorldTimerManager().ClearTimer(SpecialAnimationTimerHandle);
		CharacterOwner->GetMesh()->GetAnimInstance()->StopAllMontages(0.2f);
	}
	else
	{
		if (GetWorldTimerManager().TimerExists(SpecialAnimationTimerHandle)) {return false;}
	}
	
	CharacterOwner->GetMesh()->GetAnimInstance()->SetRootMotionMode(ERootMotionMode::Type::IgnoreRootMotion);
	WeaponSystem->ComponentOwnerPlayerController->SetIgnoreMoveInput(true);
 
	bAllowAttackByPastAnim = false;
	WeaponSystem->bHasMontageRunning = true;
	bAllowChangeWeapon = false;
	
	CharacterOwner->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

	const float AnimLength = CharacterOwner->GetMesh()->GetAnimInstance()->Montage_Play(Montage);
	
	GetWorldTimerManager().SetTimer(SpecialAnimationTimerHandle, [this, Montage] {
		CharacterOwner->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
		WeaponSystem->bHasMontageRunning = false;
		bAllowChangeWeapon = true;
		WeaponSystem->OnEndAnyMontage.Broadcast(Montage);
		WeaponSystem->ComponentOwnerPlayerController->SetIgnoreMoveInput(false);
		CharacterOwner->GetMesh()->GetAnimInstance()->SetRootMotionMode(ERootMotionMode::Type::RootMotionFromMontagesOnly);
	}, AnimLength, false);

	return AnimLength;
}

void AFNRMeleeWeapon::ThrowEnemyToUp(ACharacter* Enemy, const float TimeInAir, const float LaunchVelocity, const bool bLaunchCharacterAlso)
{
	FTimerHandle ChangeGravityTimerHandle;
	GetWorldTimerManager().SetTimer(ChangeGravityTimerHandle, [this, Enemy]
	{
		CharacterOwner->GetCharacterMovement()->GravityScale = 0.25f;
		
		if (!Enemy) {return;}
		Enemy->GetCharacterMovement()->GravityScale = 0.25f;
	}, TimeInAir, false);
	
	const FVector NewCharacterVelocity = FVector(0.0f, 0.0f, LaunchVelocity);
	Enemy->LaunchCharacter(NewCharacterVelocity, true, true);

	if (bLaunchCharacterAlso)
	{
		CharacterOwner->LaunchCharacter(NewCharacterVelocity, true, true);
	}
}
