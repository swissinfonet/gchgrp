#ifndef __cnx_H__
#define __cnx_H__


#include <stddef.h>
#include <stdint.h>


#include "align.h"


#define MEMORY      2097152 /* 2 MiB */
#define MEMORY_LITE 1048576 /* 1 MiB */


struct cnx_ctx {
    VAR_ALIGN(16, uint8_t state0[200]);
    VAR_ALIGN(16, uint8_t state1[200]);
    VAR_ALIGN(16, uint8_t* memory);
};


class Job;
class JobResult;


class cnx
{
public:
    static bool hash(const Job &job, JobResult &result, cnx_ctx *ctx);
    static bool init(int algo, int variant);
    static void hash(const uint8_t *input, size_t size, uint8_t *output, cnx_ctx *ctx);

private:
    static bool selfTest(int algo);
};

#endif /* __cnx_H__ */
