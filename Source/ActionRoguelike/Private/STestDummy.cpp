// Fill out your copyright notice in the Description page of Project Settings.


#include "STestDummy.h"

#include "Components/SkeletalMeshComponent.h"
#include "SAttributeComponent.h"

// Sets default values
ASTestDummy::ASTestDummy()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>("MeshComp");
	RootComponent = MeshComp;

	AttributeComp = CreateDefaultSubobject<USAttributeComponent>("AttributeComp");
}

void ASTestDummy::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	AttributeComp->OnHealthChanged.AddDynamic(this, &ASTestDummy::OnHealthChanged);
}

void ASTestDummy::OnHealthChanged(AActor* InstigatorActor, USAttributeComponent* OwningComp, float NewHealth, float Delta)
{
	if (Delta < 0.0f)
	{
		MeshComp->SetScalarParameterValueOnMaterials("TimeToHit", GetWorld()->TimeSeconds);
	}
}

