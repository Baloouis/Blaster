// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}



void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	
	if (Character == nullptr || WeaponToEquip == nullptr)
	{
		return;
	}
	
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
	EquippedWeapon->SetOwner(Character);
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}
void UCombatComponent::OnRep_EquippedWeapon()
{

	if (EquippedWeapon && Character)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

//Can be called on both the Clients or the Server
void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	if(bFireButtonPressed)
	{
		//Server RPC
		ServerFire();
	}
}

//This is only executed on the Server side
void UCombatComponent::ServerFire_Implementation()
{
	//This will make sure every Client will execute MulticastFire_Implementation
	MulticastFire();
}


// A multicast RPC called from the server is executed on the Server and all Clients
void UCombatComponent::MulticastFire_Implementation()
{
	if (EquippedWeapon==nullptr)
	{
		return;
	}
	if (Character)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire();
	}
}


void UCombatComponent::SetAiming(bool bIsAiming)
{
	// we're setting bAiming right away so that Client running this code can see its player aim without waiting
	// for Server response, it's not a critical information so we can do this
	bAiming = bIsAiming;

	// Not matter if a Client or the Server calls this, it's gonna execute on the Server
	ServerSetAiming(bIsAiming);

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming? AimWalkSpeed : BaseWalkSpeed;
	}
		
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	// set bAiming on the Server, bAiming being a replicated var it will notify all Clients
	// that the associated character is aiming, for everyone to see.
	bAiming = bIsAiming;

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming? AimWalkSpeed : BaseWalkSpeed;
	}
}

