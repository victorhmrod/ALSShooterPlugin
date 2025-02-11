//  Wise Labs: Gameworks (c) 2020-2024


#include "Code/Notifies/FNRAnimNotify_AttackEffect.h"

//  Wise Labs: Gameworks (c) 2020-2024


#include "Code/Notifies/FNRAnimNotify_Attack.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Code/FNRWeaponComponent.h"
#include "Code/FNRMeleeWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

void UFNRAnimNotify_AttackEffect::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                        float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
#if WITH_EDITOR
	const auto* World{MeshComp->GetWorld()};
	if (World->WorldType == EWorldType::EditorPreview)
	{
		Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
		return;
	}
#endif
	
	UFNRWeaponComponent* WeaponComp = MeshComp->GetOwner()->GetComponentByClass<UFNRWeaponComponent>();

	if (IsValid(WeaponComp) && IsValid(WeaponComp->EquippedMeleeWeapon.Get()))
	{
		AFNRMeleeWeapon* MeleeWeapon = WeaponComp->EquippedMeleeWeapon.Get();
		bCanRun = IsValid(MeleeWeapon);
		if (bCanRun && MeleeWeapon->WeaponDataAsset->MeleeWeaponTrailNiagaraEffect.LoadSynchronous())
		{
			Template = MeleeWeapon->WeaponDataAsset->MeleeWeaponTrailNiagaraEffect.LoadSynchronous();
		}
		
		FXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(Template, MeleeWeapon->MeshComponent,
			WeaponSocketName, LocationOffset, RotationOffset, EAttachLocation::SnapToTargetIncludingScale, !bDestroyAtEnd);
	}
}

void UFNRAnimNotify_AttackEffect::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
#if WITH_EDITOR
	const auto* World{MeshComp->GetWorld()};
	if (World->WorldType == EWorldType::EditorPreview)
	{
		Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	}
#endif
}

void UFNRAnimNotify_AttackEffect::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
#if WITH_EDITOR
	const auto* World{MeshComp->GetWorld()};
	if (World->WorldType == EWorldType::EditorPreview)
	{
		Super::NotifyEnd(MeshComp, Animation, EventReference);
		return;
	}
#endif

	if (IsValid(FXComponent))
	{
		if (bDestroyAtEnd)
		{
			FXComponent->DestroyComponent();
		}
		else
		{
			FXComponent->Deactivate();
		}
	}
}
