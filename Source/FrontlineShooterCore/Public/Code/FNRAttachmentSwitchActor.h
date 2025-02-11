// All rights reserved Wise Labs ®

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FNRAttachmentSwitchActor.generated.h"

UCLASS()
class FRONTLINESHOOTERCORE_API AFNRAttachmentSwitchActor : public AActor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UWidgetComponent> Widget;
	
public:
	// Sets default values for this actor's properties
	AFNRAttachmentSwitchActor();
	
};
