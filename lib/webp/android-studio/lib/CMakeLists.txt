cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)

project(webp)

include("../../../../../hltypes/android-studio/generic/CMakeLists.txt")

# defining manually so source files don't need to be changed
file(
	GLOB CppSrc
    "../../src/cpu-features.c"
    "../../src/dec/alpha.c"
    "../../src/dec/buffer.c"
    "../../src/dec/frame.c"
    "../../src/dec/idec.c"
    "../../src/dec/io.c"
    "../../src/dec/quant.c"
    "../../src/dec/tree.c"
    "../../src/dec/vp8.c"
    "../../src/dec/vp8l.c"
    "../../src/dec/webp.c"
    "../../src/demux/anim_decode.c"
    "../../src/demux/demux.c"
    "../../src/dsp/alpha_processing.c"
    "../../src/dsp/alpha_processing_mips_dsp_r2.c"
    "../../src/dsp/alpha_processing_sse2.c"
    "../../src/dsp/alpha_processing_sse41.c"
    "../../src/dsp/argb.c"
    "../../src/dsp/argb_mips_dsp_r2.c"
    "../../src/dsp/argb_sse2.c"
    "../../src/dsp/cost.c"
    "../../src/dsp/cost_mips32.c"
    "../../src/dsp/cost_mips_dsp_r2.c"
    "../../src/dsp/cost_sse2.c"
    "../../src/dsp/cpu.c"
    "../../src/dsp/dec.c"
    "../../src/dsp/dec_clip_tables.c"
    "../../src/dsp/dec_mips32.c"
    "../../src/dsp/dec_mips_dsp_r2.c"
    "../../src/dsp/dec_msa.c"
    "../../src/dsp/dec_neon.c"
    "../../src/dsp/dec_sse2.c"
    "../../src/dsp/dec_sse41.c"
    "../../src/dsp/enc.c"
    "../../src/dsp/enc_avx2.c"
    "../../src/dsp/enc_mips32.c"
    "../../src/dsp/enc_mips_dsp_r2.c"
    "../../src/dsp/enc_neon.c"
    "../../src/dsp/enc_sse2.c"
    "../../src/dsp/enc_sse41.c"
    "../../src/dsp/filters.c"
    "../../src/dsp/filters_mips_dsp_r2.c"
    "../../src/dsp/filters_sse2.c"
    "../../src/dsp/lossless.c"
    "../../src/dsp/lossless_enc.c"
    "../../src/dsp/lossless_enc_mips32.c"
    "../../src/dsp/lossless_enc_mips_dsp_r2.c"
    "../../src/dsp/lossless_enc_neon.c"
    "../../src/dsp/lossless_enc_sse2.c"
    "../../src/dsp/lossless_enc_sse41.c"
    "../../src/dsp/lossless_mips_dsp_r2.c"
    "../../src/dsp/lossless_neon.c"
    "../../src/dsp/lossless_sse2.c"
    "../../src/dsp/rescaler.c"
    "../../src/dsp/rescaler_mips32.c"
    "../../src/dsp/rescaler_mips_dsp_r2.c"
    "../../src/dsp/rescaler_neon.c"
    "../../src/dsp/rescaler_sse2.c"
    "../../src/dsp/upsampling.c"
    "../../src/dsp/upsampling_mips_dsp_r2.c"
    "../../src/dsp/upsampling_neon.c"
    "../../src/dsp/upsampling_sse2.c"
    "../../src/dsp/yuv.c"
    "../../src/dsp/yuv_mips32.c"
    "../../src/dsp/yuv_mips_dsp_r2.c"
    "../../src/dsp/yuv_sse2.c"
    "../../src/enc/alpha.c"
    "../../src/enc/analysis.c"
    "../../src/enc/backward_references.c"
    "../../src/enc/config.c"
    "../../src/enc/cost.c"
    "../../src/enc/filter.c"
    "../../src/enc/frame.c"
    "../../src/enc/histogram.c"
    "../../src/enc/iterator.c"
    "../../src/enc/near_lossless.c"
    "../../src/enc/picture.c"
    "../../src/enc/picture_csp.c"
    "../../src/enc/picture_psnr.c"
    "../../src/enc/picture_rescale.c"
    "../../src/enc/picture_tools.c"
    "../../src/enc/quant.c"
    "../../src/enc/syntax.c"
    "../../src/enc/token.c"
    "../../src/enc/tree.c"
    "../../src/enc/vp8l.c"
    "../../src/enc/webpenc.c"
    "../../src/utils/bit_reader.c"
    "../../src/utils/bit_writer.c"
    "../../src/utils/color_cache.c"
    "../../src/utils/filters.c"
    "../../src/utils/huffman.c"
    "../../src/utils/huffman_encode.c"
    "../../src/utils/quant_levels.c"
    "../../src/utils/quant_levels_dec.c"
    "../../src/utils/random.c"
    "../../src/utils/rescaler.c"
    "../../src/utils/thread.c"
    "../../src/utils/utils.c"
)

add_library(webp STATIC ${CppSrc})

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
	string(REPLACE " -mfpu=vfpv3-d16" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
	string(REPLACE " -mfpu=vfpv3-d16" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	string(REPLACE " -mfpu=vfpv3-d16" "" CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS}")
	add_definitions(
		-DWEBP_USE_THREAD
		-mfloat-abi=softfp
		-mfpu=neon
	)
endif()

target_link_libraries(
	webp
)
