/*
 * Copyright © 2015 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "anv_private.h"

#include "genxml/gen_macros.h"
#include "genxml/genX_pack.h"

VkResult
genX(init_device_state)(struct anv_device *device)
{
   GENX(MEMORY_OBJECT_CONTROL_STATE_pack)(NULL, &device->default_mocs,
                                          &GENX(MOCS));

   struct anv_batch batch;

   uint32_t cmds[64];
   batch.start = batch.next = cmds;
   batch.end = (void *) cmds + sizeof(cmds);

   anv_batch_emit(&batch, GENX(PIPELINE_SELECT),
#if GEN_GEN >= 9
                  .MaskBits = 3,
#endif
                  .PipelineSelection = _3D);

   anv_batch_emit(&batch, GENX(3DSTATE_VF_STATISTICS),
                  .StatisticsEnable = true);
   anv_batch_emit(&batch, GENX(3DSTATE_HS));
   anv_batch_emit(&batch, GENX(3DSTATE_TE));
   anv_batch_emit(&batch, GENX(3DSTATE_DS));

   anv_batch_emit(&batch, GENX(3DSTATE_STREAMOUT), .SOFunctionEnable = false);
   anv_batch_emit(&batch, GENX(3DSTATE_AA_LINE_PARAMETERS));

#if GEN_GEN >= 8
   anv_batch_emit(&batch, GENX(3DSTATE_WM_CHROMAKEY),
                  .ChromaKeyKillEnable = false);

   /* See the Vulkan 1.0 spec Table 24.1 "Standard sample locations" and
    * VkPhysicalDeviceFeatures::standardSampleLocations.
    */
   anv_batch_emit(&batch, GENX(3DSTATE_SAMPLE_PATTERN),
      ._1xSample0XOffset      = 0.5,
      ._1xSample0YOffset      = 0.5,
      ._2xSample0XOffset      = 0.25,
      ._2xSample0YOffset      = 0.25,
      ._2xSample1XOffset      = 0.75,
      ._2xSample1YOffset      = 0.75,
      ._4xSample0XOffset      = 0.375,
      ._4xSample0YOffset      = 0.125,
      ._4xSample1XOffset      = 0.875,
      ._4xSample1YOffset      = 0.375,
      ._4xSample2XOffset      = 0.125,
      ._4xSample2YOffset      = 0.625,
      ._4xSample3XOffset      = 0.625,
      ._4xSample3YOffset      = 0.875,
      ._8xSample0XOffset      = 0.5625,
      ._8xSample0YOffset      = 0.3125,
      ._8xSample1XOffset      = 0.4375,
      ._8xSample1YOffset      = 0.6875,
      ._8xSample2XOffset      = 0.8125,
      ._8xSample2YOffset      = 0.5625,
      ._8xSample3XOffset      = 0.3125,
      ._8xSample3YOffset      = 0.1875,
      ._8xSample4XOffset      = 0.1875,
      ._8xSample4YOffset      = 0.8125,
      ._8xSample5XOffset      = 0.0625,
      ._8xSample5YOffset      = 0.4375,
      ._8xSample6XOffset      = 0.6875,
      ._8xSample6YOffset      = 0.9375,
      ._8xSample7XOffset      = 0.9375,
      ._8xSample7YOffset      = 0.0625,
#if GEN_GEN >= 9
      ._16xSample0XOffset     = 0.5625,
      ._16xSample0YOffset     = 0.5625,
      ._16xSample1XOffset     = 0.4375,
      ._16xSample1YOffset     = 0.3125,
      ._16xSample2XOffset     = 0.3125,
      ._16xSample2YOffset     = 0.6250,
      ._16xSample3XOffset     = 0.7500,
      ._16xSample3YOffset     = 0.4375,
      ._16xSample4XOffset     = 0.1875,
      ._16xSample4YOffset     = 0.3750,
      ._16xSample5XOffset     = 0.6250,
      ._16xSample5YOffset     = 0.8125,
      ._16xSample6XOffset     = 0.8125,
      ._16xSample6YOffset     = 0.6875,
      ._16xSample7XOffset     = 0.6875,
      ._16xSample7YOffset     = 0.1875,
      ._16xSample8XOffset     = 0.3750,
      ._16xSample8YOffset     = 0.8750,
      ._16xSample9XOffset     = 0.5000,
      ._16xSample9YOffset     = 0.0625,
      ._16xSample10XOffset    = 0.2500,
      ._16xSample10YOffset    = 0.1250,
      ._16xSample11XOffset    = 0.1250,
      ._16xSample11YOffset    = 0.7500,
      ._16xSample12XOffset    = 0.0000,
      ._16xSample12YOffset    = 0.5000,
      ._16xSample13XOffset    = 0.9375,
      ._16xSample13YOffset    = 0.2500,
      ._16xSample14XOffset    = 0.8750,
      ._16xSample14YOffset    = 0.9375,
      ._16xSample15XOffset    = 0.0625,
      ._16xSample15YOffset    = 0.0000,
#endif
   );
#endif

   anv_batch_emit(&batch, GENX(MI_BATCH_BUFFER_END));

   assert(batch.next <= batch.end);

   return anv_device_submit_simple_batch(device, &batch);
}

static inline uint32_t
vk_to_gen_tex_filter(VkFilter filter, bool anisotropyEnable)
{
   switch (filter) {
   default:
      assert(!"Invalid filter");
   case VK_FILTER_NEAREST:
      return MAPFILTER_NEAREST;
   case VK_FILTER_LINEAR:
      return anisotropyEnable ? MAPFILTER_ANISOTROPIC : MAPFILTER_LINEAR;
   }
}

static inline uint32_t
vk_to_gen_max_anisotropy(float ratio)
{
   return (anv_clamp_f(ratio, 2, 16) - 2) / 2;
}

static const uint32_t vk_to_gen_mipmap_mode[] = {
   [VK_SAMPLER_MIPMAP_MODE_NEAREST]          = MIPFILTER_NEAREST,
   [VK_SAMPLER_MIPMAP_MODE_LINEAR]           = MIPFILTER_LINEAR
};

static const uint32_t vk_to_gen_tex_address[] = {
   [VK_SAMPLER_ADDRESS_MODE_REPEAT]          = TCM_WRAP,
   [VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT] = TCM_MIRROR,
   [VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE]   = TCM_CLAMP,
   [VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE] = TCM_MIRROR_ONCE,
   [VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER] = TCM_CLAMP_BORDER,
};

static const uint32_t vk_to_gen_compare_op[] = {
   [VK_COMPARE_OP_NEVER]                     = PREFILTEROPNEVER,
   [VK_COMPARE_OP_LESS]                      = PREFILTEROPLESS,
   [VK_COMPARE_OP_EQUAL]                     = PREFILTEROPEQUAL,
   [VK_COMPARE_OP_LESS_OR_EQUAL]             = PREFILTEROPLEQUAL,
   [VK_COMPARE_OP_GREATER]                   = PREFILTEROPGREATER,
   [VK_COMPARE_OP_NOT_EQUAL]                 = PREFILTEROPNOTEQUAL,
   [VK_COMPARE_OP_GREATER_OR_EQUAL]          = PREFILTEROPGEQUAL,
   [VK_COMPARE_OP_ALWAYS]                    = PREFILTEROPALWAYS,
};

VkResult genX(CreateSampler)(
    VkDevice                                    _device,
    const VkSamplerCreateInfo*                  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSampler*                                  pSampler)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   struct anv_sampler *sampler;

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);

   sampler = anv_alloc2(&device->alloc, pAllocator, sizeof(*sampler), 8,
                        VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!sampler)
      return vk_error(VK_ERROR_OUT_OF_HOST_MEMORY);

   uint32_t border_color_offset = device->border_colors.offset +
                                  pCreateInfo->borderColor * 64;

   struct GENX(SAMPLER_STATE) sampler_state = {
      .SamplerDisable = false,
      .TextureBorderColorMode = DX10OGL,

#if GEN_GEN >= 8
      .LODPreClampMode = CLAMP_MODE_OGL,
#else
      .LODPreClampEnable = CLAMP_ENABLE_OGL,
#endif

#if GEN_GEN == 8
      .BaseMipLevel = 0.0,
#endif
      .MipModeFilter = vk_to_gen_mipmap_mode[pCreateInfo->mipmapMode],
      .MagModeFilter = vk_to_gen_tex_filter(pCreateInfo->magFilter,
                                            pCreateInfo->anisotropyEnable),
      .MinModeFilter = vk_to_gen_tex_filter(pCreateInfo->minFilter,
                                            pCreateInfo->anisotropyEnable),
      .TextureLODBias = anv_clamp_f(pCreateInfo->mipLodBias, -16, 15.996),
      .AnisotropicAlgorithm = EWAApproximation,
      .MinLOD = anv_clamp_f(pCreateInfo->minLod, 0, 14),
      .MaxLOD = anv_clamp_f(pCreateInfo->maxLod, 0, 14),
      .ChromaKeyEnable = 0,
      .ChromaKeyIndex = 0,
      .ChromaKeyMode = 0,
      .ShadowFunction = vk_to_gen_compare_op[pCreateInfo->compareOp],
      .CubeSurfaceControlMode = OVERRIDE,

#if GEN_GEN >= 8
      .IndirectStatePointer = border_color_offset >> 6,
#else
      .BorderColorPointer = border_color_offset >> 5,
#endif

#if GEN_GEN >= 8
      .LODClampMagnificationMode = MIPNONE,
#endif

      .MaximumAnisotropy = vk_to_gen_max_anisotropy(pCreateInfo->maxAnisotropy),
      .RAddressMinFilterRoundingEnable = 0,
      .RAddressMagFilterRoundingEnable = 0,
      .VAddressMinFilterRoundingEnable = 0,
      .VAddressMagFilterRoundingEnable = 0,
      .UAddressMinFilterRoundingEnable = 0,
      .UAddressMagFilterRoundingEnable = 0,
      .TrilinearFilterQuality = 0,
      .NonnormalizedCoordinateEnable = pCreateInfo->unnormalizedCoordinates,
      .TCXAddressControlMode = vk_to_gen_tex_address[pCreateInfo->addressModeU],
      .TCYAddressControlMode = vk_to_gen_tex_address[pCreateInfo->addressModeV],
      .TCZAddressControlMode = vk_to_gen_tex_address[pCreateInfo->addressModeW],
   };

   GENX(SAMPLER_STATE_pack)(NULL, sampler->state, &sampler_state);

   *pSampler = anv_sampler_to_handle(sampler);

   return VK_SUCCESS;
}