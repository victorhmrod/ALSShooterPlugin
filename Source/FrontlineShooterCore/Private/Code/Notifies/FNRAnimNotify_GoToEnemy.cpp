//  Wise Labs: Gameworks (c) 2020-2024


#include "Code/Notifies/FNRAnimNotify_GoToEnemy.h"

#include "Code/FNRMeleeWeapon.h"
#include "Code/FNRWeaponComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

void UFNRAnimNotify_GoToEnemy::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                      const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	const UFNRWeaponComponent* WeaponComponent = MeshComp->GetOwner()->GetComponentByClass<UFNRWeaponComponent>();
	if (IsValid(WeaponComponent))
	{
		AActor* FocusedEnemy = nullptr;
		
		float Speed = 1500.0f;
		
		if (WeaponComponent->InCombatMode(FocusedEnemy) && IsValid(FocusedEnemy))
		{
			Speed = WeaponComponent->ComponentOwnerCharacter->GetDistanceTo(FocusedEnemy) / 0.12f;
		}
		if (IsValid(FocusedEnemy))
		{
			WeaponComponent->ComponentOwnerCharacter->LaunchCharacter((FocusedEnemy->GetActorLocation() -WeaponComponent->ComponentOwnerCharacter->GetActorLocation()).GetSafeNormal() * Speed, true, false);

			if (WeaponComponent->EquippedMeleeWeapon->WeaponDataAsset->DashSound.LoadSynchronous())
			{
				UGameplayStatics::PlaySound2D(this, WeaponComponent->EquippedMeleeWeapon->WeaponDataAsset->DashSound.LoadSynchronous());
			}
		}
	}
}
