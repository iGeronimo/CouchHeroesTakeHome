// Copyright Epic Games, Inc. All Rights Reserved.

#include "CHTakeHomeCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Net/UnrealNetwork.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "CHHUD.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ACHTakeHomeCharacter

ACHTakeHomeCharacter::ACHTakeHomeCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 400.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
	stamina = 10.f;
	consumeRate = 5.f;
	rechargeRate = .5f;

	gunDamage = 10.f;
	timeBetweenShots = 1.f;

	maxHealth = 100;
	health = maxHealth;

	bReplicates = true;
}

void ACHTakeHomeCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (GameHUDClass && World)
	{
		APlayerController* PlayerController = Cast<APlayerController>(GetController());
		GameHUD = CreateWidget<UCHHUD>(PlayerController, GameHUDClass);
		if (GameHUD)
		{
			GameHUD->AddToViewport();

		}
		else
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to create widget"));
		}
	}
	else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to assign HUD widget"));
	}
}

void ACHTakeHomeCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GameHUD)
	{
		GameHUD->RemoveFromParent();
		GameHUD = nullptr;
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Should have Deleted HUD"));
	}

	Super::EndPlay(EndPlayReason);
}

void ACHTakeHomeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Run(DeltaTime);
}

void ACHTakeHomeCharacter::GainHealth(const float pHealth)
{
	health += pHealth;
}

void ACHTakeHomeCharacter::Server_GainHealth_Implementation(const float pHealth)
{
	GainHealth(pHealth);
}

void ACHTakeHomeCharacter::TakeDamage(const float pDamage)
{
	if (HasAuthority())
	{
		health -= pDamage;

		if (health <= 0)
		{
			health = 0;
			GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
			GetMesh()->SetSimulatePhysics(true);
		}

		if (GameHUD) GameHUD->SetHealth(health, maxHealth);
	}
}

//void ACHTakeHomeCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//
//	DOREPLIFETIME_CONDITION(ACHTakeHomeCharacter, health, COND_OwnerOnly);
//}

void ACHTakeHomeCharacter::OnRep_Health()
{
	//GameHUD->SetHealth(health, maxHealth);
}

void ACHTakeHomeCharacter::Server_TakeDamage_Implementation(const float pDamage)
{
	TakeDamage(pDamage);
}

void ACHTakeHomeCharacter::SwitchToGun1()
{
	gunDamage = 20.f;
	timeBetweenShots = .5f;
	maxMagSize = 6;
	curMagSize = curMagSizeGun1;
}

void ACHTakeHomeCharacter::SwitchToGun2()
{
	gunDamage = 5.f;
	timeBetweenShots = .1f;
	maxMagSize = 20;
	curMagSize = curMagSizeGun2;
}

void ACHTakeHomeCharacter::SwitchToGun3()
{
	gunDamage = 5.f;
	timeBetweenShots = 1.f;
	maxMagSize = 5;
	curMagSize = curMagSizeGun3;
}

void ACHTakeHomeCharacter::Reload()
{

}

//////////////////////////////////////////////////////////////////////////
// Input

void ACHTakeHomeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACHTakeHomeCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACHTakeHomeCharacter::Look);

		//// Walking
		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Started, this, &ACHTakeHomeCharacter::StartRun);
		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Completed, this, &ACHTakeHomeCharacter::StopRun);

		//Shooting
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &ACHTakeHomeCharacter::StartFire);
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Completed, this, &ACHTakeHomeCharacter::StopFire);

		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ACHTakeHomeCharacter::AimIn);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ACHTakeHomeCharacter::AimOut);

		//Guns
		EnhancedInputComponent->BindAction(Gun1Action, ETriggerEvent::Triggered, this, &ACHTakeHomeCharacter::SwitchToGun1);
		EnhancedInputComponent->BindAction(Gun2Action, ETriggerEvent::Triggered, this, &ACHTakeHomeCharacter::SwitchToGun2);
		EnhancedInputComponent->BindAction(Gun3Action, ETriggerEvent::Triggered, this, &ACHTakeHomeCharacter::SwitchToGun3);
	}
}

void ACHTakeHomeCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ACHTakeHomeCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ACHTakeHomeCharacter::StartRun(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		running = true;
	}
}

void ACHTakeHomeCharacter::StopRun(const FInputActionValue& Value)
{
	if (!Value.Get<bool>())
	{
		running = false;
	}
}

void ACHTakeHomeCharacter::Run(float DeltaTime)
{
	if (running && stamina > 0)
	{
		GetCharacterMovement()->MaxWalkSpeed = 600.f;
		stamina -= (DeltaTime * consumeRate);
		if (stamina < 0) running = false;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = 400.f;
		stamina += (DeltaTime * rechargeRate);
		if (stamina > 100) stamina = 100;
	}
	if(GameHUD) GameHUD->SetStamina(stamina, 100);
}


void ACHTakeHomeCharacter::StartFire(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		FireShot();

		GetWorldTimerManager().SetTimer(TimerHandle_HandleRefire, this, &ACHTakeHomeCharacter::FireShot, timeBetweenShots, true);
	}
}

void ACHTakeHomeCharacter::StopFire(const FInputActionValue& Value)
{
	if (!Value.Get<bool>())
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_HandleRefire);
	}
}

void ACHTakeHomeCharacter::FireShot()
{
	FHitResult Hit;


	const FVector StartTrace = GetFollowCamera()->GetComponentLocation();
	const FVector EndTrace = GetFollowCamera()->GetForwardVector() * 10000.f + StartTrace;

	FCollisionQueryParams QueryParams(FName(TEXT("LineTrace")), true, this);
	QueryParams.AddIgnoredActor(this); //Ignores our own character when shooting

	if (GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, ECC_GameTraceChannel1, QueryParams))
	{

		//Spawning of particles if we hit something
		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, FTransform(Hit.ImpactNormal.Rotation(), Hit.ImpactPoint));
		}

		if (Hit.GetActor() != nullptr)
		{
			ACHTakeHomeCharacter* HitPlayer = Cast<ACHTakeHomeCharacter>(Hit.GetActor());
			if (HitPlayer)
			{
				if (HasAuthority()) HitPlayer->TakeDamage(gunDamage);
				else HitPlayer->Server_TakeDamage(gunDamage);
			}
			else
			{
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Could not cast to takehomecharacter"));
			}
		}
	}

	//Spawning of particles at muzzle of gun (gun not yet implemented so won't work at this current moment)
	//if(MuzzleParticles)
	//{
	//	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleParticles, FP_Gun->GetSocketTransform(FName("Muzzle")));
	//}
}

void ACHTakeHomeCharacter::AimIn(const FInputActionValue& Value)
{
	CameraBoom->TargetArmLength = 200.f;
}

void ACHTakeHomeCharacter::AimOut(const FInputActionValue& Value)
{
	CameraBoom->TargetArmLength = 400.f;
}
