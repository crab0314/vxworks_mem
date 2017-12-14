#include<stdio.h>
#include<stdlib.h>
#include<semaphore.h>
#include"mymalloc_t.h"

/*全局变量*/ 
int hasInited = 0;/*记录内存区域是否已经被初始化 */
small_block_manage * small_manage;/*指向小内存块管理区域的指针 */
big_block_manage * big_manage; /*指向大内存块管理区域的指针 */ 

SEM_ID sem_small ;
SEM_ID sem_big;
SEM_ID sem_heap;

heap_add_node * p_head = NULL; 
heap_add_node * p_tail = NULL;



/*计算每一块内存块的首地址，head_add为申请的一大块内存的首地址*/ 
void calculate_address(int* head_add){
	
	int block_index = 0; 
	small_block * small_temp;
	big_block * big_temp;
	
	small_manage = head_add;
	big_manage = small_manage + 1;
	small_temp = big_manage + 1;/*第一个小内存块的首地址，由一个小内存块指针指向 */
	big_temp = small_temp + SMALLNUM;/*第一个大内存块的首地址，由一个小内存块指针指向 */	
	
	
	/*初始化小内存块管理结构*/
	(*small_manage).base_add = small_temp;
	(*small_manage).busy_num = 0;
	
	/*初始化大内存块管理结构*/
	(*big_manage).base_add = big_temp;
	(*big_manage).busy_num = 0;	
	

	for(block_index = 0;block_index < SMALLNUM;block_index++){
		(*small_manage).busy[block_index] = BLOCKAVAILABLE;/*初始化，将每一个小内存块置为可用 */
		/*printf("小内存块地址：%d\n",(*small_manage).base_add+block_index);*/
	}
	
	for(block_index = 0;block_index < BIGNUM;block_index++){
		(*big_manage).busy[block_index] = BLOCKAVAILABLE;/*初始化，将每一个大内存块置为可用 */
		/*printf("大内存块地址：%d\n",(*big_manage).base_add+block_index);*/
	}
	
	printf("==========================================================\n");
	printf("||\t小内存管理块的首地址：%08dD,%08xH\t||\n",small_manage,small_manage);
	printf("||\t大内存管理块的首地址：%08dD,%08xH\t||\n",big_manage,big_manage);
	printf("||\t小内存块数组的首地址：%08dD,%08xH\t||\n",(*small_manage).base_add,(*small_manage).base_add);
	printf("||\t大内存块数组的首地址：%08dD,%08xH\t||\n",(*big_manage).base_add,(*big_manage).base_add);
	printf("==========================================================\n");
	/* printf("第二个大内存块数组的首地址：%08xH,%08dD\n",big_temp+1,big_temp + 1);*/
}

/*内存初始化模块，计算要申请的内存区域大小，返回内存首地址*/
int* mem_init(){
	int small_size;
	int big_size;
	int manage_size;
	int pre_alloc_size;
	
	int * array;

	sem_small = semBCreate(SEM_Q_FIFO, SEM_FULL);
	sem_big = semBCreate(SEM_Q_FIFO, SEM_FULL);
	sem_heap = semBCreate(SEM_Q_FIFO, SEM_FULL); 
	
	
	if(hasInited > 0){
		printf("已经完成内存管理初始化。\n");
		return; 
	} 
	
	
	/*计算首先要申请多少字节的内存地址*/ 
	small_size = SMALLNUM*sizeof(small_block);
	big_size = BIGNUM*sizeof(big_block);
	manage_size = sizeof(small_block_manage) + sizeof(big_block_manage);
	pre_alloc_size = small_size + big_size + manage_size;
	
	printf(" ========================================================\n"); 
	printf("||\t小内存块数量：%d，每块占用%d字节，共%d字节\t||\n",SMALLNUM,sizeof(small_block),small_size); 
	printf("||\t大内存块数量：%d，每块占用%d字节，共%d字节\t||\n",BIGNUM,sizeof(big_block),big_size); 
	printf("||\t小内存管理块占用%d字节\t\t\t\t||\n",sizeof(small_block_manage));
	printf("||\t大内存管理块占用%d字节\t\t\t\t||\n",sizeof(big_block_manage));
	printf("||\t初始化时总共需申请%d字节的内存区域\t\t||\n",pre_alloc_size);
	printf(" =========================================================\n");
	
	p_head = (struct heap_add_node *)malloc(sizeof(heap_add_node));  
	p_head -> next = NULL;
	p_tail = p_head;

	/*array是申请pre_alloc_size大小的内存块返回的首地址*/
	
	array = (int *)malloc(pre_alloc_size);
	if(array == NULL){
		printf("不能成功分配存储空间。\n");
		return NULL;
	}else{
		hasInited = 1;
		calculate_address(array); 
	
		printf(" ========================================================\n");
		printf("||\t内存管理初始化完成，起始地址：%08dD\t\t||\n",array);
		printf("||\t内存管理初始化完成，起始地址：%08xH\t\t||\n",array);
		printf(" ========================================================\n");
		return array;
	}
} 

/*按类型申请内存，返回申请到的内存块的首地址*/
int* malloc_by_type(int type,int size){


	int index;
	int* heap_add;
	heap_add_node * p_new;	
	small_block * small_first;
	big_block * big_first;
	
	int flag_small_given;
	int flag_big_given;
	int flag_heap_given;

	flag_small_given = 0;
	flag_big_given = 0;
	flag_heap_given = 0;
	

	/*int* add = NULL;*/
	index = 0;
	heap_add = NULL;
	p_new = NULL;
	small_first=NULL;
	big_first=NULL;
	/*heap_add_node * tail = p_tail;*/
	
	
	switch(type){
		case SMALLAREA :
			semTake(sem_small,WAIT_FOREVER);
			(*small_manage).busy_num++;
			small_first = (*small_manage).base_add;
			for(index = 0;index < SMALLNUM; index++){
				if((*small_manage).busy[index]==BLOCKAVAILABLE){
					(*small_manage).busy[index]=BLOCKBUSY;
					semGive(sem_small);
					flag_small_given = 1;	
					return small_first + index;
				}
			} 
			if(flag_small_given == 0)
				semGive(sem_small);
			break;
		case BIGAREA:
			semTake(sem_big,WAIT_FOREVER);
			(*big_manage).busy_num++;
			big_first = (*big_manage).base_add;
			for(index = 0;index < BIGNUM; index++){
				if((*big_manage).busy[index] == BLOCKAVAILABLE){
					(*big_manage).busy[index] = BLOCKBUSY;
					semGive(sem_big);
					flag_big_given = 1;	
					return big_first + index;
				}
			} 
			if(flag_big_given == 0)
				semGive(sem_big);
			break;
		case HEAPAREA:
			semTake(sem_heap,WAIT_FOREVER);
			p_new = (heap_add_node *)malloc(sizeof(heap_add_node));  
        		if(NULL == p_new){  
            			printf("从heap分配内存失败,程序终止!\n");  
				semGive(sem_heap);
            			return NULL;  
        		}  
        		heap_add = (int*)malloc(sizeof(size));
        		p_new -> heap_add = heap_add; 
			p_new -> size = size; 
			p_new -> next = NULL; 
			
        		p_tail -> next = p_new;  
        		p_tail = p_new; 
			semGive(sem_heap);
			return heap_add; 
					
			break;
		default:
			break; 		
	}
} 

/*
	申请内存块程序
	1.小于SMALLSIZE的部分申请小内存块，如果小内存块区域满，则申请大内存块
	2.大于SMALLSIZE小于BIGSIZE申请大内存块，如果大内存块区域满，则从heap申请内存块
	3.从堆申请内存块，并连接到heap内存块链表上 
*/ 
int* mem_malloc(int size){
	
	int small_busy;
	int big_busy;
	int flag;
	
	small_busy = 0;
	big_busy = 0;
	flag = SMALL_FREE;
	
	if(size > 0 && size <= SMALLSIZE){
		printf("正在申请小内存块...\n");
		small_busy = (*small_manage).busy_num;
		if(small_busy == SMALLNUM){
			printf("小内存块满...\n");
			flag = SMALL_FULL;
		}else{
			return malloc_by_type(SMALLAREA,size);
		}	
	}
	if((size > SMALLSIZE && size <= BIGSIZE)||flag == SMALL_FULL){
		printf("正在申请大内存块...\n");
		big_busy = (*small_manage).busy_num;
		if(big_busy == BIGNUM){
			printf("大内存块满...\n");
			flag = BIG_FULL;
		}else{
			return malloc_by_type(BIGAREA,size); 
		} 
	}
	if(size > BIGSIZE||flag == BIG_FULL){
		printf("正在从heap申请内存块...\n");
		return malloc_by_type(HEAPAREA,size);		
	}
}

/*
	首先检验地址合法性，不合法return;
	合法判断是三条链中哪一条链上的内存块，执行free操作 
*/ 
void mem_free(int free_add){
	/*printf("正在检验地址合法性...\n");*/


	if(free_add >= (*small_manage).base_add && free_add <= (*small_manage).base_add + SMALLNUM-1){
		int index = 0;

		small_block* small_first = (*small_manage).base_add;

		for(index = 0;index<SMALLNUM;index++){
			if(small_first+index==free_add){
				(*small_manage).busy_num--;
				(*small_manage).busy[index]=BLOCKAVAILABLE;
				printf("已成功释放一块小内存\n"); 
			}
		} 
	}else if(free_add >= (*big_manage).base_add && free_add <= (*big_manage).base_add + BIGNUM-1){
		int index = 0;
		big_block* big_first = (*big_manage).base_add;
		for(index = 0;index<BIGNUM;index++){
			if(big_first+index==free_add){
				(*big_manage).busy_num--;
				(*big_manage).busy[index]=BLOCKAVAILABLE;
				printf("已成功释放一块大内存\n"); 
			}
		} 
	} 

	/*
		先判断是否是从heap申请的，
		如果是，则释放
		否则提示输入地址无效
	*/
	else if(p_head ->next != NULL){
		heap_add_node * p = p_head;  
		heap_add_node * p_temp = NULL; 
		while(p->next!=NULL)  
        {  
        	p_temp = p->next; 
            if(p_temp->heap_add == free_add){
            	free(free_add);
            	p->next = p_temp->next;
            	free(p_temp);
            	printf("已成功释放一块heap内存\n"); 
            	return;
            } else{
            	p = p->next;  
            } 
        } 
		printf("该首地址无效。\n"); 
	}else{
		printf("该首地址无效。\n"); 
	}
}


/*遍历记录从heap申请的内存块的链表*/
void traverse_list()  
{  
	
   	heap_add_node * p = p_head->next;  

    	if(p_head ->next == NULL){  
        	printf("链表为空！\n");  
   	}else{  
       		printf("从heap申请到的内存信息如下：\n");  
       		while(p!=NULL)  
        	{  
            		printf("首地址：%08dD，首地址：%08xH，占用：%d字节\n",p->heap_add,p->heap_add,p->size);  
            		p = p->next;  
        	}  
   	 }  
}  

/*释放记录从heap申请的内存块的链表*/
void free_list(){
	heap_add_node * p = p_head; 
	heap_add_node *p_temp = NULL; 
	while(p->next != NULL){
		p_temp = p->next;
        free(p_temp->heap_add);
        p->next = p_temp->next;
        free(p_temp);
        printf("已成功释放一块heap内存\n"); 
	} 
}

/*展示内存的占用情况*/ 
void mem_show(){
	double f1;
	double f2;
	int index;
	
	if(hasInited == 0){
		printf("内存暂未初始化。\n");
		return; 
	}else if(hasInited > 0){
		printf("\n============================内存使用情况如下============================\n");
		f1 = (double)(*small_manage).busy_num/(double)SMALLNUM*100;;
		printf("\n小内存块共%d个，已占用%d个，占用率%.2f%%\n",SMALLNUM,(*small_manage).busy_num,f1); 
		
		for(index=0; index<SMALLNUM; index++){
			if((*small_manage).busy[index]==BLOCKAVAILABLE) 
				printf("%d块:可用\t",index);
			if((*small_manage).busy[index]==BLOCKBUSY)
				printf("%d块:占用\t",index);
			if((index+1)%4==0)printf("\t||\n"); 
		} 
		printf("\n");
		f2 = (double)(*big_manage).busy_num/(double)BIGNUM*100;;
		printf("大内存块共%d个，已占用%d个，占用率%.2f%%\n",BIGNUM,(*big_manage).busy_num,f2); 
		for(index=0; index<BIGNUM; index++){
			if((*big_manage).busy[index]==BLOCKAVAILABLE) 
				printf("%d块:可用\t",index);
			if((*big_manage).busy[index]==BLOCKBUSY)
				printf("%d块:占用\t",index);
			if((index+1)%4==0)printf("\t||\n"); 
		} 
		printf("\n");
		
		traverse_list();
		printf("========================================================================\n");
	}else{
		printf("内存初始化出现问题！\n");
	}	
}

/*打印用户选择菜单*/ 
void print_menu(){
	printf("*********************************************************\n");
	printf("*\t\t\t0.退出程序。\t\t\t*\n");
	printf("*\t\t\t1.申请内存。\t\t\t*\n");
	printf("*\t\t\t2.释放内存。\t\t\t*\n");
	printf("*\t\t\t3.展示占用。\t\t\t*\n");
	printf("*********************************************************\n");
	printf("请输入0-3的整数：");
}

/*主程序入口*/
void main(){
	int * head_add;
	int malloc_size;
	int * add;
	int free_add;
	int choice;
	
	printf("请等待程序初始化...\n");
	head_add = mem_init();/*记录初始化时的首地址，程序结束时进行free */
	print_menu();
	

	
	choice = -1;
	scanf("%d",&choice);
	
	while(choice!=0){
		switch(choice){
			case 1:
				printf("输入需要申请多少字节？\n");
				scanf("%d",&malloc_size);
				add = mem_malloc(malloc_size);
				if(add!=NULL){
					printf("申请到的地址为%08dD,%08xH\n",add,add);
				}else{
					printf("申请不到内存空间\n");
				}	
				break;
			case 2:
				printf("输入需释放的首地址(十进制)：");
				free_add = -1;
				scanf("%d",&free_add);
				mem_free(free_add);
				break;
			case 3:
				mem_show();
				break;
			default:
				break;	
		}
		print_menu();
		scanf("%d",&choice);
	} 
	
	free(head_add);/*释放大内存块，小内存块 */
	
	free_list();/*释放存放heap内存块地址的链表 */
	free(p_head);/*释放链表头 */
	
	return 0;
}
