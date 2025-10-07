// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "NetworkCompulsoryCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;
class AProjectile;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS()
class NETWORKCOMPULSORY_API ANetworkCompulsoryCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ANetworkCompulsoryCharacter();

protected:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** Input actions */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MouseLookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* FireAction;

	/** Projectile class */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Projectile")
	TSubclassOf<AProjectile> ProjectileClass;

	/** Fire rate */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	float FireRate;

	bool bIsFiringWeapon;
	FTimerHandle FiringTimer;

protected:
	/** Health */
	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float MaxHealth;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, VisibleAnywhere, Category = "Health")
	float CurrentHealth;

	UFUNCTION()
	void OnRep_CurrentHealth();
	void OnHealthUpdate();

public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Movement / Look */
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoMove(float Right, float Forward);

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoLook(float Yaw, float Pitch);

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpStart();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpEnd();

	/** Health API */
	UFUNCTION(BlueprintPure, Category = "Health")

	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetCurrentHealth(float HealthValue);
	UFUNCTION(BlueprintCallable, Category = "Health")
	virtual float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	/** Firing */
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void StopFire();

	UFUNCTION(Server, Reliable)
	void HandleFire();

	/** Damage RPCs */
	UFUNCTION(Server, Reliable)
	void ServerTakeDamage(float DamageAmount);

	UFUNCTION(Client, Reliable)
	void ClientShowHitEffect();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayExplosion();

	/** Example RPC test functions */
	UFUNCTION(Client, Reliable)
	void ClientRPC();

	UFUNCTION(Server, Reliable)
	void ServerRPC();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC();

protected:
	void PlayHitFeedback();
	void HandleDeath();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
//protected:
	 
	//UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Projectile")
	//TSubclassOf<class AProjectile> ProjectileClass;
	 
	/** Delay between shots in seconds. Used to control fire rate for your test projectile, but also to prevent an overflow of server functions from binding SpawnProjectile directly to input.*/
	//UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	//float FireRate;
	 
	/** If true, you are in the process of firing projectiles. */
	//bool bIsFiringWeapon;
	 
	/** Function for beginning weapon fire.*/
	//UFUNCTION(BlueprintCallable, Category = "Gameplay")
	//void StartFire();
	 
	/** Function for ending weapon fire. Once this is called, the player can use StartFire again.*/
	//UFUNCTION(BlueprintCallable, Category = "Gameplay")
	//void StopFire();
	 
	/** Server function for spawning projectiles.*/
	//UFUNCTION(Server, Reliable)
	//void HandleFire();
	 
	/** A timer handle used for providing the fire rate delay in-between spawns.*/
	//FTimerHandle FiringTimer;

	// Input assets (assign these in the editor)
	//UPROPERTY(EditDefaultsOnly, Category="Input")
	//UInputMappingContext* DefaultMappingContext;

	//UPROPERTY(EditDefaultsOnly, Category="Input")
	//UInputAction* FireAction;


