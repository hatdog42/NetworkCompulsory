#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKCOMPULSORY_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
	public:
		UFUNCTION(BlueprintCallable)
		void HostLANGame(const FName MapName);
	
		UFUNCTION(BlueprintCallable)
		void JoinLANGame(const FString& ServerAddress);
};
