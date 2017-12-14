#define SMALLSIZE 50     /*小内存块数据域的大小 */
#define SMALLNUM 20      /*小内存块的个数 */
#define BIGSIZE 1000     /*大内存块数据域的大小 */
#define BIGNUM 4         /*大内存块的个数 */

#define SMALLAREA 1      /*从小内存块区域申请 */
#define BIGAREA 2        /*从大内存块区域申请 */
#define HEAPAREA 3       /*从heap内存块区域申请 */

#define BLOCKAVAILABLE 0
#define BLOCKBUSY 1

#define SMALL_FREE 0	 /*小内存块可用 */
#define SMALL_FULL 1     /*小内存块满 ，转为申请大内存块 */
#define BIG_FULL 2       /*大内存块满 ，转为申请heap内存块 */

/*小内存块结构*/
typedef struct{
	char data[SMALLSIZE];/*存放data域*/
}small_block;

/*大内存块结构 */ 
typedef struct{
	char data[BIGSIZE];/*存放data域*/
}big_block;

/*小内存块管理结构*/ 
typedef struct{
	small_block * base_add;/*小内存块的基地址*/
	int busy_num;/*被使用的数量 */
	char busy[SMALLNUM];/*每个小内存块的使用情况*/ 
}small_block_manage;

/*大内存块管理结构*/ 
typedef struct{
	big_block * base_add;/*大内存块的基地址*/
	int busy_num;/*被使用的数量 */
	char busy[BIGNUM]; /*每个大内存块的使用情况*/ 
}big_block_manage; 

typedef struct{
	int * heap_add;/*记录从heap申请的内存块的地址 */
	int size;/*该首地址的内存区占用多少字节 */
	struct heap_add_node *next;/*指向下一个内存区域 */
}heap_add_node; 
