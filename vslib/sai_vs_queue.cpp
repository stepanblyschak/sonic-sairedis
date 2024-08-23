#include "sai_vs.h"

VS_GENERIC_QUAD(QUEUE,queue);
VS_GENERIC_STATS(QUEUE,queue);
VS_BULK_QUAD(QUEUE,queue);

const sai_queue_api_t vs_queue_api = {

    VS_GENERIC_QUAD_API(queue)
    VS_GENERIC_STATS_API(queue)
    VS_BULK_QUAD_API(queue)
};
