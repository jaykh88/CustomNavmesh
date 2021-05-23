#pragma once

#include "CoreMinimal.h"
//#include "TP_CustomNavmeshEnums.generated.h"

/**
* This is a struct to hold the spawn data used for the navLinks
*/
struct FNavLinkProxySpawnData
{
	FVector SpawnLocation;
	FVector StartLocation;
	FVector EndLocation;

	FNavLinkProxySpawnData(const FVector& _SpawnLoc, const FVector& _StartLoc, const FVector& _EndLoc)
		: SpawnLocation(_SpawnLoc)
		, StartLocation(_StartLoc)
		, EndLocation(_EndLoc)
	{}

	// check if links are the same
	FORCEINLINE bool operator==(const FNavLinkProxySpawnData& right) const
	{
		if (StartLocation.Equals(right.StartLocation, 50.0f) && EndLocation.Equals(right.EndLocation, 50.0f))
			return true;

		return false;
	}
};