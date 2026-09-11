// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Waves.generated.h"

class UProceduralMeshComponent;

UCLASS()
class AWaves : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWaves();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	//UStaticMeshComponent* VisualMesh;
	UPROPERTY(EditAnywhere)
	TArray<UProceduralMeshComponent*> Meshes;
	
	UPROPERTY(EditAnywhere)
	float TileSize;
	
	UPROPERTY(EditAnywhere)
	int32 AmountOfTiles;
	
	
	void CreateGrid(float tileSize, int32 amount, UProceduralMeshComponent*& mesh);
		

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
};
