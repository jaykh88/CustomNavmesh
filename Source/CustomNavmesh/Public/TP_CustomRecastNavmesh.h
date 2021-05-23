// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NavMesh/RecastNavMesh.h"
#include "TP_CustomNavmeshEnums.h"
#include "TP_CustomRecastNavmesh.generated.h"

/**
 * 
 */
UCLASS()
class CUSTOMNAVMESH_API ATP_CustomRecastNavmesh : public ARecastNavMesh
{
	GENERATED_BODY()
	
public:

	ATP_CustomRecastNavmesh(const FObjectInitializer& ObjectInitializer);

	/** how many navlinks to spawn per edge currently its sat to (min=2, max=5) */
	UPROPERTY(EditAnywhere, Category = "NavLinkGenerator", meta=(ClampMin = "2", ClampMax = "5"))
	float EdgeDivisor;

	/** 
	* the height of the character's jump, this will be used as a threshold to generate a navlink or not
	* if the edge hight is heigher than the jump height links will not be generated 
	*/
	UPROPERTY(EditAnywhere, Category = "NavLinkGenerator")
	float JumpHeight;

	/** 
	* this angle is used to trace away from the edge 
	*/
	UPROPERTY(EditAnywhere, Category = "NavLinkGenerator")
	float SlantDegree;

	/**
	* Delete current navLinks if any, and generate new ones
	* else just generate links
	*/
	UPROPERTY(EditAnywhere, Category = "NavLinkGenerator")
	uint8 bRegenerateNavLink : 1;

	// gets called when navMesh tiles are updated is updated
	virtual void OnNavMeshTilesUpdated(const TArray<uint32>& ChangedTiles) override;
	//virtual void OnNavMeshGenerationFinished() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	
	void processNavMeshEdge(const TArray<uint32>& ChangedTiles);

	void GenerateLinkSpawnData(const FVector& EdgeStepVertex, const FVector& EdgePerpDir);

	void RegenerateNavLinks();

	void DeleteAllNavLinks();

	FORCEINLINE FVector GetPerpendicularClockwise(const FVector& Vector) const { return FVector(-Vector.Y, Vector.X, Vector.Z); }

	TArray<FNavLinkProxySpawnData> NavLinkSpawnData;

	UPROPERTY(EditAnywhere, Category = "NavLinkGenerator")
	TSubclassOf<class ATP_CustomNavLinkProxy> BPNavlinkClass;

	TArray<class ATP_CustomNavLinkProxy*> NavLinkReferences;
};
