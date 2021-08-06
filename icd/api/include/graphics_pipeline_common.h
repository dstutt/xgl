/*
 ***********************************************************************************************************************
 *
 *  Copyright (c) 2021 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *
 **********************************************************************************************************************/

#ifndef __GRAPHICS_PIPELINE_COMMON_H__
#define __GRAPHICS_PIPELINE_COMMON_H__

#pragma once

#include "include/vk_pipeline.h"
#include "palColorBlendState.h"
#include "palDepthStencilState.h"
#include "palMetroHash.h"

namespace vk
{

struct PipelineOptimizerKey;
struct GraphicsPipelineBinaryCreateInfo;
struct GraphicsPipelineShaderStageInfo;

// =====================================================================================================================
// Sample pattern structure containing pal format sample locations and sample counts
// ToDo: Move this struct to different header once render_graph implementation is removed.
struct SamplePattern
{
    Pal::MsaaQuadSamplePattern locations;
    uint32_t                   sampleCount;
};

// =====================================================================================================================
// Information required by the VB table manager that is defined by the graphics pipeline
struct VbBindingInfo
{
    uint32_t bindingTableSize;
    uint32_t bindingCount;

    struct
    {
        uint32_t slot;
        uint32_t byteStride;
    } bindings[Pal::MaxVertexBuffers];
};

// =====================================================================================================================
// Immediate state info that will be written during Bind() but is not
// encapsulated within a state object.
struct GraphicsPipelineObjectImmedInfo
{
    Pal::InputAssemblyStateParams         inputAssemblyState;
    Pal::TriangleRasterStateParams        triangleRasterState;
    Pal::BlendConstParams                 blendConstParams;
    Pal::DepthBiasParams                  depthBiasParams;
    Pal::DepthBoundsParams                depthBoundParams;
    Pal::PointLineRasterStateParams       pointLineRasterParams;
    Pal::LineStippleStateParams           lineStippleParams;
    Pal::ViewportParams                   viewportParams;
    Pal::ScissorRectParams                scissorRectParams;
    Pal::StencilRefMaskParams             stencilRefMasks;
    SamplePattern                         samplePattern;
    Pal::DynamicGraphicsShaderInfos       graphicsShaderInfos;
    Pal::VrsRateParams                    vrsRateParams;
    Pal::DepthStencilStateCreateInfo      depthStencilCreateInfo;
    bool                                  rasterizerDiscardEnable;

    // Static pipeline parameter token values.  These can be used to efficiently redundancy check static pipeline
    // state programming during pipeline binds.
    struct
    {
        uint32_t inputAssemblyState;
        uint32_t triangleRasterState;
        uint32_t pointLineRasterState;
        uint32_t lineStippleState;
        uint32_t depthBias;
        uint32_t blendConst;
        uint32_t depthBounds;
        uint32_t viewport;
        uint32_t scissorRect;
        uint32_t samplePattern;
        uint32_t fragmentShadingRate;
    } staticTokens;
};

// =====================================================================================================================
// Creation info parameters for all the necessary state objects encapsulated
// by the Vulkan graphics pipeline.
struct GraphicsPipelineObjectCreateInfo
{
    Pal::GraphicsPipelineCreateInfo             pipeline;
    Pal::MsaaStateCreateInfo                    msaa;
    Pal::ColorBlendStateCreateInfo              blend;
    Pal::DepthStencilStateCreateInfo            ds;
    GraphicsPipelineObjectImmedInfo             immedInfo;
    uint32_t                                    staticStateMask;
    const PipelineLayout*                       pLayout;
    uint32_t                                    sampleCoverage;
    VkShaderStageFlagBits                       activeStages;
    VkFormat                                    dbFormat;

    union
    {
        struct
        {
            uint32_t   bresenhamEnable         : 1;
            uint32_t   bindDepthStencilObject  : 1;
            uint32_t   bindTriangleRasterState : 1;
            uint32_t   bindStencilRefMasks     : 1;
            uint32_t   bindInputAssemblyState  : 1;
            uint32_t   customMultiSampleState  : 1;
            uint32_t   customSampleLocations   : 1;
            uint32_t   force1x1ShaderRate      : 1;
            uint32_t   sampleShadingEnable     : 1;
            uint32_t   reserved                : 23;
        };
        uint32_t value;
    } flags;
};

// =====================================================================================================================
// Include pipeline binary information from compiler which affects information info of pipeline object
struct GraphicsPipelineBinaryInfo
{
    const PipelineOptimizerKey* pOptimizerKey;
};

// =====================================================================================================================
// The common part used by both executable graphics pipelines and graphics pipeline libraries
class GraphicsPipelineCommon : public Pipeline
{
public:
    // Get the active shader stages through API info
    static VkShaderStageFlagBits GetActiveShaderStages(
        const VkGraphicsPipelineCreateInfo*             pGraphicsPipelineCreateInfo
        );

    // Returns true if Dual Source Blending is to be enabled based on the given ColorBlendAttachmentState
    static bool GetDualSourceBlendEnableState(
        const Device*                              pDevice,
        const VkPipelineColorBlendStateCreateInfo* pColorBlendState,
        const Pal::ColorBlendStateCreateInfo*      pPalInfo = nullptr);

    // Returns true if src alpha is used in blending
    static bool IsSrcAlphaUsedInBlend(VkBlendFactor blend);

    // Get the dynamics states specified by API info
    static uint32_t GetDynamicStateFlags(
        const VkPipelineDynamicStateCreateInfo*   pDy
        );

protected:
    // Convert API information into internal create info used to create internal pipeline binary
    static VkResult BuildPipelineBinaryCreateInfo(
        const Device*                       pDevice,
        const VkGraphicsPipelineCreateInfo* pCreateInfo,
        GraphicsPipelineBinaryCreateInfo*   pBinInfo,
        GraphicsPipelineShaderStageInfo*    pShaderInfo,
        VbBindingInfo*                      pVbInfo,
        ShaderModuleHandle*                 pTempModules);

    // Convert API information into internal create info used to create internal pipeline object
    static void BuildPipelineObjectCreateInfo(
        const Device*                       pDevice,
        const VkGraphicsPipelineCreateInfo* pIn,
        const VbBindingInfo*                pVbInfo,
        const GraphicsPipelineBinaryInfo*   pBinInfo,
        GraphicsPipelineObjectCreateInfo*   pObjInfo);

    // Generates the API PSO hash using the contents of the VkGraphicsPipelineCreateInfo struct
    static uint64_t BuildApiHash(
        const VkGraphicsPipelineCreateInfo*     pCreateInfo,
        const GraphicsPipelineObjectCreateInfo* pInfo);

    GraphicsPipelineCommon(
        Device* const pDevice)
        : Pipeline(
            pDevice,
            VK_PIPELINE_BIND_POINT_GRAPHICS)
    { }
};

}

#endif/*__GRAPHICS_PIPELINE_COMMON_H__*/