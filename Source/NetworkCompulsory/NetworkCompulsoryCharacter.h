#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "NetworkCompulsoryCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class ANetworkCompulsoryCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MouseLookAction;

public:

	/** Constructor */
	ANetworkCompulsoryCharacter();
	UFUNCTION(Server, Reliable)
	void MyServerFunc();
	void MyServerFunc_Implementation();
	static bool MyServerFunc_Validate();


protected:

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpEnd();
	static bool MyClientFunc_Validate();
	/** CLIENT → SERVER RPC */



	/** SERVER → SINGLE CLIENT RPC */
	UFUNCTION(Client, Reliable)
	void MyClientFunc();
	void MyClientFunc_Implementation();

	/** SERVER → ALL CLIENTS RPC */
	UFUNCTION(NetMulticast, Reliable)
	void MyMulticastFunc();
	void MyMulticastFunc_Implementation();

	// Client → Server RPC for taking damage
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerTakeDamage(float DamageAmount);
	bool ServerTakeDamage_Validate(float DamageAmount);
	void ServerTakeDamage_Implementation(float DamageAmount);

	// Client → Server RPC for healing
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerHeal(float HealAmount);
	bool ServerHeal_Validate(float HealAmount);
	void ServerHeal_Implementation(float HealAmount);


protected:
	/** Character health */
	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "Health")
	float Health;

	/** Maximum health */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float MaxHealth = 100.f;

	/** Called on clients when Health changes */
	UFUNCTION()
	void OnRep_Health();


public:
	int ServerTakeDamage(int _cpp_par_);
	/** Apply damage (can be called from server or client via RPC) */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void TakeDamage(float DamageAmount);

	/** Heal the character */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void Heal(float HealAmount);


public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};


