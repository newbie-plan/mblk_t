#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "msqueue.h"


typedef struct
{
	MSQueue **inputs;
	MSQueue **outputs;
	void *data; 			/*私有数据*/
}Step;


typedef struct
{
	Step first;
	Step second;
	Step third;
}Global;


static int first_step_init(Step *first)
{
	printf("%s : %s : %d\n", __FILE__, __func__, __LINE__);
	if (first == NULL)
	{
		printf("%s failed.\n", __func__);
		exit(-1);
	}

	first->outputs = (MSQueue **)ms_new0(MSQueue *, 1);
	if (first->outputs == NULL)
	{
		printf("%s : ms_new0 for outputs failed.\n", __func__);
		exit(-1);
	}

	return 0;
}

static int first_step_process(Step *first)
{
	mblk_t *om = NULL;
	char *data = "hello";
	int len = strlen(data) + 1;

	printf("%s : %s : %d\n", __FILE__, __func__, __LINE__);
	if (first == NULL)
	{
		printf("%s failed.\n", __func__);
		exit(0);
	}

	/*put数据，实际可能会比这复杂*/
	om = allocb(len, 0);
	memcpy(om->b_wptr, data, len);
	om->b_wptr += len;
	ms_queue_put(first->outputs[0], om);

	return 0;
}

static int first_step_uninit(Step *first)
{
	if (first == NULL)
	{
		printf("%s failed.\n", __func__);
		exit(0);
	}
	
	printf("%s : %s : %d\n", __FILE__, __func__, __LINE__);
	if (first->outputs != NULL)
	{
		ms_free(first->outputs);
	}

	return 0;
}


static int second_step_init(Step *second)
{
	if (second == NULL)
	{
		printf("%s failed.\n", __func__);
		exit(-1);
	}

	printf("%s : %s : %d\n", __FILE__, __func__, __LINE__);
	second->inputs = (MSQueue **)ms_new0(MSQueue *, 1);
	second->outputs = (MSQueue **)ms_new0(MSQueue *, 1);
	if (second->inputs == NULL || second->outputs == NULL)
	{
		printf("%s : ms_new0 for inputs || outputs failed.\n", __func__);
		exit(-1);
	}

	return 0;
}

static int second_step_process(Step *second)
{
	mblk_t *im = NULL;
	mblk_t *om = NULL;
	char *data = "world";
	int len = strlen(data) + 1;

	printf("%s : %s : %d\n", __FILE__, __func__, __LINE__);
	if (second == NULL)
	{
		printf("%s failed.\n", __func__);
		exit(-1);
	}

	while ((im = ms_queue_get(second->inputs[0])) != NULL)
	{
		int size = im->b_wptr - im->b_rptr + len;

		printf("\t%s : get data is [%s]\n", __func__, im->b_rptr);

		/*get到数据处理之后再put出去*/
		om = allocb(size, 0);
		snprintf(om->b_wptr, size, "%s-%s", im->b_rptr, data);
		om->b_wptr += size;
		ms_queue_put(second->outputs[0], om);
		freemsg(im);
	}

	return 0;
}

static int second_step_uninit(Step *second)
{
	printf("%s : %s : %d\n", __FILE__, __func__, __LINE__);
	if (second == NULL)
	{
		printf("%s failed.\n", __func__);
		exit(-1);
	}

	if (second->inputs) ms_free(second->inputs);
	if (second->outputs) ms_free(second->outputs);

	return 0;
}


static int third_step_init(Step *third)
{
	printf("%s : %s : %d\n", __FILE__, __func__, __LINE__);
	if (third == NULL)
	{
		printf("%s failed.\n", __func__);
		exit(-1);
	}

	third->inputs = (MSQueue **)ms_new0(MSQueue *, 1);
	if (third->inputs == NULL)
	{
		printf("%s : ms_new0 for inputs failed.\n", __func__);
		exit(-1);
	}

	return 0;
}

static int third_step_process(Step *third)
{
	mblk_t *im = NULL;

	printf("%s : %s : %d\n", __FILE__, __func__, __LINE__);
	if (third == NULL)
	{
		printf("%s failed.\n", __func__);
		exit(0);
	}

	while ((im = ms_queue_get(third->inputs[0])) != NULL)
	{
		printf("\t%s : get data is [%s]\n", __func__, im->b_rptr);	
		freemsg(im);	
	}

	return 0;
}

static int third_step_uninit(Step *third)
{
	printf("%s : %s : %d\n", __FILE__, __func__, __LINE__);
	if (third == NULL)
	{
		printf("%s failed.\n", __func__);
		exit(0);
	}

	if (third->inputs) ms_free(third->inputs);

	return 0;
}


static int link(Step *f1, int pin1, Step *f2, int pin2)
{
	printf("%s : %s : %d\n", __FILE__, __func__, __LINE__);
	MSQueue *q = NULL;
	q = ms_queue_new();
	f1->outputs[pin1] = q;
	f2->inputs[pin2] = q;
	return 0;
}

static int init(Global *g)
{
	printf("%s : %s : %d\n", __FILE__, __func__, __LINE__);
	if (g == NULL)
	{
		printf("%s : global data is NULL.\n");
		exit(0);
	}

	first_step_init(&g->first);
	second_step_init(&g->second);
	third_step_init(&g->third);

	link(&g->first, 0, &g->second, 0);
	link(&g->second, 0, &g->third, 0);

	return 0;
}


static int process(Global *g)
{
	printf("%s : %s : %d\n", __FILE__, __func__, __LINE__);
	if (g == NULL)
	{
		printf("%s : global data is NULL.\n");
		exit(0);
	}

	/*一般这里可以创建线程循环处理，每次循环一圈，就有一部分数据处理完成*/
	first_step_process(&g->first);
	second_step_process(&g->second);
	third_step_process(&g->third);

	return 0;
}


static unlink(Step *f1, int pin1, Step *f2, int pin2)
{
	printf("%s : %s : %d\n", __FILE__, __func__, __LINE__);
	MSQueue *q = NULL;
	q = f1->outputs[pin1];
	f1->outputs[pin1] = f2->inputs[pin2] = NULL;
	ms_queue_destroy(q);
	return 0;
}

static destroy(Global *g)
{
	printf("%s : %s : %d\n", __FILE__, __func__, __LINE__);
	if (g == NULL)
	{
		printf("%s : global data is NULL.\n");
		exit(0);
	}

	unlink(&g->first, 0, &g->second, 0);
	unlink(&g->second, 0, &g->third, 0);

	first_step_uninit(&g->first);
	second_step_uninit(&g->second);
	third_step_uninit(&g->third);

	return 0;
}



int main(int argc, const char *argv[])
{
	Global g;
	memset(&g, 0, sizeof(Global));

	init(&g);
	process(&g);
	destroy(&g);
	
	return 0;
}
