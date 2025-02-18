// All rights reserved Wise Labs �


#include "Code/FNRGrenade.h"

#include "Code/FNRFireWeapon.h"
#include "Core/ExplosionComponent.h"
#include "Core/InteractableComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values for the grenade
AFNRGrenade::AFNRGrenade()
{
	PrimaryActorTick.bCanEverTick = true; // Enables ticking if needed

	// Initialize projectile movement component
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Grenade Projectile Movement"));
	ProjectileMovement->InitialSpeed = 0.0f;  // Starts stationary until fired
	ProjectileMovement->bAutoActivate = false;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.5f;
	ProjectileMovement->Friction = 0.5f;
	ProjectileMovement->bInterpMovement = true;
	ProjectileMovement->bInterpRotation = true;

	// Initialize explosion component
	ExplosionComponent = CreateDefaultSubobject<UExplosionComponent>(FName{TEXTVIEW("ExplosionComponent")});
}

// Handles interaction with the grenade
void AFNRGrenade::OnInteract(UInteractorComponent* Interactor, UInteractableComponent* Interactable)
{
	Super::OnInteract(Interactor, Interactable);
	// Interaction logic can be added here if needed
}

// Fires the grenade
bool AFNRGrenade::Fire(const bool bFire)
{
	//MeshComponent->SetSimulatePhysics(true); // Uncomment if physics simulation is needed
	Fire_Server(); // Call the server-side fire logic
	InteractableComponent->SetInteractionActive(false); // Disable further interaction after firing

	// Check if grenade should explode immediately or after a delay
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

// Server-side implementation of firing logic
void AFNRGrenade::Fire_Server_Implementation()
{
	// Set velocity based on actor's facing direction and grenade speed
	ProjectileMovement->Velocity = GetActorRotation().Vector() * GrenadeData->GrenadeSpeed;

	// Enable collision for physics interaction
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);

	// Prevent collision with the character who threw the grenade
	MeshComponent->IgnoreActorWhenMoving(CharacterOwner, true);

	// Activate projectile movement
	ProjectileMovement->Activate(false);
}

// Called when the game starts or when spawned
void AFNRGrenade::BeginPlay()
{
	Super::BeginPlay();
}

// Retrieves general grenade data
FGeneralWeaponData AFNRGrenade::GetGeneralData() const
{
	if (GrenadeData)
	{
		return GrenadeData->General;
	}
	return FGeneralWeaponData{}; // Return default data if GrenadeData is null
}

// Grenades typically don't have "ammo", so always return false
bool AFNRGrenade::HasAmmo()
{
	return false;
}
