// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SAICharacter.h"

#include "DrawDebugHelpers.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/PawnSensingComponent.h"

ASAICharacter::ASAICharacter()
{
	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));
}

void ASAICharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	PawnSensingComp->OnSeePawn.AddDynamic(this, &ASAICharacter::OnPawnSeen);
}

void ASAICharacter::OnPawnSeen(APawn* Pawn)
{
	AAIController* AICon = Cast<AAIController>(GetController());
	if (AICon)
	{
		UBlackboardComponent* BBComp = AICon->GetBlackboardComponent();	// lol

		BBComp->SetValueAsObject(TEXT("TargetActor"), Pawn);

		DrawDebugString(GetWorld(), GetActorLocation(), "PLAYER SPOTTED", nullptr, FColor::White, 4.0f, true);
	}
}
 