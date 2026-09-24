#include "WavesRender.h"

#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "RenderTargetPool.h"
#include "RHI.h"

#include "Modules/ModuleManager.h"

#define NUM_THREADS_PER_GROUP_DIMENSION 8 //Change this after

class FSpectrum : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FSpectrum)
	SHADER_USE_PARAMETER_STRUCT(FSpectrum, FGlobalShader)
	
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, OutputTexture)
		SHADER_PARAMETER(float, scale)
		SHADER_PARAMETER(float, angle)
		SHADER_PARAMETER(float, spreadBlend)
		SHADER_PARAMETER(float, swell)
		SHADER_PARAMETER(float, alpha)
		SHADER_PARAMETER(float, peakOmega)
		SHADER_PARAMETER(float, gamma)
		SHADER_PARAMETER(float, shortWavesFade)

	END_SHADER_PARAMETER_STRUCT()
	

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		//We're using it here to add some preprocessor defines. That way we don't have to change both C++ and HLSL code when we change the value for NUM_THREADS_PER_GROUP_DIMENSION
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), 1);
	}
};
IMPLEMENT_GLOBAL_SHADER(FSpectrum, "/CustomShaders/Private/FFTWaves.usf", "MainCS", SF_Compute);

class FConjugate : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FConjugate)
	SHADER_USE_PARAMETER_STRUCT(FConjugate, FGlobalShader)
	
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, OutputTexture)
	END_SHADER_PARAMETER_STRUCT()
	

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
	
};
IMPLEMENT_GLOBAL_SHADER(FConjugate, "/CustomShaders/Private/FFTWaves.usf", "ConjugateCS", SF_Compute);


WavesRender::WavesRender()
{
}



void WavesRender::UpdateParameters(SpectrumParameters& DrawParameters)
{
	cachedParams = DrawParameters;
	bCachedParamsAreValid = true;
}

void WavesRender::DispatchRenderThread(FRHICommandListImmediate& RHICmdList, SpectrumParameters Params)
{
	FRDGBuilder GraphBuilder(RHICmdList);
	
	FRDGTextureRef OutTexture = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(Params.RenderTarget->GetRenderTargetResource()->TextureRHI, TEXT("HeightField")));
	// This is a pointer to the shader-parameters we declared in the .h
	FSpectrum::FParameters* PassParameters;
	// We ask the RDG to allocate some memory for our shader-parameters
	PassParameters = GraphBuilder.AllocParameters<FSpectrum::FParameters>();

	PassParameters->OutputTexture = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(OutTexture));
	PassParameters->scale = Params.Scale;
	PassParameters->alpha = Params.Alpha;
	PassParameters->angle = Params.Angle;
	PassParameters->gamma = Params.Gamma;
	PassParameters->peakOmega = Params.PeakOmega;
	PassParameters->shortWavesFade = Params.ShortWavesFade;
	PassParameters->spreadBlend = Params.SpreadBlend;
	PassParameters->swell = Params.Swell;
	
	//Get a reference to our shader type from global shader map
	//Pass 1 (static initial spectrum texture)
	TShaderMapRef<FSpectrum> SpectrumCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("Waves Pass"), SpectrumCS, PassParameters, 
		FIntVector(
	FMath::DivideAndRoundUp(Params.GetRenderTargetSize().X, NUM_THREADS_PER_GROUP_DIMENSION),
	FMath::DivideAndRoundUp(Params.GetRenderTargetSize().Y, NUM_THREADS_PER_GROUP_DIMENSION),
	1
));
	//Pass 2 (Conjugate)
	FConjugate::FParameters* PassParameters2 = GraphBuilder.AllocParameters<FConjugate::FParameters>();
	
	PassParameters2->OutputTexture = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(OutTexture));
	TShaderMapRef<FConjugate> ConjugateCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("Conjugate Pass"), ConjugateCS, PassParameters2, 
		FIntVector(
	FMath::DivideAndRoundUp(Params.GetRenderTargetSize().X, NUM_THREADS_PER_GROUP_DIMENSION),
	FMath::DivideAndRoundUp(Params.GetRenderTargetSize().Y, NUM_THREADS_PER_GROUP_DIMENSION),
	1));
	
	GraphBuilder.Execute();
}
//Conjugate Shader

/*class FConjugateCS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FConjugateCS)
	SHADER_USE_PARAMETER_STRUCT(FConjugateCS, FGlobalShader)

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, OutputTexture)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
};
IMPLEMENT_GLOBAL_SHADER(FConjugateCS, "/CustomShaders/Private/FFTWaves.usf", "ConjugateCS", SF_Compute);*/
