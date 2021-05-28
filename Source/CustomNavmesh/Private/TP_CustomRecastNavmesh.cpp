// this is me
// Fill out your copyright notice in the Description page of Project Settings.

#include "TP_CustomRecastNavmesh.h"
#include "Engine/World.h"
#include "TP_CustomNavmeshEnums.h"
#include "TP_CustomNavLinkProxy.h"
#include "Kismet/KismetSystemLibrary.h"

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST) || WITH_EDITOR
#include "DrawDebugHelpers.h"
#endif

ATP_CustomRecastNavmesh::ATP_CustomRecastNavmesh(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	EdgeDivisor = 2.f;
	JumpHeight = 500.f;
	SlantDegree = 30.f;
	bRegenerateNavLink = false;

// 	static ConstructorHelpers::FObjectFinder<UClass> BPNavLink(TEXT("/Plugins/CustomNavmesh/Content/BP_CustomNavLink.BP_CustomNavLink_C"));
//
// 	ensure(BPNavLink.Object);
//
// 	BPNavlinkClass = BPNavLink.Object;
}

void ATP_CustomRecastNavmesh::OnNavMeshTilesUpdated(const TArray<uint32>& ChangedTiles)
{
	processNavMeshEdge(ChangedTiles);
}

#if WITH_EDITOR
void ATP_CustomRecastNavmesh::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (bRegenerateNavLink)
	{
		DeleteAllNavLinks();
		RegenerateNavLinks();
		NavLinkSpawnData.Empty();

		bRegenerateNavLink = false;
	}
}
#endif

void ATP_CustomRecastNavmesh::processNavMeshEdge(const TArray<uint32>& ChangedTiles)
{
	FRecastDebugGeometry DebugNavGeo;
	DebugNavGeo.bGatherNavMeshEdges = true;

	for (auto const& TileIndex : ChangedTiles)
	{
		BeginBatchQuery();
		GetDebugGeometry(DebugNavGeo, TileIndex);
		FinishBatchQuery();

		const TArray<FVector>& NavMeshEdgeVerts = DebugNavGeo.NavMeshEdges;
		for (int32 i = 0; i < NavMeshEdgeVerts.Num(); i += 2)
		{
			const FVector EdgeStart = NavMeshEdgeVerts[i];
			const FVector EdgeEnd = NavMeshEdgeVerts[i + 1];
			const FVector Edge = EdgeEnd - EdgeStart;
			const FVector EdgeDir = Edge.GetSafeNormal();

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST) || WITH_EDITOR
			UKismetSystemLibrary::DrawDebugArrow(GetWorld(), EdgeStart, EdgeEnd, 200.f, FLinearColor::Yellow, 30.f, 5.f);
#endif
			const float EdgeSteps = Edge.Size() / EdgeDivisor;
			// TODO: Revise this
			for (float EdgeStep = EdgeSteps; EdgeStep < Edge.Size(); EdgeStep+= EdgeSteps)
			{
				GenerateLinkSpawnData((EdgeStart + (EdgeDir * EdgeStep)), GetPerpendicularClockwise(EdgeDir));
			}
		}
	}
}

void ATP_CustomRecastNavmesh::GenerateLinkSpawnData(const FVector & EdgeStepVertex, const FVector & EdgePerpDir)
{
	const FVector TraceEnd = EdgeStepVertex + (EdgePerpDir * 100.f);

	FHitResult Hit;
	// Trace away from edge to see if you can generate a link
	bool bSuccess = GetWorld()->LineTraceSingleByChannel(Hit, EdgeStepVertex, TraceEnd, ECollisionChannel::ECC_Visibility);

	if (!bSuccess)
	{
		// Direction towards the outside of the edge
		FVector EdgeToOutside = (TraceEnd - EdgeStepVertex).GetSafeNormal();
		// Get Y direction
		FVector YDirection = FVector::CrossProduct(FVector(0, 0, 1), EdgeToOutside);
		// Get Z direction
		FVector ZDirection = FVector::CrossProduct(EdgeToOutside, YDirection);
		// Create a rotation matrix to use for checking for the floor
		FMatrix RotationMatrix = FMatrix(EdgeToOutside, YDirection, ZDirection, FVector4(0, 0, 0, 1));

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST) || WITH_EDITOR
		UKismetSystemLibrary::DrawDebugArrow(this, EdgeStepVertex, TraceEnd, 200.0f, FColor::Cyan, 30.0f, 5.0f);

		UKismetSystemLibrary::DrawDebugArrow(this, TraceEnd, TraceEnd + RotationMatrix.TransformPosition(FVector(1, 0, 0)) * 50.0f, 200.0f, FColor::Red, 30.0f, 5.0f);
		UKismetSystemLibrary::DrawDebugArrow(this, TraceEnd, TraceEnd + RotationMatrix.TransformPosition(FVector(0, 1, 0)) * 50.0f, 200.0f, FColor::Green, 30.0f, 5.0f);
		UKismetSystemLibrary::DrawDebugArrow(this, TraceEnd, TraceEnd + RotationMatrix.TransformPosition(FVector(0, 0, 1)) * 50.0f, 200.0f, FColor::Blue, 30.0f, 5.0f);
#endif

		FVector TraceToFloorDirection = FVector::UpVector.RotateAngleAxis(-SlantDegree, RotationMatrix.TransformPosition(FVector(0, 1, 0)));

		bSuccess = GetWorld()->LineTraceSingleByChannel(Hit, TraceEnd, TraceEnd - TraceToFloorDirection * JumpHeight, ECollisionChannel::ECC_Visibility);

		if (bSuccess)
		{
			// if the agent can step on the surface, ignore
			if (Hit.Actor->IsValidLowLevelFast())
			{
				auto const& bDontGeneratLinksEnd = Hit.Actor->ActorHasTag(FName("NoLinks"));
				if (bDontGeneratLinksEnd)
				{
					return;
				}
			}

			auto const& DistanceBetweenNodes = FVector::Distance(TraceEnd, Hit.Location);
			if (TraceEnd.Z - Hit.Location.Z < AgentMaxStepHeight || DistanceBetweenNodes < 300.f)
			{
				return;
			}

			NavLinkSpawnData.AddUnique(FNavLinkProxySpawnData((EdgeStepVertex - Hit.Location) / 2.0f, EdgeStepVertex, Hit.Location));

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST) || WITH_EDITOR
			// debug floor hit location
			UKismetSystemLibrary::DrawDebugArrow(this, TraceEnd, Hit.Location, 200.0f, FColor::Cyan, 30.0f, 5.0f);
			UKismetSystemLibrary::DrawDebugSphere(this, Hit.Location, 25.0f, 6, FColor::Cyan, 30.0f, 5.0f);
#endif
		}
	}
}

void ATP_CustomRecastNavmesh::RegenerateNavLinks()
{
	for (auto const& LinkData : NavLinkSpawnData)
	{
		auto NavLink = GetWorld()->SpawnActor<ATP_CustomNavLinkProxy>(BPNavlinkClass, FTransform(LinkData.SpawnLocation));
		if (!NavLink)
		{
			UE_LOG(LogTemp, Warning, TEXT("ATP_CustomRecastNavmesh::RegenerateNavLinks - Spawn failed Navlink is invalid. "));
			continue;
		}
		NavLink->SetEditorCompLocation(LinkData.StartLocation, LinkData.EndLocation);
		NavLinkReferences.AddUnique(NavLink);

#if WITH_EDITOR
		NavLink->SetFolderPath("/JumpLinks");
#endif
	}
}

void ATP_CustomRecastNavmesh::DeleteAllNavLinks()
{
	for (auto const& NavLink : NavLinkReferences)
	{
		if (GetWorld() && NavLink)
		{
			GetWorld()->DestroyActor(Cast<AActor>(NavLink));
		}
	}
}