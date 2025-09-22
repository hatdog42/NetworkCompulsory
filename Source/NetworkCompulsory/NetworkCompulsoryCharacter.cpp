// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetworkCompulsoryCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "NetworkCompulsory.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"

ANetworkCompulsoryCharacter::ANetworkCompulsoryCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ANetworkCompulsoryCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ANetworkCompulsoryCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ANetworkCompulsoryCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ANetworkCompulsoryCharacter::Look);
	}
	else
	{
		UE_LOG(LogNetworkCompulsory, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ANetworkCompulsoryCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void ANetworkCompulsoryCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ANetworkCompulsoryCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr && (Right != 0.0f || Forward != 0.0f))
	{
		if (HasAuthority())
			// it says am i the server and if we are the server it says called on server
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("called on SERVER !!!"));
			//server to single client
			MyClientFunc();

			//server to all clients
			MyMulticastFunc();
		}
		else
			MyServerFunc(); // from client to server
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void ANetworkCompulsoryCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ANetworkCompulsoryCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void ANetworkCompulsoryCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

bool ANetworkCompulsoryCharacter::MyClientFunc_Validate()
{
	return true;
}

// client to server RPC
void ANetworkCompulsoryCharacter::MyServerFunc_Implementation()
{

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("called on client and executed on the SERVER !!!"));
}

bool ANetworkCompulsoryCharacter::MyServerFunc_Validate()
{
	return true;
}

// server to single client RPC
void ANetworkCompulsoryCharacter::MyClientFunc_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("called on SERVER and executed on the CLIENT !"));
}
// server to all clients RPC
void ANetworkCompulsoryCharacter::MyMulticastFunc_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("called on SERVER and executed on ALL CLIENTS !!!"));
}

void ANetworkCompulsoryCharacter::OnRep_Health()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::Printf(TEXT("Health Updated: %f"), Health));

}



// Replication setup
void ANetworkCompulsoryCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetworkCompulsoryCharacter, Health);
}

void ANetworkCompulsoryCharacter::TakeDamage(float DamageAmount)
{
	if (HasAuthority()) // Only server should modify health
	{
		Health = FMath::Clamp(Health - DamageAmount, 0.f, MaxHealth);

		if (Health <= 0.f)
		{
			// Character died
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Character Died!"));
		}
	}
	else
	{
		// If client calls, route to server
		ServerTakeDamage(DamageAmount);
	}
}

void ANetworkCompulsoryCharacter::Heal(float HealAmount)
{
	if (HasAuthority())
	{
		Health = FMath::Clamp(Health + HealAmount, 0.f, MaxHealth);
	}
	else
	{
		ServerHeal(HealAmount);
	}
}
bool ANetworkCompulsoryCharacter::ServerTakeDamage_Validate(float DamageAmount)
{
	return DamageAmount >= 0.f; // basic validation
}

void ANetworkCompulsoryCharacter::ServerTakeDamage_Implementation(float DamageAmount)
{
	TakeDamage(DamageAmount);
}

bool ANetworkCompulsoryCharacter::ServerHeal_Validate(float HealAmount)
{
	return HealAmount >= 0.f;
}

void ANetworkCompulsoryCharacter::ServerHeal_Implementation(float HealAmount)
{
	Heal(HealAmount);
}
