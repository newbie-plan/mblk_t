/*通过模拟flac解码器filter来展示一下MSQueue和MSBuffer用法*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "msqueue.h"


typedef struct MSFilter
{
	void *priv_data;
}MSFilter;

typedef struct MSFlacEncInfo
{
	MSBufferizer encoder_before;
	MSQueue 	 encoder_after;
	pthread_t enc_thread;
	int state;
	FILE *fp;
}MSFlacEncInfo;

static const char *file = NULL;
static void *flac_enc_thread(void *arg);

static void flac_enc_init(MSFilter *f)
{
	MSFlacEncInfo *d = ms_new0(MSFlacEncInfo, 1);
	ms_bufferizer_init(&d->encoder_before);
	ms_queue_init(&d->encoder_after);
	d->state = 1;
	pthread_create(&d->enc_thread, NULL, flac_enc_thread, f);
	d->fp = fopen(file, "rb");
	f->priv_data = d;
}

static void flac_enc_precess(MSFilter *f)
{
	int len = 16000 * 10 / 1000 * 2; 	/*10ms数据*/
	MSFlacEncInfo *d = (MSFlacEncInfo *)f->priv_data;
	mblk_t *im = NULL;
	mblk_t *om = NULL;

	if (feof(d->fp) == 0)
	{
		int ret = 0;
		im = allocb(len, 0);
		if (im != NULL)
		{
			ret = fread(im->b_wptr, 1, len, d->fp); 		/*填充mblk_t,从b_wptr开始写*/
			im->b_wptr += ret; 								/*b_wptr向后移这么多字节*/
			ms_bufferizer_put(&d->encoder_before, im);
		}
	}

	while (om = ms_queue_get(&d->encoder_after)) 			/*从队列中取一个mblk_t*/
	{
		int size = om->b_wptr - om->b_rptr;
		printf("->> %s : size = [%d]\n", __func__, size);

		msgpullup(om, -1); 									/*将b_cont后面所有的数据合并到一个mblk_t中*/

		size = om->b_wptr - om->b_rptr;
		printf("->> %s : msgpullup after size = [%d]\n", __func__, size);

		printf("->> %s : data = [%s].\n", __func__, om->b_rptr); /*从b_rptr开始读*/
		freemsg(om);
	}
}


static void flac_enc_uninit(MSFilter *f)
{
	MSFlacEncInfo *d = (MSFlacEncInfo *)f->priv_data;

	d->state = 0;
	if (d->enc_thread)
	{
		pthread_join(d->enc_thread, NULL);
		printf("->> %s : flac encoder thread has join.\n", __func__);
	}
	if (d->fp != NULL)
	{
		fclose(d->fp);
	}
	ms_queue_flush(&d->encoder_after);
	ms_bufferizer_flush(&d->encoder_before);
	ms_free(d);
}



static void *flac_enc_thread(void *arg)
{
	int len = 1152 * 2;
	char buf[1152*2];
	int ret = -1;
	MSFilter *f = (MSFilter *)arg;
	MSFlacEncInfo *d = (MSFlacEncInfo *)f->priv_data;
	mblk_t *im = NULL;
	mblk_t *cont = NULL;

	while (d->state == 1)
	{
		while (ms_bufferizer_get_avail(&d->encoder_before) >= len) 			/*缓冲buffer中可用的总字节数*/
		{
			memset(buf, 0, sizeof(buf));
			ret = ms_bufferizer_read(&d->encoder_before, buf, len); 		/*从buffer中读len个字节*/
			printf("->> %s : read %d data.\n", __func__, ret);

			im = allocb(sizeof("after"), 0);
			memcpy(im->b_wptr, "after", sizeof("after"));
			im->b_wptr += sizeof("after");

			cont = allocb(sizeof("+encoder"), 0);
			memcpy(cont->b_wptr, "+encoder", sizeof("+encoder"));
			cont->b_wptr += sizeof("+encoder");
			cont->b_cont = NULL;
			
			im->b_cont = cont;
			ms_queue_put(&d->encoder_after, im);
		}
		usleep(12*1000);
	}

}




int main(int argc, const char *argv[])
{
	MSFilter f;
	FILE *fp = NULL;
	if (argc < 2)
	{
		printf("Usage: %s <inputfile>.\n", argv[0]);
		return -1;
	}
	file = argv[1];

	flac_enc_init(&f);

	fp = ((MSFlacEncInfo *)f.priv_data)->fp;
	while (feof(fp) == 0)
	{
		flac_enc_precess(&f);
		usleep(10*1000);
	}

	flac_enc_uninit(&f);
	return 0;
}
