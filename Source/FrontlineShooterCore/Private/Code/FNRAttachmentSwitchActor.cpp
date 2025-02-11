// All rights reserved Wise Labs ®


#include "Code/FNRAttachmentSwitchActor.h"

#include "Components/WidgetComponent.h"
#include "Components/WidgetInteractionComponent.h"


// Sets default values
AFNRAttachmentSwitchActor::AFNRAttachmentSwitchActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Widget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	SetRootComponent(Widget);
	Widget->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));
	Widget->SetDrawAtDesiredSize(true);
	Widget->SetTwoSided(true);
}


