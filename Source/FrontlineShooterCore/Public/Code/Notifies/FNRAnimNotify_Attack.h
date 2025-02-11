//  Wise Labs: Gameworks (c) 2020-2024

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "FNRAnimNotify_Attack.generated.h"

class AFNRMeleeWeapon;
class UFNRWeaponComponent;

UCLASS()
class FRONTLINESHOOTERCORE_API UFNRAttack_AnimNotify : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Info")
	bool bResetOnEnd = true;
	
	UPROPERTY(EditAnywhere, Category = "Info")
	FVector2D DamageRange = FVector2D(1.0f, 1.0f);
	
	UPROPERTY(EditAnywhere, Category = "Info")
    float LaunchForce = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Info")
	bool bDebug = false;

private:
	UPROPERTY()
	UFNRWeaponComponent* WeaponComp = nullptr;

	UPROPERTY()
	AFNRMeleeWeapon* MeleeWeapon = nullptr;
	
	bool bCanRun = false;

	bool bEffectIsValid = false;
	
	UPROPERTY()
	TArray<AActor*> HitActors{};
	
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
};
