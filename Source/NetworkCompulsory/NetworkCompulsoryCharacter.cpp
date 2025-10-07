// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetworkCompulsoryCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Projectile.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Particles/ParticleSystem.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

ANetworkCompulsoryCharacter::ANetworkCompulsoryCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	MaxHealth = 100.f;
	CurrentHealth = MaxHealth;

	ProjectileClass = AProjectile::StaticClass();
	FireRate = 0.25f;
	bIsFiringWeapon = false;

	bReplicates = true;
}

void ANetworkCompulsoryCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ANetworkCompulsoryCharacter::Move);

		if (LookAction)
		{
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ANetworkCompulsoryCharacter::Look);
		}
		if (MouseLookAction)
		{
			EnhancedInput->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ANetworkCompulsoryCharacter::Look);
		}
		if (FireAction)
		{
			EnhancedInput->BindAction(FireAction, ETriggerEvent::Started, this, &ANetworkCompulsoryCharacter::StartFire);
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("%s missing EnhancedInputComponent!"), *GetName());
	}
}

void ANetworkCompulsoryCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	DoMove(MovementVector.X, MovementVector.Y);
}

void ANetworkCompulsoryCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>();
	DoLook(LookAxis.X, LookAxis.Y);
}

void ANetworkCompulsoryCharacter::DoMove(float Right, float Forward)
{
	if (Controller)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDir, Forward);
		AddMovementInput(RightDir, Right);
	}
}

void ANetworkCompulsoryCharacter::DoLook(float Yaw, float Pitch)
{
	if (Controller)
	{
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ANetworkCompulsoryCharacter::DoJumpStart() { Jump(); }
void ANetworkCompulsoryCharacter::DoJumpEnd() { StopJumping(); }

// -------------------- Firing --------------------
void ANetworkCompulsoryCharacter::StartFire()
{
	if (!bIsFiringWeapon)
	{
		bIsFiringWeapon = true;

		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(FiringTimer, this, &ANetworkCompulsoryCharacter::StopFire, FireRate, false);
		}
		HandleFire();
	}
}

void ANetworkCompulsoryCharacter::StopFire()
{
	bIsFiringWeapon = false;
}

void ANetworkCompulsoryCharacter::HandleFire_Implementation()
{
	if (!ProjectileClass || !GetWorld())
		return;

	const FVector SpawnLocation = GetActorLocation() + (GetActorForwardVector() * 100.f) + (GetActorUpVector() * 50.f);
	const FRotator SpawnRotation = GetActorRotation();

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = GetInstigator();

	GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, Params);
}

// -------------------- Health & Replication --------------------
void ANetworkCompulsoryCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANetworkCompulsoryCharacter, CurrentHealth);
}

void ANetworkCompulsoryCharacter::OnHealthUpdate()
{
	if (IsLocallyControlled() && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, FString::Printf(TEXT("Health: %.1f"), CurrentHealth));
		if (CurrentHealth <= 0.f)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("You have died."));
		}
	}

	if (HasAuthority())
	{
		UE_LOG(LogTemplateCharacter, Log, TEXT("%s now has %.1f HP"), *GetName(), CurrentHealth);
	}

	if (CurrentHealth <= 0.f)
	{
		HandleDeath();
	}
}

void ANetworkCompulsoryCharacter::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}

void ANetworkCompulsoryCharacter::SetCurrentHealth(float HealthValue)
{
	if (HasAuthority())
	{
		CurrentHealth = FMath::Clamp(HealthValue, 0.f, MaxHealth);
		OnHealthUpdate();
	}
}

float ANetworkCompulsoryCharacter::TakeDamage(float DamageTaken, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority())
	{
		ServerTakeDamage(DamageTaken);
		return DamageTaken;
	}

	SetCurrentHealth(CurrentHealth - DamageTaken);
	return DamageTaken;
}

// -------------------- RPC IMPLEMENTATIONS --------------------
void ANetworkCompulsoryCharacter::ServerTakeDamage_Implementation(float DamageAmount)
{
	if (!HasAuthority())
		return;

	if (DamageAmount <= 0.f || DamageAmount > 1000.f)
	{
		UE_LOG(LogTemplateCharacter, Warning, TEXT("Rejected invalid damage: %f from %s"), DamageAmount, *GetName());
		return;
	}

	SetCurrentHealth(CurrentHealth - DamageAmount);
	ClientShowHitEffect();

	if (CurrentHealth <= 0.f)
	{
		MulticastPlayExplosion();
		HandleDeath();
	}
}

void ANetworkCompulsoryCharacter::ClientShowHitEffect_Implementation()
{
	PlayHitFeedback();
}

void ANetworkCompulsoryCharacter::ClientRPC_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("ClientRPC executed."));
}

void ANetworkCompulsoryCharacter::ServerRPC_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("ServerRPC executed."));
}

void ANetworkCompulsoryCharacter::MulticastRPC_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("MulticastRPC executed."));
}

void ANetworkCompulsoryCharacter::MulticastPlayExplosion_Implementation()
{
	if (!GetWorld())
		return;

	UParticleSystem* ExplosionEffect = LoadObject<UParticleSystem>(nullptr, TEXT("/Game/StarterContent/Particles/P_Explosion.P_Explosion"));
	if (ExplosionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange, TEXT("Explosion multicast triggered"));
	}
}

// -------------------- Helpers --------------------
void ANetworkCompulsoryCharacter::PlayHitFeedback()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow, TEXT("You've been hit!"));
	}
}

void ANetworkCompulsoryCharacter::HandleDeath()
{
	DisableInput(nullptr);
	GetCharacterMovement()->DisableMovement();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Character has died."));
	}
}

/*	 
void ANetworkCompulsoryCharacter::StartFire()
{
	if (!bIsFiringWeapon)
	{
		bIsFiringWeapon = true;
		UWorld* World = GetWorld();
		World->GetTimerManager().SetTimer(FiringTimer, this, &ANetworkCompulsoryCharacter::StopFire, FireRate, false);
		HandleFire();
	}
}
	 
void ANetworkCompulsoryCharacter::StopFire()
{
	bIsFiringWeapon = false;
}
	 
void ANetworkCompulsoryCharacter::HandleFire_Implementation()
{
	FVector spawnLocation = GetActorLocation() + (GetActorRotation().Vector() * 100.0f) + (GetActorUpVector() * 50.0f);
	FRotator spawnRotation = GetActorRotation();
	 
	FActorSpawnParameters spawnParameters;
	spawnParameters.Instigator = GetInstigator();
	spawnParameters.Owner = this;
	 
	ANetworkCompulsoryCharacter* spawnedProjectile = GetWorld()->SpawnActor<AProjectile>(spawnLocation, spawnRotation, spawnParameters);
}*/