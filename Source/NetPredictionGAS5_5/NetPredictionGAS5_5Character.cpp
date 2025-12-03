// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetPredictionGAS5_5Character.h"

#include "EngineUtils.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "NetworkPredictionLagCompensation.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ANetPredictionGAS5_5Character

ANetPredictionGAS5_5Character::ANetPredictionGAS5_5Character()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
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
}

//////////////////////////////////////////////////////////////////////////
// Input

void ANetPredictionGAS5_5Character::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ANetPredictionGAS5_5Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ANetPredictionGAS5_5Character::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ANetPredictionGAS5_5Character::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ANetPredictionGAS5_5Character::DoSomething()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Find nearest AI with a head bone
    const FName HeadBoneName = FName("head");

    APawn* BestTarget = nullptr;
    float BestDistSq = FLT_MAX;

    for (TActorIterator<APawn> It(World); It; ++It)
    {
        APawn* Pawn = *It;

    	if (Pawn == this)
    	{
    		continue;
    	}
    	
        const float DistSq = FVector::DistSquared(Pawn->GetActorLocation(), GetActorLocation());
        if (DistSq < BestDistSq && DistSq < 50000 * 50000)
        {
            BestDistSq = DistSq;
            BestTarget = Pawn;
        	break;
        }
    }

    if (!BestTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("[AutoShoot] No target found"));
        return;
    }

    USkeletalMeshComponent* TargetMesh = BestTarget->FindComponentByClass<USkeletalMeshComponent>();
    if (!TargetMesh) return;

    const FVector HeadLocation = TargetMesh->GetSocketLocation(HeadBoneName);

    // Calculate aim direction toward head
    FVector CamLoc;
    FRotator CamRot;
    PC->GetPlayerViewPoint(CamLoc, CamRot);

    FVector AimDirection = (HeadLocation - CamLoc).GetSafeNormal();
    FVector TraceEnd = CamLoc + AimDirection * 50000;

    // Draw debug view line

    DrawDebugLine(
        World,
        CamLoc,
        TraceEnd,
        FColor::Red,
        false,
        0.25f,
        0,
        1.0f
    );
	
    // Line trace (local prediction)
    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(AutoShoot), false, this);

    bool bHit = World->LineTraceSingleByChannel(
        Hit,
        CamLoc,
        TraceEnd,
        ECC_Visibility,
        Params
    );

	if (!bHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("FAILED TO HIT"));
		return;
	}

	AActor* HitActor = Hit.GetActor();
	if (!HitActor)return;

	UPrimitiveComponent* PrimComp = Hit.GetComponent();
	if (!PrimComp) return;
	
	APawn* PPawn = Cast<APawn>(Hit.GetActor());
	if (!PPawn) return;
	
	// UNetworkPredictionLagCompensation* LagCompensation;
	// if (!LagCompensation) return;

	// UMoverComponent* MoverComponent = HitActor->FindComponentByClass<UMoverComponent>();
	// if (!MoverComponent) return;
	
	// FMoverTimeStep TimeStep = MoverComponent->GetLastTimeStep();
	//
	// if (GEngine)
	// {
	// 	GEngine->AddOnScreenDebugMessage(123, 10.f, FColor::Red, FString::Printf(TEXT("TimeStep: %f"), TimeStep.BaseSimTimeMs));
	// }
	
				
	// DrawDebugCapsule(GetWorld(), Cap->GetComponentLocation(), Cap->GetUnscaledCapsuleHalfHeight(), Cap->GetUnscaledCapsuleRadius(),
	// 		 Cap->GetComponentRotation().Quaternion(), bHit ? FColor::Green : FColor::Red,
	// 		 false, 10.f);
	//
	// DrawDebugPoint(
	//  GetWorld(),
	//  Hit.ImpactPoint,
	//  10.0f,
	//  FColor::Red,
	//  false,
	//  10.0f);
	//
	// DrawDebugSphere(
	//  GetWorld(),
	//  HeadLocation,
	//  10.0f,
	//  6,
	//  FColor::Blue,
	//  false,
	//  10.0f);


	// GetLagCompensationTimeMS
	// UNetworkPrediction
	
	// ServerKillButtonPressed(PChar, CorrectedShotTime, CamLoc, TraceEnd);
}

void ANetPredictionGAS5_5Character::Move(const FInputActionValue& Value)
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

void ANetPredictionGAS5_5Character::Look(const FInputActionValue& Value)
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
