# mblk_t

typedef struct msgb
{
	struct msgb *b_prev; 			/*指向上一个节点*/
	struct msgb *b_next; 			/*指向下一个节点*/
	struct msgb *b_cont; 			/*复合数据:如果这个节点有多个mblk_t,那么后续的节点跟在这个指针后*/
	struct datab *b_datap; 			/*数据块*/
	unsigned char *b_rptr; 			/*从这里开始读数据*/
	unsigned char *b_wptr; 			/*从这里开始写数据*/
	uint32_t reserved1;
	uint32_t reserved2;
} mblk_t;

 # MSQueue是队列. 
  # ms_queue_put -> 向队列中插入一个mblk_t
  # ms_queue_get -> 从队列中读出一个mblk_t,需要使用者自己释放此mblk_t
 # MSBufferizer是一个缓冲bufferr
  # ms_bufferizer_put ->插入一个mblk_t
  # ms_bufferizer_read -> 读出指定字节数的数据,如果这个节点的数据不够,会自动继续从下个节点读,并把消耗完数据的节点释放
