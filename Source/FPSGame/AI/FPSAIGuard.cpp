// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSAIGuard.h"

#include "Perception/PawnSensingComponent.h"
#include "DrawDebugHelpers.h"
#include "AI/Navigation/NavigationSystem.h"
#include "Net/UnrealNetwork.h"

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
	GuardController = GetController();

	if (bPatrol)
	{
		MoveNextPatrolPoint();
		UE_LOG(LogTemp, Warning, TEXT("Begin PatrolPoint"));
	}

}

// Called every frame
void AFPSAIGuard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Patrol Goal Check
	if (CurrentPatrolPoint)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tick PatrolPoint_1"));
		FVector Delta = GetActorLocation() - CurrentPatrolPoint->GetActorLocation();
		float DistanceToGoal = Delta.Size();

		// Check if we are within 50 units of our goal, if so pick a new patrol point
		if (DistanceToGoal < 100.f && bCanTick)
		{
			GetWorldTimerManager().ClearTimer(TimerHandle_ResetOrientation);
			GetWorldTimerManager().SetTimer(TimerHandle_ResetOrientation, this, &AFPSAIGuard::MoveNextPatrolPoint, 3.0f);
			bCanTick = false;
			UE_LOG(LogTemp, Warning, TEXT("Tick PatrolPoint"));
		}
	}

}

// Called to bind functionality to input
void AFPSAIGuard::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

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

	SetActorRotation(NewLookAt);

	SetAIState(EAIState::Alert);

	// Stop Movement if Patrolling
	if (GuardController)
		GuardController->StopMovement();

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

	SetActorRotation(NewLookAt);

	GetWorldTimerManager().ClearTimer(TimerHandle_ResetOrientation);
	GetWorldTimerManager().SetTimer(TimerHandle_ResetOrientation, this, &AFPSAIGuard::ResetOrientation, 3.0f);

	SetAIState(EAIState::Suspisious);

	// Stop Movement if Patrolling
	if (GuardController)
		GuardController->StopMovement();
}

void AFPSAIGuard::ResetOrientation()
{
	if (GuardState == EAIState::Alert)
		return;

	SetActorRotation(OriginalRotation);
	SetAIState(EAIState::Idle);

	// Stopped Investigating if we are a patrolling pawn, pick a new patrol point to move to
	if (bPatrol)
		MoveNextPatrolPoint();
}

void AFPSAIGuard::SetAIState(EAIState State)
{
	if (GuardState == State)
		return;

	GuardState = State;

	OnStateChanged(GuardState);
}

void AFPSAIGuard::MoveNextPatrolPoint()
{

	PatrolIndex = PatrolIndex % PatrolPoints.Num();
	CurrentPatrolPoint = PatrolPoints[PatrolIndex];

	UNavigationSystem::SimpleMoveToActor(GetController(), CurrentPatrolPoint);

	PatrolIndex++;
	bCanTick = true;

}



