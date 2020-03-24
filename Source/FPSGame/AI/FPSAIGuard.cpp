// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSAIGuard.h"

#include "Perception/PawnSensingComponent.h"
#include "DrawDebugHelpers.h"

#include "FPSGameMode.h"


// Sets default values
AFPSAIGuard::AFPSAIGuard()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// This need not attachment because it's not SceneComponent and has not location rotation and so on
	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComponent"));

	PawnSensingComp->OnSeePawn.AddDynamic(this, &AFPSAIGuard::OnPawnSeen);
	PawnSensingComp->OnHearNoise.AddDynamic(this, &AFPSAIGuard::OnPawnHearing);

	GuardState = EAIState::Idle;

}

// Called when the game starts or when spawned
void AFPSAIGuard::BeginPlay()
{
	Super::BeginPlay();
	
	OriginalRotation = GetActorRotation();

}

void AFPSAIGuard::OnPawnSeen(APawn * SeenPawn)
{
	if (!SeenPawn)
		return;

	
	DrawDebugSphere(GetWorld(), SeenPawn->GetActorLocation(), 40.f, 10.f, FColor::Yellow, false, 10.f);

	AFPSGameMode* GM = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode());
	if (GM && SeenPawn->IsControlled())
	{
		GM->CompleteMission(SeenPawn, false);
		SeenPawn->UnPossessed();
		
	}

	FVector Direction = GetActorLocation() - SeenPawn->GetActorLocation();
	Direction.Normalize();
	FRotator NewLookAt = FRotationMatrix::MakeFromX(Direction).Rotator();
	NewLookAt.Pitch = 0.f;
	NewLookAt.Roll = 0.f;

	SetAIState(EAIState::Alert, NewLookAt);

}

void AFPSAIGuard::OnPawnHearing(APawn* NoiseInstigator, const FVector & Location, float Volume)
{
	if (!NoiseInstigator && GuardState == EAIState::Alert)
		return;


	UE_LOG(LogTemp, Warning, TEXT("Hearing Perception"));
	DrawDebugSphere(GetWorld(), Location, 40.f, 10.f, FColor::Green, false, 10.f);

	FVector Direction = Location - GetActorLocation();
	Direction.Normalize();

	FRotator NewLookAt = FRotationMatrix::MakeFromX(Direction).Rotator();
	NewLookAt.Pitch = 0.f;
	NewLookAt.Roll = 0.f;

	//SetActorRotation(NewLookAt);

	GetWorldTimerManager().ClearTimer(TimerHandle_ResetOrientation);
	GetWorldTimerManager().SetTimer(TimerHandle_ResetOrientation, this, &AFPSAIGuard::ResetOrientation, 3.0f);

	SetAIState(EAIState::Suspisious, NewLookAt);
}

void AFPSAIGuard::ResetOrientation()
{
	if (GuardState == EAIState::Alert)
		return;

	//SetActorRotation(OriginalRotation);
	SetAIState(EAIState::Idle, OriginalRotation);
}

void AFPSAIGuard::SetAIState(EAIState State, FRotator LookAt)
{
	if (GuardState == State)
		return;

	GuardState = State;

	OnStateChanged(State, LookAt);
}

// Called every frame
void AFPSAIGuard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AFPSAIGuard::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

