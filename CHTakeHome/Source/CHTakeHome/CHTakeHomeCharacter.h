// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "CHHUD.h"
#include "CHTakeHomeCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ACHTakeHomeCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* WalkAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ShootAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Gun1Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Gun2Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Gun3Action;



public:
	ACHTakeHomeCharacter();


	//virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	
	/** Called for walking modifier */
	
	
	UFUNCTION()
	void StartRun(const FInputActionValue& Value);
	UFUNCTION()
	void StopRun(const FInputActionValue& Value);
	UFUNCTION()
	void Run(float DeltaTime);
	
	//Shooting Functions
	UFUNCTION()
	void StartFire(const FInputActionValue& Value);
	UFUNCTION()
	void StopFire(const FInputActionValue& Value);
	UFUNCTION()
	void FireShot();

	//GunVariables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	int gunDamage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float timeBetweenShots;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	int maxMagSize;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	int curMagSize;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	int curMagSizeGun1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	int curMagSizeGun2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	int curMagSizeGun3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float reloadTime;

	//Aim down sight Functions
	UFUNCTION()
	void AimIn(const FInputActionValue& Value);
	UFUNCTION()
	void AimOut(const FInputActionValue& Value);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float stamina;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	bool running;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float rechargeRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float consumeRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	FTimerHandle TimerHandle_HandleRefire;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float health;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float maxHealth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gampplay)
	float healthRegenRate;

	UFUNCTION()
	void GainHealth(const float pHealth);

	UFUNCTION(Server, Reliable)
	void Server_GainHealth(const float pHealth);

	UFUNCTION()
	void TakeDamage(const float pDamage);
	UFUNCTION(Server, Reliable)
	void Server_TakeDamage(const float pDamage);

	UFUNCTION()
	void OnRep_Health();

	UFUNCTION()
	void SwitchToGun1();
	UFUNCTION()
	void SwitchToGun2();
	UFUNCTION()
	void SwitchToGun3();

	UFUNCTION()
	void Reload();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	TSubclassOf<UCHHUD> GameHUDClass;

	UPROPERTY()
	class UCHHUD* GameHUD;
			

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	virtual void Tick(float DeltaTime) override;
};

