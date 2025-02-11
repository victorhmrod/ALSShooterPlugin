//  Wise Labs: Gameworks (c) 2020-2024

#include "Code/Notifies/FNRAnimNotify_ResetAttack.h"

#include "Code/FNRMeleeWeapon.h"
#include "Code/FNRWeaponComponent.h"

// *Frytas
void UFNRAnimNotify_ResetAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                        const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (UFNRWeaponComponent* WeaponComp = MeshComp->GetOwner()->GetComponentByClass<UFNRWeaponComponent>())
	{
		if (!WeaponComp->EquippedMeleeWeapon) {return;}

		WeaponComp->EquippedMeleeWeapon->bAllowAttackByPastAnim = true;
		WeaponComp->EquippedMeleeWeapon->OnResetAttack.Broadcast();
	}
}
// Frytas*