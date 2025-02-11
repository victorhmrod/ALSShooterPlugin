// All rights reserved Wise Labs �

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "FNRAnimNotify_AddWeaponAmmoByAnim.generated.h"

/**
 * 
 */
UCLASS(DisplayName="AddAmmoNotify")
class FRONTLINESHOOTERCORE_API UFNRAnimNotify_AddWeaponAmmoByAnim : public UAnimNotify
{
	GENERATED_BODY()

protected:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int AmmoQuantity{1};
};
