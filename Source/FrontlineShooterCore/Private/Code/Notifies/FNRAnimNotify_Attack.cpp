//  Wise Labs: Gameworks (c) 2020-2024


#include "Code/Notifies/FNRAnimNotify_Attack.h"

#include "Code/FNRWeaponComponent.h"
#include "Code/FNRMeleeWeapon.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

void UFNRAttack_AnimNotify::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                        float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	HitActors.Empty();
	WeaponComp = MeshComp->GetOwner()->GetComponentByClass<UFNRWeaponComponent>();

	if (IsValid(WeaponComp) && IsValid(WeaponComp->EquippedMeleeWeapon.Get()))
	{
		MeleeWeapon = WeaponComp->EquippedMeleeWeapon.Get();
		bCanRun = true;
		bEffectIsValid = IsValid(MeleeWeapon->WeaponDataAsset->MeleeWeaponHitNiagaraEffect.LoadSynchronous());
	}
}

void UFNRAttack_AnimNotify::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!bResetOnEnd)
	{
		return;
	}

	if (IsValid(WeaponComp) && IsValid(MeleeWeapon))
	{
		MeleeWeapon->bAllowAttackByPastAnim = true;
		MeleeWeapon->OnResetAttack.Broadcast();
		WeaponComp->ComponentOwnerPlayerController->SetIgnoreMoveInput(false);
		MeshComp->GetAnimInstance()->StopAllMontages(0.2f);
	}
}

void UFNRAttack_AnimNotify::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                       float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
#if WITH_EDITOR
	const auto* World{MeshComp->GetWorld()};
	if (World->WorldType == EWorldType::EditorPreview)
	{
		return;
	}
#endif
	
	if (bCanRun)
	{
		TArray<FHitResult> HitResult;
		const TArray<AActor*> ActorsToIgnore = {MeshComp->GetOwner(), MeleeWeapon};
		const FTransform StartEnd = MeleeWeapon->MeshComponent->GetSocketTransform(MeleeWeapon->GetGeneralData().MuzzleSocket);

		const TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes{UEngineTypes::ConvertToObjectType(ECC_Pawn)};
		UKismetSystemLibrary::BoxTraceMultiForObjects(MeshComp, StartEnd.GetLocation(), StartEnd.GetLocation(), StartEnd.GetScale3D(),
			StartEnd.Rotator(), ObjectTypes, false,
			ActorsToIgnore, bDebug ? EDrawDebugTrace::None : EDrawDebugTrace::None, HitResult, true, FLinearColor::Red, FLinearColor:: Green, 5.0f);
		
		for (const auto& h : HitResult)
		{
			if (HitActors.Contains(h.GetActor()))
			{
				continue;
			}
			
			if (ACharacter* EnemyCharacter = Cast<ACharacter>(h.GetActor()); !MeleeWeapon->bDisableKnowback && EnemyCharacter && LaunchForce != 0.0f)
			{
				EnemyCharacter->LaunchCharacter(EnemyCharacter->GetActorForwardVector() * -LaunchForce +
					EnemyCharacter->GetActorUpVector() * 30.0f, false, false);
			}
			
			MeleeWeapon->OnHitTarget(h, DamageRange);
			HitActors.Add(h.GetActor());
		}
	}
}
