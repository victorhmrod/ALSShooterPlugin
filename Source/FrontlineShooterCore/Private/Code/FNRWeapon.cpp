// All rights reserved Wise Labs ®


#include "Code/FNRWeapon.h"

#include "NiagaraComponent.h"
#include "Code/FNRAttachment.h"
#include "Code/FNRAttachmentWeaponComponent.h"
#include "Code/FNRFireWeapon.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Pawn.h"
#include "Code/FNRWeaponComponent.h"
#include "Components/BillboardComponent.h"
#include "Core/InteractableComponent.h"
#include "Core/InteractorComponent.h"
#include "Data/FNRFireWeaponData.h"
#include "Net/UnrealNetwork.h"

void AFNRWeapon::OnRep_bIsFiring()
{
}

FGameplayTagContainer AFNRWeapon::GetWeaponState()
{
	return WeaponState;
}

// Sets default values
AFNRWeapon::AFNRWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Creating components
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	MeshComponent->SetIsReplicated(true);
	SetRootComponent(MeshComponent);
		
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	InteractableComponent = CreateDefaultSubobject<UInteractableComponent>(TEXT("Weapon Interactable Component"));
	InteractableComponent->SetupAttachment(MeshComponent);

	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("BillboardComponent"));
	BillboardComponent->SetupAttachment(MeshComponent);
	
	GlowNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(FName{TEXTVIEW("GlowNiagaraComponent")});
	GlowNiagaraComponent->SetupAttachment(MeshComponent, "Center");

	MeshComponent->SetReceivesDecals(false);

	bReplicates = true;
	AActor::SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void AFNRWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	// Bind events to on overlap weapon collision
	InteractableComponent->OnInteract.AddDynamic(this, &AFNRWeapon::OnInteract);
	
	WeaponState.AddTag(FscWeaponStateTags::CanFire);

	RefreshCPPOnly();
}

void AFNRWeapon::ReadValues()
{
	if (IsValid(GetGeneralData().WeaponBodyMesh.LoadSynchronous()))
	{
		LoadedWeaponMesh = GetGeneralData().WeaponBodyMesh.Get();
		MeshComponent->SetSkeletalMeshAsset(LoadedWeaponMesh);
	}

	if (GetGeneralData().WeaponAnimInstance.LoadSynchronous())
	{
		MeshComponent->SetAnimInstanceClass(GetGeneralData().WeaponAnimInstance.Get());
	}
	
	if (IsValid(GetGeneralData().CharacterUnEquipMontage.LoadSynchronous()))
	{
		LoadedUnEquipCharacterMontage = GetGeneralData().CharacterUnEquipMontage.Get();
	}

	if (IsValid(GetGeneralData().CharacterEquipMontage.LoadSynchronous()))
	{
		LoadedEquipCharacterMontage = GetGeneralData().CharacterEquipMontage.Get();
	}
	
	if (IsValid(GetGeneralData().CharacterFireMontage.LoadSynchronous()))
	{
		LoadedFireCharacterMontage = GetGeneralData().CharacterFireMontage.Get();
	}

	if (IsValid(GetGeneralData().WeaponEquipAnim.LoadSynchronous()))
	{
		LoadedWeaponEquipAnim = GetGeneralData().WeaponEquipAnim.Get();
	}

	if (IsValid(GetGeneralData().WeaponUnEquipAnim.LoadSynchronous()))
	{
		LoadedWeaponUnEquipAnim = GetGeneralData().WeaponUnEquipAnim.Get();
	}

	if (IsValid(GetGeneralData().WeaponFireAnim.LoadSynchronous()))
	{
		LoadedWeaponFireAnim = GetGeneralData().WeaponFireAnim.Get();
	}
}

void AFNRWeapon::PreInitializeComponents()
{
	ReadValues();
	
	Super::PreInitializeComponents();
}

IFNRWeaponComponentInterface* AFNRWeapon::GetWeaponComponentInterface() const
{
	if (IsValid(CharacterOwner) && CharacterOwner->Implements<UFNRWeaponComponentInterface>())
	{
		return Cast<IFNRWeaponComponentInterface>(CharacterOwner);
	}
	return nullptr;
}

// Called every frame
void AFNRWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFNRWeapon::RefreshGlow_Implementation() const
{
	GlowNiagaraComponent->SetVisibility(!IsValid(CharacterOwner), true);

	const auto& GlowableMeshes = GetGlowableMeshes();
	if (GlowableMeshes.Num() > 0)
	{
		for (const auto& ArrayElement : GlowableMeshes)
		{
			if (IsValid(ArrayElement))
			{
				ArrayElement->SetScalarParameterValueOnMaterials(FName{"Is Pickup"}, GetOwner() ? 0.f : 1.f);
			}
		}
	}

	FLinearColor ColorToSet;
	if (GetGeneralData().RarityMode == FscRarityType::Commom)
	{
		ColorToSet = RarityColors.Common;
	}
	else if (GetGeneralData().RarityMode == FscRarityType::Uncommom)
	{
		ColorToSet = RarityColors.Uncommon;
	}
	else if (GetGeneralData().RarityMode == FscRarityType::Rare)
	{
		ColorToSet = RarityColors.Rare;
	}
	else if (GetGeneralData().RarityMode == FscRarityType::UltraRare)
	{
		ColorToSet = RarityColors.UltraRare;
	}
	else if (GetGeneralData().RarityMode == FscRarityType::Divine)
	{
		ColorToSet = RarityColors.Divine;
	}
	else if (GetGeneralData().RarityMode == FscRarityType::Mythic)
	{
		ColorToSet = RarityColors.Mythic;
	}
	else
	{
		ColorToSet = RarityColors.Common;
	}

	for (const auto& ArrayElement : GlowableMeshes)
	{
		if (IsValid(ArrayElement))
		{
			ArrayElement->SetVectorParameterValueOnMaterials(FName{"Pickup Color"}, FVector	(ColorToSet));
		}
	}
	GlowNiagaraComponent->SetVariableLinearColor(FName{"Color"}, ColorToSet);
}

TArray<UMeshComponent*> AFNRWeapon::GetGlowableMeshes() const
{
	TArray<UMeshComponent*> FoundMeshes;

	FoundMeshes.Add(MeshComponent);

	return FoundMeshes;
}

void AFNRWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CharacterOwner);
	DOREPLIFETIME(ThisClass, WeaponSystem);
	DOREPLIFETIME(ThisClass, bIsFiring);
}

void AFNRWeapon::SetFade(const bool bHolster)
{
	GetWorldTimerManager().ClearTimer(FadeTimerHandle);
	
	const float TargetValue = bHolster ? 1.0f : 0.0f;
	const float InterpVelocity = bHolster ? GetGeneralData().HolsterFadeVelocity : GetGeneralData().EquipFadeVelocity;
	
	GetWorldTimerManager().SetTimer(FadeTimerHandle, [this, TargetValue, InterpVelocity]
	{
		if (CurrentValue != TargetValue)
		{
			CurrentValue = FMath::FInterpConstantTo(CurrentValue, TargetValue,
				GetWorld()->GetDeltaSeconds(), InterpVelocity);
				
			MeshComponent->SetScalarParameterValueOnMaterials("Fade", CurrentValue);

			TArray<USceneComponent*> ChildrenArray;
			MeshComponent->GetChildrenComponents(true, ChildrenArray);
			if (ChildrenArray.Num() > 0)
			{
				for (const auto& ArrayElement : ChildrenArray)
				{
					UMeshComponent* ChildrenMeshComponent = Cast<UMeshComponent>(ArrayElement);
					if (ChildrenMeshComponent)
					{
						ChildrenMeshComponent->SetScalarParameterValueOnMaterials("Fade", CurrentValue);
					}
				}
			}

			const AFNRFireWeapon* FireWeapon = Cast<AFNRFireWeapon>(this);
			if (FireWeapon)
			{
				FireWeapon->MagazineComponent->SetScalarParameterValueOnMaterials("Fade", CurrentValue);
					
				for (const auto& a : FireWeapon->AttachmentComponent->GetAttachments())
				{
					if (!a)
					{
						continue;
					}
						
					a->AttachmentMesh->SetScalarParameterValueOnMaterials("Fade", CurrentValue);

					a->SetActorHiddenInGame(false);
					if (CurrentValue >= 1.0f)
					{
						a->SetActorHiddenInGame(true);
					}
				}
			}
		}
		else
		{
			GetWorldTimerManager().ClearTimer(FadeTimerHandle);
		}
	}, GetWorld()->GetDeltaSeconds(), true);
}

#pragma region Binded Functions
void AFNRWeapon::OnInteract(UInteractorComponent* Interactor, UInteractableComponent* Interactable)
{
	UFNRWeaponComponent* Ws = Interactor->GetOwner()->GetComponentByClass<UFNRWeaponComponent>();
	if (IsValid(Ws))
	{
		Ws->PickupWeapon(this);
	}
}

FGeneralWeaponData AFNRWeapon::GetGeneralData() const
{
	return FGeneralWeaponData();
}

void AFNRWeapon::GetFactors(float& SpreadFactor, float& DamageFactor, float& RecoilFactor) const
{
	DamageFactor = GetWeaponComponentInterface()->GetDamageMultiplier();

	SpreadFactor = GetWeaponComponentInterface()->GetSpreadMultiplier();

	RecoilFactor = GetWeaponComponentInterface()->GetRecoilMultiplier();
}

bool AFNRWeapon::Fire(const bool bFire)
{
	return true;
}

bool AFNRWeapon::AIFire(const bool bFire, const FVector TargetLocation, const float Precision)
{
	return true;
}

bool AFNRWeapon::HasAmmo()
{
	return false;
}

void AFNRWeapon::RefreshCPPOnly()
{
	ReadValues();
	RefreshGlow();
	
	InteractableComponent->SetDisplayText(FText::FromName(GetGeneralData().WeaponName).ToString());
}

void AFNRWeapon::SetAimingStatus_Implementation(bool bStatus)
{
	if (bStatus)
	{
		WeaponState.AddTag(FscWeaponStateTags::InAds);
	}
	else
	{
		WeaponState.RemoveTag(FscWeaponStateTags::InAds);
	}
}

void AFNRWeapon::SetCharacterOwner_Implementation(AFNRPlayerCharacter* NewCharacterOwner)
{
	CharacterOwner = NewCharacterOwner;
	SetOwner(NewCharacterOwner);
	SetInstigator(NewCharacterOwner);
	if (NewCharacterOwner)
	{
		WeaponSystem = NewCharacterOwner->GetComponentByClass<UFNRWeaponComponent>();
	}
	
	Internal_SetCharacterOwner(NewCharacterOwner);
}

void AFNRWeapon::Internal_SetCharacterOwner(AFNRPlayerCharacter* NewCharacterOwner)
{
	RefreshGlow();
}

#pragma endregion Binded Functions

