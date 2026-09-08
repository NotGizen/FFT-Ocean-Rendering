#pragma once
#include "SceneViewExtension.h"

class TRIANGLETEST_API FHelloTriangleViewExtension : public FSceneViewExtensionBase
{
public:
	FHelloTriangleViewExtension(const FAutoRegister& AutoRegister);
	
	virtual void PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& InView, const FPostProcessingInputs& Inputs) override;
};
