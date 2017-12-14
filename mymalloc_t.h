#define SMALLSIZE 50     /*С�ڴ��������Ĵ�С */
#define SMALLNUM 20      /*С�ڴ��ĸ��� */
#define BIGSIZE 1000     /*���ڴ��������Ĵ�С */
#define BIGNUM 4         /*���ڴ��ĸ��� */

#define SMALLAREA 1      /*��С�ڴ���������� */
#define BIGAREA 2        /*�Ӵ��ڴ���������� */
#define HEAPAREA 3       /*��heap�ڴ���������� */

#define BLOCKAVAILABLE 0
#define BLOCKBUSY 1

#define SMALL_FREE 0	 /*С�ڴ����� */
#define SMALL_FULL 1     /*С�ڴ���� ��תΪ������ڴ�� */
#define BIG_FULL 2       /*���ڴ���� ��תΪ����heap�ڴ�� */

/*С�ڴ��ṹ*/
typedef struct{
	char data[SMALLSIZE];/*���data��*/
}small_block;

/*���ڴ��ṹ */ 
typedef struct{
	char data[BIGSIZE];/*���data��*/
}big_block;

/*С�ڴ�����ṹ*/ 
typedef struct{
	small_block * base_add;/*С�ڴ��Ļ���ַ*/
	int busy_num;/*��ʹ�õ����� */
	char busy[SMALLNUM];/*ÿ��С�ڴ���ʹ�����*/ 
}small_block_manage;

/*���ڴ�����ṹ*/ 
typedef struct{
	big_block * base_add;/*���ڴ��Ļ���ַ*/
	int busy_num;/*��ʹ�õ����� */
	char busy[BIGNUM]; /*ÿ�����ڴ���ʹ�����*/ 
}big_block_manage; 

typedef struct{
	int * heap_add;/*��¼��heap������ڴ��ĵ�ַ */
	int size;/*���׵�ַ���ڴ���ռ�ö����ֽ� */
	struct heap_add_node *next;/*ָ����һ���ڴ����� */
}heap_add_node; 
