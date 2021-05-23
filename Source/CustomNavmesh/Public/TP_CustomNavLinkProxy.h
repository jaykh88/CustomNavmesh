// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIModule/Classes/Navigation/NavLinkProxy.h"
#include <Components/SphereComponent.h>
#include "TP_CustomNavLinkProxy.generated.h"

/**
 * 
 */
UCLASS()
class CUSTOMNAVMESH_API ATP_CustomNavLinkProxy : public ANavLinkProxy
{
	GENERATED_BODY()
	
public:

	ATP_CustomNavLinkProxy(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditMove(bool bFinished) override;
	virtual void PostRegisterAllComponents() override;

	virtual void SyncLinkDataToComponents();
	virtual void OnChildTransformUpdated(USceneComponent* UpdatedComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType TeleportType);
	bool bRegisteredCallbacks;
#endif

	void SetEditorCompLocation(const FVector& StartCompLoc, const FVector& EndCompLoc);

	FVector GetStartCompLocation() const { return StartEditorComp->GetComponentLocation(); }
	FVector GetEndCompLocation() const { return EndEditorComp->GetComponentLocation(); }

protected:
#if WITH_EDITORONLY_DATA

	UPROPERTY(EditAnywhere, Category = "AI")
	USphereComponent* StartEditorComp;

	UPROPERTY(EditAnywhere, Category = "AI")
	USphereComponent* EndEditorComp;
#endif
};
