#include <stdio.h>
#include <string.h>
#include "msqueue.h"


int main(int argc, const char *argv[])
{
    MSQueue test_queue;
    mblk_t *im = NULL;
    mblk_t *om = NULL;
    int len = 0;
    char buf[32] = {0};

    ms_queue_init(&test_queue);

    im = allocb(32, 0);
    memcpy(im->b_wptr, "hello", sizeof("hello"));
    im->b_wptr += sizeof("hello");
    ms_queue_put(&test_queue, im);


    om = ms_queue_get(&test_queue);
    len = msgdsize(om);
    printf("->> len = [%d]\n", len);
    memcpy(buf, om->b_rptr, len);
    printf("->> buf = [%s]\n", buf);
    freemsg(om);

    ms_queue_flush(&test_queue);

    return 0;
}
