//  Wise Labs: Gameworks (c) 2020-2024

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "FNRAnimNotify_ResetAttack.generated.h"

/**
 * 
 */
UCLASS()
class FRONTLINESHOOTERCORE_API UFNRAnimNotify_ResetAttack : public UAnimNotify
{
	GENERATED_BODY()

	// *Frytas
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	// Frytas*
};
