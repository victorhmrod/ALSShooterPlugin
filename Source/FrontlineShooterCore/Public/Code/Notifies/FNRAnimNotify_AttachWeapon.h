// All rights reserved Wise Labs ®

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "FNRAnimNotify_AttachWeapon.generated.h"

UCLASS(DisplayName="AttachWeaponNotify")
class FRONTLINESHOOTERCORE_API UFNRAnimNotify_AttachWeapon : public UAnimNotify
{
	GENERATED_BODY()

protected:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Attaching")
	bool bHolster;
};
