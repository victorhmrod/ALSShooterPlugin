// All rights reserved Wise Labs ®


#include "Code/Notifies/FNRAnimNotify_AddWeaponAmmoByAnim.h"
#include "Code/FNRWeaponComponent.h"

void UFNRAnimNotify_AddWeaponAmmoByAnim::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                   const FAnimNotifyEventReference& EventReference)
{
	if (UFNRWeaponComponent* WeaponSystem = MeshComp->GetOwner()->GetComponentByClass<UFNRWeaponComponent>(); WeaponSystem)
	{
		WeaponSystem->ReloadByAnim(AmmoQuantity);
	}
	
	Super::Notify(MeshComp, Animation, EventReference);
}
