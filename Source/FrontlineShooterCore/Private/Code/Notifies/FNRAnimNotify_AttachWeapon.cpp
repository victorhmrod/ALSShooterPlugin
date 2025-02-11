// All rights reserved Wise Labs ®


#include "Code/Notifies/FNRAnimNotify_AttachWeapon.h"

#include "Code/FNRFireWeapon.h"
#include "Code/FNRWeapon.h"
#include "Code/FNRWeaponComponent.h"
#include "Kismet/GameplayStatics.h"

void UFNRAnimNotify_AttachWeapon::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                         const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp || !MeshComp->GetOwner()) return;
	
	UFNRWeaponComponent* WeaponSystem = MeshComp->GetOwner()->GetComponentByClass<UFNRWeaponComponent>();
	if (WeaponSystem && WeaponSystem->EquippedWeapon)
	{
		WeaponSystem->AttachWeapon(WeaponSystem->EquippedWeapon, bHolster);
		
		if (bHolster)
		{
			WeaponSystem->ClearEquippedWeapon();
		}
        
		Super::Notify(MeshComp, Animation, EventReference);
	}
}
