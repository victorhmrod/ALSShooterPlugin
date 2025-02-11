//  Wise Labs: Gameworks (c) 2020-2024

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "FNRAnimNotify_GoToEnemy.generated.h"

/**
 * 
 */
UCLASS()
class FRONTLINESHOOTERCORE_API UFNRAnimNotify_GoToEnemy : public UAnimNotify
{
	GENERATED_BODY()

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
