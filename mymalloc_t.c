#include<stdio.h>
#include<stdlib.h>
#include<semaphore.h>
#include"mymalloc_t.h"

/*ȫ�ֱ���*/ 
int hasInited = 0;/*��¼�ڴ������Ƿ��Ѿ�����ʼ�� */
small_block_manage * small_manage;/*ָ��С�ڴ����������ָ�� */
big_block_manage * big_manage; /*ָ����ڴ����������ָ�� */ 

SEM_ID sem_small ;
SEM_ID sem_big;
SEM_ID sem_heap;

heap_add_node * p_head = NULL; 
heap_add_node * p_tail = NULL;



/*����ÿһ���ڴ����׵�ַ��head_addΪ�����һ����ڴ���׵�ַ*/ 
void calculate_address(int* head_add){
	
	int block_index = 0; 
	small_block * small_temp;
	big_block * big_temp;
	
	small_manage = head_add;
	big_manage = small_manage + 1;
	small_temp = big_manage + 1;/*��һ��С�ڴ����׵�ַ����һ��С�ڴ��ָ��ָ�� */
	big_temp = small_temp + SMALLNUM;/*��һ�����ڴ����׵�ַ����һ��С�ڴ��ָ��ָ�� */	
	
	
	/*��ʼ��С�ڴ�����ṹ*/
	(*small_manage).base_add = small_temp;
	(*small_manage).busy_num = 0;
	
	/*��ʼ�����ڴ�����ṹ*/
	(*big_manage).base_add = big_temp;
	(*big_manage).busy_num = 0;	
	

	for(block_index = 0;block_index < SMALLNUM;block_index++){
		(*small_manage).busy[block_index] = BLOCKAVAILABLE;/*��ʼ������ÿһ��С�ڴ����Ϊ���� */
		/*printf("С�ڴ���ַ��%d\n",(*small_manage).base_add+block_index);*/
	}
	
	for(block_index = 0;block_index < BIGNUM;block_index++){
		(*big_manage).busy[block_index] = BLOCKAVAILABLE;/*��ʼ������ÿһ�����ڴ����Ϊ���� */
		/*printf("���ڴ���ַ��%d\n",(*big_manage).base_add+block_index);*/
	}
	
	printf("==========================================================\n");
	printf("||\tС�ڴ�������׵�ַ��%08dD,%08xH\t||\n",small_manage,small_manage);
	printf("||\t���ڴ�������׵�ַ��%08dD,%08xH\t||\n",big_manage,big_manage);
	printf("||\tС�ڴ��������׵�ַ��%08dD,%08xH\t||\n",(*small_manage).base_add,(*small_manage).base_add);
	printf("||\t���ڴ��������׵�ַ��%08dD,%08xH\t||\n",(*big_manage).base_add,(*big_manage).base_add);
	printf("==========================================================\n");
	/* printf("�ڶ������ڴ��������׵�ַ��%08xH,%08dD\n",big_temp+1,big_temp + 1);*/
}

/*�ڴ��ʼ��ģ�飬����Ҫ������ڴ������С�������ڴ��׵�ַ*/
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
		printf("�Ѿ�����ڴ�����ʼ����\n");
		return; 
	} 
	
	
	/*��������Ҫ��������ֽڵ��ڴ��ַ*/ 
	small_size = SMALLNUM*sizeof(small_block);
	big_size = BIGNUM*sizeof(big_block);
	manage_size = sizeof(small_block_manage) + sizeof(big_block_manage);
	pre_alloc_size = small_size + big_size + manage_size;
	
	printf(" ========================================================\n"); 
	printf("||\tС�ڴ��������%d��ÿ��ռ��%d�ֽڣ���%d�ֽ�\t||\n",SMALLNUM,sizeof(small_block),small_size); 
	printf("||\t���ڴ��������%d��ÿ��ռ��%d�ֽڣ���%d�ֽ�\t||\n",BIGNUM,sizeof(big_block),big_size); 
	printf("||\tС�ڴ�����ռ��%d�ֽ�\t\t\t\t||\n",sizeof(small_block_manage));
	printf("||\t���ڴ�����ռ��%d�ֽ�\t\t\t\t||\n",sizeof(big_block_manage));
	printf("||\t��ʼ��ʱ�ܹ�������%d�ֽڵ��ڴ�����\t\t||\n",pre_alloc_size);
	printf(" =========================================================\n");
	
	p_head = (struct heap_add_node *)malloc(sizeof(heap_add_node));  
	p_head -> next = NULL;
	p_tail = p_head;

	/*array������pre_alloc_size��С���ڴ�鷵�ص��׵�ַ*/
	
	array = (int *)malloc(pre_alloc_size);
	if(array == NULL){
		printf("���ܳɹ�����洢�ռ䡣\n");
		return NULL;
	}else{
		hasInited = 1;
		calculate_address(array); 
	
		printf(" ========================================================\n");
		printf("||\t�ڴ�����ʼ����ɣ���ʼ��ַ��%08dD\t\t||\n",array);
		printf("||\t�ڴ�����ʼ����ɣ���ʼ��ַ��%08xH\t\t||\n",array);
		printf(" ========================================================\n");
		return array;
	}
} 

/*�����������ڴ棬�������뵽���ڴ����׵�ַ*/
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
            			printf("��heap�����ڴ�ʧ��,������ֹ!\n");  
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
	�����ڴ�����
	1.С��SMALLSIZE�Ĳ�������С�ڴ�飬���С�ڴ������������������ڴ��
	2.����SMALLSIZEС��BIGSIZE������ڴ�飬������ڴ�������������heap�����ڴ��
	3.�Ӷ������ڴ�飬�����ӵ�heap�ڴ�������� 
*/ 
int* mem_malloc(int size){
	
	int small_busy;
	int big_busy;
	int flag;
	
	small_busy = 0;
	big_busy = 0;
	flag = SMALL_FREE;
	
	if(size > 0 && size <= SMALLSIZE){
		printf("��������С�ڴ��...\n");
		small_busy = (*small_manage).busy_num;
		if(small_busy == SMALLNUM){
			printf("С�ڴ����...\n");
			flag = SMALL_FULL;
		}else{
			return malloc_by_type(SMALLAREA,size);
		}	
	}
	if((size > SMALLSIZE && size <= BIGSIZE)||flag == SMALL_FULL){
		printf("����������ڴ��...\n");
		big_busy = (*small_manage).busy_num;
		if(big_busy == BIGNUM){
			printf("���ڴ����...\n");
			flag = BIG_FULL;
		}else{
			return malloc_by_type(BIGAREA,size); 
		} 
	}
	if(size > BIGSIZE||flag == BIG_FULL){
		printf("���ڴ�heap�����ڴ��...\n");
		return malloc_by_type(HEAPAREA,size);		
	}
}

/*
	���ȼ����ַ�Ϸ��ԣ����Ϸ�return;
	�Ϸ��ж�������������һ�����ϵ��ڴ�飬ִ��free���� 
*/ 
void mem_free(int free_add){
	/*printf("���ڼ����ַ�Ϸ���...\n");*/


	if(free_add >= (*small_manage).base_add && free_add <= (*small_manage).base_add + SMALLNUM-1){
		int index = 0;

		small_block* small_first = (*small_manage).base_add;

		for(index = 0;index<SMALLNUM;index++){
			if(small_first+index==free_add){
				(*small_manage).busy_num--;
				(*small_manage).busy[index]=BLOCKAVAILABLE;
				printf("�ѳɹ��ͷ�һ��С�ڴ�\n"); 
			}
		} 
	}else if(free_add >= (*big_manage).base_add && free_add <= (*big_manage).base_add + BIGNUM-1){
		int index = 0;
		big_block* big_first = (*big_manage).base_add;
		for(index = 0;index<BIGNUM;index++){
			if(big_first+index==free_add){
				(*big_manage).busy_num--;
				(*big_manage).busy[index]=BLOCKAVAILABLE;
				printf("�ѳɹ��ͷ�һ����ڴ�\n"); 
			}
		} 
	} 

	/*
		���ж��Ƿ��Ǵ�heap����ģ�
		����ǣ����ͷ�
		������ʾ�����ַ��Ч
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
            	printf("�ѳɹ��ͷ�һ��heap�ڴ�\n"); 
            	return;
            } else{
            	p = p->next;  
            } 
        } 
		printf("���׵�ַ��Ч��\n"); 
	}else{
		printf("���׵�ַ��Ч��\n"); 
	}
}


/*������¼��heap������ڴ�������*/
void traverse_list()  
{  
	
   	heap_add_node * p = p_head->next;  

    	if(p_head ->next == NULL){  
        	printf("����Ϊ�գ�\n");  
   	}else{  
       		printf("��heap���뵽���ڴ���Ϣ���£�\n");  
       		while(p!=NULL)  
        	{  
            		printf("�׵�ַ��%08dD���׵�ַ��%08xH��ռ�ã�%d�ֽ�\n",p->heap_add,p->heap_add,p->size);  
            		p = p->next;  
        	}  
   	 }  
}  

/*�ͷż�¼��heap������ڴ�������*/
void free_list(){
	heap_add_node * p = p_head; 
	heap_add_node *p_temp = NULL; 
	while(p->next != NULL){
		p_temp = p->next;
        free(p_temp->heap_add);
        p->next = p_temp->next;
        free(p_temp);
        printf("�ѳɹ��ͷ�һ��heap�ڴ�\n"); 
	} 
}

/*չʾ�ڴ��ռ�����*/ 
void mem_show(){
	double f1;
	double f2;
	int index;
	
	if(hasInited == 0){
		printf("�ڴ���δ��ʼ����\n");
		return; 
	}else if(hasInited > 0){
		printf("\n============================�ڴ�ʹ���������============================\n");
		f1 = (double)(*small_manage).busy_num/(double)SMALLNUM*100;;
		printf("\nС�ڴ�鹲%d������ռ��%d����ռ����%.2f%%\n",SMALLNUM,(*small_manage).busy_num,f1); 
		
		for(index=0; index<SMALLNUM; index++){
			if((*small_manage).busy[index]==BLOCKAVAILABLE) 
				printf("%d��:����\t",index);
			if((*small_manage).busy[index]==BLOCKBUSY)
				printf("%d��:ռ��\t",index);
			if((index+1)%4==0)printf("\t||\n"); 
		} 
		printf("\n");
		f2 = (double)(*big_manage).busy_num/(double)BIGNUM*100;;
		printf("���ڴ�鹲%d������ռ��%d����ռ����%.2f%%\n",BIGNUM,(*big_manage).busy_num,f2); 
		for(index=0; index<BIGNUM; index++){
			if((*big_manage).busy[index]==BLOCKAVAILABLE) 
				printf("%d��:����\t",index);
			if((*big_manage).busy[index]==BLOCKBUSY)
				printf("%d��:ռ��\t",index);
			if((index+1)%4==0)printf("\t||\n"); 
		} 
		printf("\n");
		
		traverse_list();
		printf("========================================================================\n");
	}else{
		printf("�ڴ��ʼ���������⣡\n");
	}	
}

/*��ӡ�û�ѡ��˵�*/ 
void print_menu(){
	printf("*********************************************************\n");
	printf("*\t\t\t0.�˳�����\t\t\t*\n");
	printf("*\t\t\t1.�����ڴ档\t\t\t*\n");
	printf("*\t\t\t2.�ͷ��ڴ档\t\t\t*\n");
	printf("*\t\t\t3.չʾռ�á�\t\t\t*\n");
	printf("*********************************************************\n");
	printf("������0-3��������");
}

/*���������*/
void main(){
	int * head_add;
	int malloc_size;
	int * add;
	int free_add;
	int choice;
	
	printf("��ȴ������ʼ��...\n");
	head_add = mem_init();/*��¼��ʼ��ʱ���׵�ַ���������ʱ����free */
	print_menu();
	

	
	choice = -1;
	scanf("%d",&choice);
	
	while(choice!=0){
		switch(choice){
			case 1:
				printf("������Ҫ��������ֽڣ�\n");
				scanf("%d",&malloc_size);
				add = mem_malloc(malloc_size);
				if(add!=NULL){
					printf("���뵽�ĵ�ַΪ%08dD,%08xH\n",add,add);
				}else{
					printf("���벻���ڴ�ռ�\n");
				}	
				break;
			case 2:
				printf("�������ͷŵ��׵�ַ(ʮ����)��");
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
	
	free(head_add);/*�ͷŴ��ڴ�飬С�ڴ�� */
	
	free_list();/*�ͷŴ��heap�ڴ���ַ������ */
	free(p_head);/*�ͷ�����ͷ */
	
	return 0;
}
