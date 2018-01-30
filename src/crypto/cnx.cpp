#include "crypto/cnx.h"

#if defined(gchgrp_ARM)
#   include "crypto/cnx_arm.h"
#else
#   include "crypto/cnx_x86.h"
#endif

#include "crypto/cnx_test.h"
#include "net/Job.h"
#include "net/JobResult.h"
#include "Options.h"


void (*cnx_hash_ctx)(const void *input, size_t size, void *output, cnx_ctx *ctx) = nullptr;


static void cnx_av1_aesni(const void *input, size_t size, void *output, struct cnx_ctx *ctx) {
#   if !defined(gchgrp_ARMv7)
    cnx_hash<0x80000, MEMORY, 0x1FFFF0, false>(input, size, output, ctx);
#   endif
}


static void cnx_av2_aesni_double(const void *input, size_t size, void *output, cnx_ctx *ctx) {
#   if !defined(gchgrp_ARMv7)
    cnx_double_hash<0x80000, MEMORY, 0x1FFFF0, false>(input, size, output, ctx);
#   endif
}


static void cnx_av3_softaes(const void *input, size_t size, void *output, cnx_ctx *ctx) {
    cnx_hash<0x80000, MEMORY, 0x1FFFF0, true>(input, size, output, ctx);
}


static void cnx_av4_softaes_double(const void *input, size_t size, void *output, cnx_ctx *ctx) {
    cnx_double_hash<0x80000, MEMORY, 0x1FFFF0, true>(input, size, output, ctx);
}


#ifndef gchgrp_NO_AEON
static void cnx_lite_av1_aesni(const void *input, size_t size, void *output, cnx_ctx *ctx) {
    #   if !defined(gchgrp_ARMv7)
    cnx_hash<0x40000, MEMORY_LITE, 0xFFFF0, false>(input, size, output, ctx);
#endif
}


static void cnx_lite_av2_aesni_double(const void *input, size_t size, void *output, cnx_ctx *ctx) {
#   if !defined(gchgrp_ARMv7)
    cnx_double_hash<0x40000, MEMORY_LITE, 0xFFFF0, false>(input, size, output, ctx);
#   endif
}


static void cnx_lite_av3_softaes(const void *input, size_t size, void *output, cnx_ctx *ctx) {
    cnx_hash<0x40000, MEMORY_LITE, 0xFFFF0, true>(input, size, output, ctx);
}


static void cnx_lite_av4_softaes_double(const void *input, size_t size, void *output, cnx_ctx *ctx) {
    cnx_double_hash<0x40000, MEMORY_LITE, 0xFFFF0, true>(input, size, output, ctx);
}

void (*cnx_variations[8])(const void *input, size_t size, void *output, cnx_ctx *ctx) = {
            cnx_av1_aesni,
            cnx_av2_aesni_double,
            cnx_av3_softaes,
            cnx_av4_softaes_double,
            cnx_lite_av1_aesni,
            cnx_lite_av2_aesni_double,
            cnx_lite_av3_softaes,
            cnx_lite_av4_softaes_double
        };
#else
void (*cnx_variations[4])(const void *input, size_t size, void *output, cnx_ctx *ctx) = {
            cnx_av1_aesni,
            cnx_av2_aesni_double,
            cnx_av3_softaes,
            cnx_av4_softaes_double
        };
#endif


bool cnx::hash(const Job &job, JobResult &result, cnx_ctx *ctx)
{
    cnx_hash_ctx(job.blob(), job.size(), result.result, ctx);

    return *reinterpret_cast<uint64_t*>(result.result + 24) < job.target();
}


bool cnx::init(int algo, int variant)
{
    if (variant < 1 || variant > 4) {
        return false;
    }

#   ifndef gchgrp_NO_AEON
    const int index = algo == Options::ALGO_cnx_LITE ? (variant + 3) : (variant - 1);
#   else
    const int index = variant - 1;
#   endif

    cnx_hash_ctx = cnx_variations[index];

    return selfTest(algo);
}


void cnx::hash(const uint8_t *input, size_t size, uint8_t *output, cnx_ctx *ctx)
{
    cnx_hash_ctx(input, size, output, ctx);
}


bool cnx::selfTest(int algo) {
    if (cnx_hash_ctx == nullptr) {
        return false;
    }

    char output[64];

    struct cnx_ctx *ctx = (struct cnx_ctx*) _mm_malloc(sizeof(struct cnx_ctx), 16);
    ctx->memory = (uint8_t *) _mm_malloc(MEMORY * 2, 16);

    cnx_hash_ctx(test_input, 76, output, ctx);

    _mm_free(ctx->memory);
    _mm_free(ctx);

#   ifndef gchgrp_NO_AEON
    return memcmp(output, algo == Options::ALGO_cnx_LITE ? test_output1 : test_output0, (Options::i()->doubleHash() ? 64 : 32)) == 0;
#   else
    return memcmp(output, test_output0, (Options::i()->doubleHash() ? 64 : 32)) == 0;
#   endif
}
