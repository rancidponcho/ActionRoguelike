// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"

#include "DrawDebugHelpers.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SInteractionComponent.h"
#include "SAttributeComponent.h"

#include "Math/UnrealMathUtility.h"
#include "CoreFwd.h"


// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->bDoCollisionTest = false;
	SpringArmComp->TargetArmLength = 250.f;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);
	CameraComp->FieldOfView = 75.f;

	InteractionComp = CreateDefaultSubobject<USInteractionComponent>(TEXT("InteractionComp"));

	AttributeComp = CreateDefaultSubobject<USAttributeComponent>(TEXT("AttributeComp"));

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;

	AttackAnimDelay = 0.169;
}

void ASCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	AttributeComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);
}

void ASCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	/* --- Reticle placement --- */
	FVector Vel = GetVelocity();
	FRotator Rot = CameraComp->GetComponentRotation();
	Vel = Rot.UnrotateVector(Vel);
	float TargetY;
	float InterpSpeed = 2.f;
	(Vel.Y > 5.0f || Vel.Y < -5.0f) ? TargetY = Vel.Y * 0.25f : TargetY = 75.f * ((SpringArmComp->SocketOffset.Y > 0.f) - (SpringArmComp->SocketOffset.Y < 0.f));
	SpringArmComp->SocketOffset.Y = FMath::FInterpTo(SpringArmComp->SocketOffset.Y, TargetY, GetWorld()->GetDeltaSeconds(), InterpSpeed);
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	/* --- Player --- */
	PlayerInputComponent->BindAction("PrimaryInteract", IE_Pressed, this, &ASCharacter::PrimaryInteract);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAction("PrimaryAttack", IE_Pressed, this, &ASCharacter::PrimaryAttack);
	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &ASCharacter::Dash);
	PlayerInputComponent->BindAction("BlackHole", IE_Pressed, this, &ASCharacter::BlackHole);

	/* ----------// Designer Tools //---------- */
}

void ASCharacter::OnHealthChanged(AActor* InstigatorActor, USAttributeComponent* OwningComp, float NewHealth, float Delta)
{
	if (NewHealth <= 0.0f && Delta < 0.0f)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		DisableInput(PC);
	}
}

void ASCharacter::MoveForward(float Value)
{
	FRotator ControlRot = GetControlRotation();
	ControlRot.Pitch = 0.0f;
	ControlRot.Roll = 0.0f;
	AddMovementInput(ControlRot.Vector(), Value);
}

void ASCharacter::MoveRight(float Value)
{
	FRotator ControlRot = GetControlRotation();
	ControlRot.Pitch = 0.0f;
	ControlRot.Roll = 0.0f;
	FVector RightVector = FRotationMatrix(ControlRot).GetScaledAxis(EAxis::Y);
	AddMovementInput(RightVector, Value);
}

void ASCharacter::PrimaryInteract()
{
	if (InteractionComp) {
		InteractionComp->PrimaryInteract();
	}
}

void ASCharacter::PrimaryAttack()
{
	// Primary attack animation
	PlayAnimMontage(AttackAnim);

	// Lets animation play before spawning projectile
	GetWorldTimerManager().SetTimer(TimerHandle_PrimaryAttack, this, &ASCharacter::PrimaryAttack_TimeElapsed, AttackAnimDelay);
}

void ASCharacter::PrimaryAttack_TimeElapsed()
{
	SpawnProjectile(ProjectileClass);
}

void ASCharacter::Dash()
{
	// Primary attack animation
	PlayAnimMontage(AttackAnim);

	// Lets animation play before spawning projectile
	GetWorldTimerManager().SetTimer(TimerHandle_Dash, this, &ASCharacter::Dash_TimeElapsed, AttackAnimDelay);
}

void ASCharacter::Dash_TimeElapsed()
{
	SpawnProjectile(DashProjectileClass);
}

void ASCharacter::BlackHole()
{
	// Primary attack animation
	PlayAnimMontage(AttackAnim);

	// Lets animation play before spawning projectile
	GetWorldTimerManager().SetTimer(TimerHandle_BlackHoleAttack, this, &ASCharacter::BlackHole_TimeElapsed, AttackAnimDelay);
}

void ASCharacter::BlackHole_TimeElapsed()
{
	SpawnProjectile(BlackHoleProjectileClass);
}

void ASCharacter::SpawnProjectile(TSubclassOf<AActor> ClassToSpawn)
{
	if (ensureAlways(ClassToSpawn))
	{
		// Initializes spawn location from hand socket
		FVector HandLocation = GetMesh()->GetSocketLocation("Muzzle_01");

		// Finds intended impact point on camera angle
		FVector TraceStart = CameraComp->GetComponentLocation();
		FVector TraceEnd = TraceStart + (CameraComp->GetComponentRotation().Vector() * 50000);	// IDK what max hit distance should be

		// Trace shape
		FCollisionShape Shape;
		// Shape.SetSphere(20.0f);	// For sweep. Consider passing radius as parameter

		// Ignore player
		FCollisionQueryParams ColParams;
		ColParams.AddIgnoredActor(this);

		// Sets what object types hit by line trace 
		FCollisionObjectQueryParams ObjParams;
		ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);
		ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);
		ObjParams.AddObjectTypesToQuery(ECC_Pawn);

		// Misc.
		FHitResult Hit;
		FTransform SpawnTM;

		// On hit finds projectile vector based on difference between intended collision & hand location
		if (GetWorld()->SweepSingleByObjectType(Hit, TraceStart, TraceEnd, FQuat::Identity, ObjParams, Shape, ColParams))		// sweep: GetWorld()->SweepSingleByObjectType(Hit, TraceStart, TraceEnd, FQuat::Identity, ObjParams, Shape, ColParams) trace: GetWorld()->LineTraceSingleByObjectType(Hit, TraceStart, TraceEnd, ObjParams, ColParams)
		{
			SpawnTM = FTransform((Hit.Location - HandLocation).Rotation(), HandLocation);

			// DEBUG
			DrawDebugString(GetWorld(), Hit.ImpactPoint, FString::Printf(TEXT("Line Trace: %s"), *Hit.ImpactPoint.ToString()), nullptr, FColor::Green, 2.0f, true);
		}
		else
		{
			SpawnTM = FTransform(GetControlRotation(), HandLocation);

			//DEBUG
			DrawDebugString(GetWorld(), HandLocation, TEXT("NO HIT"), nullptr, FColor::Red, 2.0f, true);
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Instigator = this;	// Used in BP to disable collision with this character

		// Spawns projectile
		GetWorld()->SpawnActor<AActor>(ClassToSpawn, SpawnTM, SpawnParams);
	}
}

/* ----------// Designer Tools //---------- */
