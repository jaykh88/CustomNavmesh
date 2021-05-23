// Fill out your copyright notice in the Description page of Project Settings.
#include "TP_CustomNavLinkProxy.h"
#include <NavLinkCustomComponent.h>




ATP_CustomNavLinkProxy::ATP_CustomNavLinkProxy(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetActorTickEnabled(false);

	PointLinks.Empty();

	bSmartLinkIsRelevant = true;

#if WITH_EDITORONLY_DATA
	StartEditorComp = CreateDefaultSubobject<USphereComponent>(TEXT("Start"));
	StartEditorComp->InitSphereRadius(30.0f);
	StartEditorComp->SetupAttachment(RootComponent);
	StartEditorComp->SetRelativeLocation(FVector(250.0f, 0, 0));
	StartEditorComp->SetCollisionProfileName("NoCollision");
	StartEditorComp->SetGenerateOverlapEvents(false);
	StartEditorComp->SetCanEverAffectNavigation(false);


	EndEditorComp = CreateDefaultSubobject<USphereComponent>(TEXT("End"));
	EndEditorComp->InitSphereRadius(30.0f);
	EndEditorComp->SetupAttachment(RootComponent);
	EndEditorComp->SetRelativeLocation(FVector(-250.0f, 0, 0));
	StartEditorComp->SetCollisionProfileName("NoCollision");
	EndEditorComp->SetGenerateOverlapEvents(false);
	EndEditorComp->SetCanEverAffectNavigation(false);
#endif
}

#if WITH_EDITOR
void ATP_CustomNavLinkProxy::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{

#if WITH_EDITORONLY_DATA
	UNavLinkCustomComponent* const LinkComp = GetSmartLinkComp();
	if (LinkComp)
	{
		StartEditorComp->SetWorldLocation(LinkComp->GetStartPoint());
		EndEditorComp->SetWorldLocation(LinkComp->GetEndPoint());
	}
#endif

	SyncLinkDataToComponents();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void ATP_CustomNavLinkProxy::PostEditMove(bool bFinished)
{
	SyncLinkDataToComponents();
	Super::PostEditMove(bFinished);
}

void ATP_CustomNavLinkProxy::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();

#if WITH_EDITORONLY_DATA
	if (!bRegisteredCallbacks && !IsTemplate())
	{
		bRegisteredCallbacks = true;
		StartEditorComp->TransformUpdated.AddUObject(this, &ATP_CustomNavLinkProxy::OnChildTransformUpdated);
		EndEditorComp->TransformUpdated.AddUObject(this, &ATP_CustomNavLinkProxy::OnChildTransformUpdated);
	}
#endif
}

void ATP_CustomNavLinkProxy::SyncLinkDataToComponents()
{
#if WITH_EDITORONLY_DATA
	FVector const NewLinkRelativeStart = GetActorTransform().InverseTransformPosition(StartEditorComp->GetComponentLocation());
	FVector const NewLinkRelativeEnd = GetActorTransform().InverseTransformPosition(EndEditorComp->GetComponentLocation());

	FVector LeftPt, RightPt;
	ENavLinkDirection::Type Direction;

	GetSmartLinkComp()->GetLinkData(LeftPt, RightPt, Direction);
	GetSmartLinkComp()->SetLinkData(NewLinkRelativeStart, NewLinkRelativeEnd, Direction);
#endif
}

void ATP_CustomNavLinkProxy::OnChildTransformUpdated(USceneComponent* UpdatedComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType TeleportType)
{
	SyncLinkDataToComponents();
}
#endif

void ATP_CustomNavLinkProxy::SetEditorCompLocation(const FVector& StartCompLoc, const FVector& EndCompLoc)
{
	StartEditorComp->SetWorldLocation(StartCompLoc);
	EndEditorComp->SetWorldLocation(EndCompLoc);
	SyncLinkDataToComponents();
}
