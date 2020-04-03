// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSExtractionZone.h"

#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"
#include "Kismet/GameplayStatics.h"

#include "FPSCharacter.h"
#include "FPSGameMode.h"


// Sets default values
AFPSExtractionZone::AFPSExtractionZone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapComponent"));
	BoxComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoxComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	BoxComp->SetBoxExtent(FVector(200.f));
	RootComponent = BoxComp;

	BoxComp->OnComponentBeginOverlap.AddDynamic(this, &AFPSExtractionZone::HandleOverlap);

	// For Debug
	BoxComp->SetHiddenInGame(false);


	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComponent"));
	DecalComp->DecalSize = FVector(200.f);
	DecalComp->SetupAttachment(RootComponent);

	
}

void AFPSExtractionZone::HandleOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, 
	UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	AFPSCharacter* PlayerPawn = Cast<AFPSCharacter>(OtherActor);
	if (!PlayerPawn)
		return;

	if (PlayerPawn->bIsCarryingObjective)
	{
		/// GetAuthGameMode() == that this function will be executed only for Server
		/// this is meaningful only MultiPlayer Game. In MultiPlayer Game this function Executed only Server
		/// for Client GetAuthGameMode() = NULL
		AFPSGameMode* GM = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			GM->CompleteMission(PlayerPawn, true);
		}
	}
	else
	{
		/// GamePlayStatics is a Class which include all Static functions
		/// Some of that function returns UObject type but in case that static functions are singleton
		/// it's return Object does not exists in Level
		UGameplayStatics::PlaySound2D(this, ObjectiveMissingSOund);
		UE_LOG(LogTemp, Warning, TEXT("Extraction Zone"));
	}


}

// Called when the game starts or when spawned
void AFPSExtractionZone::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFPSExtractionZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

