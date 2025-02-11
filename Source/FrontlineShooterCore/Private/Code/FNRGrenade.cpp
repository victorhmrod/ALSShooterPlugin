// All rights reserved Wise Labs �


#include "Code/FNRGrenade.h"

#include "Code/FNRFireWeapon.h"
#include "Core/ExplosionComponent.h"
#include "Core/InteractableComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AFNRGrenade::AFNRGrenade()
{
	PrimaryActorTick.bCanEverTick = true;
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Grenade Projectile Movement"));
	ProjectileMovement->InitialSpeed = 0.0f;
	ProjectileMovement->bAutoActivate = false;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.5f;
	ProjectileMovement->Friction = 0.5f;
	ProjectileMovement->bInterpMovement = true;
	ProjectileMovement->bInterpRotation = true;

	ExplosionComponent = CreateDefaultSubobject<UExplosionComponent>(FName{TEXTVIEW("ExplosionComponent")});
}

void AFNRGrenade::OnInteract(UInteractorComponent* Interactor, UInteractableComponent* Interactable)
{
	Super::OnInteract(Interactor, Interactable);
}

bool AFNRGrenade::Fire(const bool bFire)
{
	//MeshComponent->SetSimulatePhysics(true);
	Fire_Server();
	InteractableComponent->SetInteractionActive(false);
	if (GrenadeData->TimeToExplode <= 0.0f)
	{	
		ExplosionComponent->Explode(true);
	}
	else
	{
		FTimerHandle GrenadeTimerHandle;
		GetWorldTimerManager().SetTimer(GrenadeTimerHandle, [this]
		{
			ExplosionComponent->Explode(true);
		}, GrenadeData->TimeToExplode, false);
	}

	return true;
}

void AFNRGrenade::Fire_Server_Implementation()
{
	ProjectileMovement->Velocity = GetActorRotation().Vector() * GrenadeData->GrenadeSpeed;
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	MeshComponent->IgnoreActorWhenMoving(CharacterOwner, true);
	ProjectileMovement->Activate(false);
}

// Called when the game starts or when spawned
void AFNRGrenade::BeginPlay()
{
	Super::BeginPlay();
}

FGeneralWeaponData AFNRGrenade::GetGeneralData() const
{
	if (GrenadeData)
	{
		return GrenadeData->General;
	}
	return FGeneralWeaponData{};
}

bool AFNRGrenade::HasAmmo()
{
	return false;
}

