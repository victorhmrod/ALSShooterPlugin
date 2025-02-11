//  Wise Labs: Gameworks (c) 2020-2024

#pragma once

#include "CoreMinimal.h"
#include "Code/FNRFireWeapon.h"
#include "FNRBow.generated.h"

UCLASS()
class FRONTLINESHOOTERCORE_API AFNRBow : public AFNRFireWeapon
{
	GENERATED_BODY()

	AFNRBow();

public:
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "Info")
	float FireStrength = 0.0f;

private:
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaSeconds) override;
	
	virtual bool Fire(const bool bFire) override;
};
