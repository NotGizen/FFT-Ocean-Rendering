#pragma once


#include "CoreMinimal.h"

#include "RenderGraphUtils.h"
#include "Engine/TextureRenderTargetVolume.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"

struct FForceFieldCSParameters
{
	UTextureRenderTarget2D* RenderTarget;
	float Scale;
	float Angle;
	float SpreadBlend;
	float Swell;
	float Alpha;
	float PeakOmega;
	float Gamma;
	float ShortWavesFade;

	FIntVector2 GetRenderTargetSize() const
	{
		return CachedRenderTargetSize;
	}

	FForceFieldCSParameters()
	{
		
	}
	FForceFieldCSParameters(UTextureRenderTarget2D* IORenderTarget)
		: RenderTarget(IORenderTarget)
	{
		CachedRenderTargetSize = RenderTarget ? FIntVector2(RenderTarget->SizeX, RenderTarget->SizeY) : FIntVector2::ZeroValue;
	}


private:
	FIntVector2 CachedRenderTargetSize;

public:
};

/**
 * 
 */
class FFTOCEAN_API ForceField
{
public:
	ForceField();

	// Executes this shader on the render thread
	static void DispatchRenderThread(
		FRHICommandListImmediate& RHICmdList,
		FForceFieldCSParameters Params

	);

	static void Dispatch(
		FForceFieldCSParameters Params

	)
	{
		ENQUEUE_RENDER_COMMAND(SceneDrawCompletion)(
			[Params](FRHICommandListImmediate& RHICmdList)
			{
				DispatchRenderThread(RHICmdList, Params);
			});
	}

	void UpdateParameters(FForceFieldCSParameters& DrawParameters);

private:

	//Cached Shader Manager Parameters
	FForceFieldCSParameters cachedParams;
	//Whether we have cached parameters to pass to the shader or not
	volatile bool bCachedParamsAreValid;

	//Reference to a pooled render target where the shader will write its output
	TRefCountPtr<IPooledRenderTarget> ComputeShaderOutput;
};
