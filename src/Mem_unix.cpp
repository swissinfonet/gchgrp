#include <stdlib.h>
#include <sys/mman.h>


#if defined(gchgrp_ARM) && !defined(__clang__)
#   include "aligned_malloc.h"
#else
#   include <mm_malloc.h>
#endif


#include "crypto/CryptoNight.h"
#include "log/Log.h"
#include "Mem.h"
#include "Options.h"


bool Mem::allocate(int algo, int threads, bool doubleHash, bool enabled)
{
    m_algo       = algo;
    m_threads    = threads;
    m_doubleHash = doubleHash;

    const int ratio   = (doubleHash && algo != Options::ALGO_CRYPTONIGHT_LITE) ? 2 : 1;
    const size_t size = MEMORY * (threads * ratio + 1);

    if (!enabled) {
        m_memory = static_cast<uint8_t*>(_mm_malloc(size, 16));
        return true;
    }

    m_flags |= HugepagesAvailable;

#   if defined(__APPLE__)
    m_memory = static_cast<uint8_t*>(mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, VM_FLAGS_SUPERPAGE_SIZE_2MB, 0));
#   elif defined(__FreeBSD__)
    m_memory = static_cast<uint8_t*>(mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_ALIGNED_SUPER | MAP_PREFAULT_READ, -1, 0));
#   else
    m_memory = static_cast<uint8_t*>(mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB | MAP_POPULATE, 0, 0));
#   endif
    if (m_memory == MAP_FAILED) {
        m_memory = static_cast<uint8_t*>(_mm_malloc(size, 16));
        return true;
    }

    m_flags |= HugepagesEnabled;

    if (madvise(m_memory, size, MADV_RANDOM | MADV_WILLNEED) != 0) {
        LOG_ERR("madvise failed");
    }

    if (mlock(m_memory, size) == 0) {
        m_flags |= Lock;
    }

    return true;
}


void Mem::release()
{
    const int size = MEMORY * (m_threads + 1);

    if (m_flags & HugepagesEnabled) {
        if (m_flags & Lock) {
            munlock(m_memory, size);
        }

        munmap(m_memory, size);
    }
    else {
        _mm_free(m_memory);
    }
}
