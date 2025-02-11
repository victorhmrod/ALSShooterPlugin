// Fill out your copyright notice in the Description page of Project Settings.


#include "Code/Notifies/FNRAnimNotify_ThrowGrenade.h"

#include "Code/FNRWeaponComponent.h"

void UFNRAnimNotify_ThrowGrenade::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                         const FAnimNotifyEventReference& EventReference)
{
	if (UFNRWeaponComponent* WeaponSystem = MeshComp->GetOwner()->GetComponentByClass<UFNRWeaponComponent>())
	{
		if (bStartThrowing)
		{
			WeaponSystem->StartSpawnGrenade();
		}
		else
		{
			WeaponSystem->ThrowGrenade();
		}
	}
	
	Super::Notify(MeshComp, Animation, EventReference);
}
