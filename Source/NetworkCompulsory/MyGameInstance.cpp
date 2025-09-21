#include "MyGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"

void UMyGameInstance::HostLANGame(const FName MapName)
{
	FString Options = MapName.ToString() + TEXT("?listen");
	UGameplayStatics::OpenLevel(GetWorld(),MapName,true,Options);
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1,3.f,FColor::Red,TEXT("Hosting LAN"));
}

void UMyGameInstance::JoinLANGame(const FString& ServerAddress)
{
	if (APlayerController* PC = GetFirstLocalPlayerController(GetWorld()))
	{
		PC->ClientTravel(ServerAddress, TRAVEL_Absolute);
		if (GEngine)
		{
			FString Msg = FString::Printf(TEXT("Joining %s"), *ServerAddress);
			GEngine->AddOnScreenDebugMessage(-1,3.f,FColor::Red,Msg);
		}
	}
}


