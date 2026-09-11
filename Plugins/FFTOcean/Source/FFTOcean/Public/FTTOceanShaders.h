#pragma once

/*
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"


//Vertex shader setup
BEGIN_SHADER_PARAMETER_STRUCT(FGerstnerVSParams, )
END_SHADER_PARAMETER_STRUCT()
class FGerstnerVS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FGerstnerVS);
    using FParameters = FGerstnerVSParams;
    SHADER_USE_PARAMETER_STRUCT(FGerstnerVS, FGlobalShader);

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) {
        return true;
    }
};

//Fragment Shader Setup
BEGIN_SHADER_PARAMETER_STRUCT(FGerstnerPSParams, )
    RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()
class FGerstnerPS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FGerstnerPS);
    using FParameters = FGerstnerPSParams;
    SHADER_USE_PARAMETER_STRUCT(FGerstnerPS, FGlobalShader)
};



//-----------------

//Shader Data
struct FGerstnerVertex
{
    FVector2f Position;
    
};

//Rendering resources to hold and lay out the data on the GPU

class FGerstnerVertexBuffer : public FVertexBuffer
{
public:
    void InitRHI(FRHICommandListBase& RHICmdList);
};

class FTrianlgeVertexBufferElementDesc : public FRenderResource
{
public:
    FVertexDeclarationRHIRef VertexDeclarationRHI;
    virtual ~FTrianlgeVertexBufferElementDesc() {}

    virtual void InitRHI(FRHICommandListBase& RHICmdList);
    virtual void ReleaseRHI();
};

TGlobalResource<FGerstnerVertexBuffer> GTriangleVertexBuf;
TGlobalResource<FTrianlgeVertexBufferElementDesc> GTriangleVertexBufElementDesc;
*/
