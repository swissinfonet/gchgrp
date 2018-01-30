#include <memory.h>


#include "crypto/cnx.h"
#include "Mem.h"
#include "Options.h"


bool Mem::m_doubleHash = false;
int Mem::m_algo        = 0;
int Mem::m_flags       = 0;
int Mem::m_threads     = 0;
size_t Mem::m_offset   = 0;
uint8_t *Mem::m_memory = nullptr;


cnx_ctx *Mem::create(int threadId)
{
#   ifndef gchgrp_NO_AEON
    if (m_algo == Options::ALGO_cnx_LITE) {
        return createLite(threadId);
    }
#   endif

    cnx_ctx *ctx = reinterpret_cast<cnx_ctx *>(&m_memory[MEMORY - sizeof(cnx_ctx) * (threadId + 1)]);

    const int ratio = m_doubleHash ? 2 : 1;
    ctx->memory = &m_memory[MEMORY * (threadId * ratio + 1)];

    return ctx;
}



void *Mem::calloc(size_t num, size_t size)
{
    void *mem = &m_memory[m_offset];
    m_offset += (num * size);

    memset(mem, 0, num * size);

    return mem;
}


#ifndef gchgrp_NO_AEON
cnx_ctx *Mem::createLite(int threadId) {
    cnx_ctx *ctx;

    if (!m_doubleHash) {
        const size_t offset = MEMORY * (threadId + 1);

        ctx = reinterpret_cast<cnx_ctx *>(&m_memory[offset + MEMORY_LITE]);
        ctx->memory = &m_memory[offset];
        return ctx;
    }

    ctx = reinterpret_cast<cnx_ctx *>(&m_memory[MEMORY - sizeof(cnx_ctx) * (threadId + 1)]);
    ctx->memory = &m_memory[MEMORY * (threadId + 1)];

    return ctx;
}
#endif
