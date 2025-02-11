//  Wise Labs: Gameworks (c) 2020-2024

#pragma once

#include "CoreMinimal.h"
#include "AnimNotifyState_TimedNiagaraEffect.h"
#include "FNRAnimNotify_AttackEffect.generated.h"

class AFNRMeleeWeapon;
class UFNRWeaponComponent;

UCLASS()
class FRONTLINESHOOTERCORE_API UFNRAnimNotify_AttackEffect : public UAnimNotifyState_TimedNiagaraEffect
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Info")
	FName WeaponSocketName = "Effect";
	
private:
	UPROPERTY()
	class UNiagaraComponent* FXComponent;

	bool bCanRun = false;
	
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

};
