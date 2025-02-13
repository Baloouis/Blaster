// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Blaster.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Microsoft/AllowMicrosoftPlatformTypes.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.0f;
	CameraBoom->bUsePawnControlRotation = true;
	
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget -> SetupAttachment(RootComponent);

	CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComp->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	//Set collision type to custom collision channel
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	//Prevent characters from blocking the camera of other players
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 850.0f);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
}


void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(ABlasterCharacter, OverlappingWeapon);
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, Health);
}


void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

//Called only on the server
void ABlasterCharacter::Elim()
{
	if (CombatComp && CombatComp->EquippedWeapon)
	{
		CombatComp->EquippedWeapon->Dropped();
	}
	MulticastElim();
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&ThisClass::ElimTimerFinished ,
		ElimDelay
		);
}
// Multicast called when a character is eliminated ( Called on Server AND Clients )
void ABlasterCharacter::MulticastElim_Implementation()
{
	bElimmed = true;
	PlayElimMontage();

	// Start dissolve effect
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);

		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue("Dissolve", 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue("Glow", 200.f);
	}
	StartDissolve();

	// Disable character movement
	GetCharacterMovement()->DisableMovement(); // prevent translation
	GetCharacterMovement()->StopMovementImmediately(); // prevent rotation too
	if ( BlasterPlayerController )
	{
		 DisableInput(BlasterPlayerController); // to prevent stuff like shooting the weapon
	}

	//Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//Spawn Elim bot
	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X,GetActorLocation().Y,GetActorLocation().Z + 200.f);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotEffect,
			ElimBotSpawnPoint,
			GetActorRotation()
		);
	}
	if (ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			ElimBotSound,
			GetActorLocation()
			);
	}
}

//Only called on the server
void ABlasterCharacter::ElimTimerFinished()
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if (BlasterGameMode)
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}
}


void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();
	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	BlasterPlayerController = Cast<ABlasterPlayerController>(GetController());
	if (BlasterPlayerController)
	{
		//Assigns CharacterMappingContext to this Character
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(BlasterPlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(CharacterMappingContext, 0);
		}
	}
	UpdateHUDHealth();
	
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ThisClass::ReceiveDamage);
	}


}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//AimOffset(DeltaTime);

	//if we're not a simulated proxy
	if (GetLocalRole() > ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
	
	HideCharacterIfCameraClose();
	PollInit();
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Moving
		EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &ThisClass::MoveForward);
		EnhancedInputComponent->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &ThisClass::MoveRight);

		//Rotation
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &ThisClass::Turn);
		EnhancedInputComponent->BindAction(LookUpAction, ETriggerEvent::Triggered, this, &ThisClass::LookUp);

		//Other boolean actions
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ThisClass::Jump);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &ThisClass::EquipButtonPressed);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered , this, &ThisClass::CrouchButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered , this, &ThisClass::AimButtonPressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered , this, &ThisClass::FireButtonPressed);
		
		
	}
}
void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (CombatComp)
	{
		CombatComp->Character = this;
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (CombatComp == nullptr || CombatComp->EquippedWeapon == nullptr)
	{
		return;
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}
void ABlasterCharacter::PlayElimMontage()
{
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if (CombatComp == nullptr || CombatComp->EquippedWeapon == nullptr)
	{
		return;
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

#pragma region Inputs Callbacks
void ABlasterCharacter::MoveForward(const FInputActionValue& Value)
{
	float FloatValue = Value.Get<float>();

	if (Controller != nullptr && FloatValue != 0.f)
	{
		// Getting a vector parallel to the ground but pointing in the direction of the Yaw of the controller
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.0f);
		const FVector Direction( FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, FloatValue);
	}
}

void ABlasterCharacter::MoveRight(const FInputActionValue& Value)
{
	float FloatValue = Value.Get<float>();

	if (Controller != nullptr && FloatValue != 0.f)
	{
		// Getting a vector parallel to the ground but pointing perpendicular to the direction of the Yaw of the controller
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.0f);
		const FVector Direction( FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, FloatValue);
	}
}

void ABlasterCharacter::Turn(const FInputActionValue& Value)
{
	AddControllerYawInput(Value.Get<float>());
}

void ABlasterCharacter::LookUp(const FInputActionValue& Value)
{
	AddControllerPitchInput(Value.Get<float>());
}



void ABlasterCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}



void ABlasterCharacter::EquipButtonPressed(const FInputActionValue& Value)
{
	if (CombatComp)
	{
		if (HasAuthority()) // We're on the Server
		{
			CombatComp->EquipWeapon(OverlappingWeapon);
		}
		else // We're on a Client
		{
			ServerEquipButtonPressed(); // calls RPC to execute this method on the Server side
		}
	}
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (CombatComp)
	{
		CombatComp->EquipWeapon(OverlappingWeapon);
	}
}

void ABlasterCharacter::CrouchButtonPressed(const FInputActionValue& Value)
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABlasterCharacter::AimButtonPressed(const FInputActionValue& Value)
{
	if (CombatComp)
	{
		CombatComp->SetAiming(Value.Get<bool>());
	}
}

void ABlasterCharacter::FireButtonPressed(const FInputActionValue& Value)
{
	if (CombatComp)
	{
		CombatComp->FireButtonPressed(Value.Get<bool>());
	}
}

#pragma endregion Inputs Callbacks

void ABlasterCharacter::HideCharacterIfCameraClose()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraHideCharacterThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (CombatComp && CombatComp->EquippedWeapon && CombatComp->EquippedWeapon->GetWeaponMesh())
		{
			CombatComp->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (CombatComp && CombatComp->EquippedWeapon && CombatComp->EquippedWeapon->GetWeaponMesh())
		{
			CombatComp->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

//Only called on Clients
void ABlasterCharacter::OnRep_Health()
{
	UpdateHUDHealth();
	    if (!bElimmed)
        {
			PlayHitReactMontage();
        }
}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = (BlasterPlayerController==nullptr) ? Cast<ABlasterPlayerController>(GetController()): BlasterPlayerController;

	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void ABlasterCharacter::PollInit()
{
	if ( BlasterPlayerState == nullptr )
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			BlasterPlayerState->AddToScore(0.f); //Update the Score UI
			BlasterPlayerState->AddToDefeats(0); //Update the Defeats UI
		}
	}
}


void ABlasterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ThisClass::ABlasterCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}


void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue("Dissolve", DissolveValue);
	}
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if(IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(false);
		}
	}
	OverlappingWeapon = Weapon;
	//To handle the case for when the BlasterCharacter controlled by the Server is Overlapping ( so there not Replication called )
	if(IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

bool ABlasterCharacter::IsWeaponEquipped()
{

	bool bRes = (CombatComp && CombatComp->EquippedWeapon);

	return bRes;
}

bool ABlasterCharacter::IsAiming()
{
	return (CombatComp && CombatComp->bAiming);
}

AWeapon* ABlasterCharacter::GetEquippedWeapon() const
{
	if ( CombatComp == nullptr )
	{
		return nullptr;
	}
	return CombatComp->EquippedWeapon;
}

float ABlasterCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (CombatComp && CombatComp->EquippedWeapon == nullptr)
	{
		return;
	}
	
	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();
	
	if (Speed == 0.f && !bIsInAir) // standing still and not jumping
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.0f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation,StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir) // running or jumping
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.0f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();

}

void ABlasterCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	// To take into account the effect of CharacterMovement compression on FRotators that erase rotation winding
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}

}
void ABlasterCharacter::SimProxiesTurn()
{
	if (CombatComp == nullptr && CombatComp->EquippedWeapon == nullptr)
	{
		return;
	}
	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;
	
	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

//Only called on the Server
void ABlasterCharacter::ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType,
	class AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();
	PlayHitReactMontage();

	if (Health == 0.f)
	{
		ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
		if (BlasterGameMode)
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
		}
	}
}




void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;

	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		//Interpolating towards zero
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw =  InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw)<15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.0f);

		}
	}
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if (CombatComp == nullptr)
	{
		return FVector();
	}
	return CombatComp->HitTarget;
}