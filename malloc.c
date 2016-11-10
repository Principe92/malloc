// Princewill Okorie
// Lab 4

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

typedef struct node {
	struct node *next;
	int size;
	bool free;
	void *ptr;
} Node;

Node *head;

int getWord(int size, int pageSize){
	int i = 1;
	int result = pageSize;

	while (result < size) result = pageSize * ++i;

	return result;
}

void *roundPointer(void *ptr){
	while ((uintptr_t)ptr % 8 != 0) ptr++;

	return ptr;
}

void splitMem(Node *cnt, int size){
        //divide and relink

	int nextSize = cnt->size - size - sizeof(Node);
	if (nextSize > 8){ 
	        Node *new = (Node *)(cnt->ptr + size);
        	new->next = cnt->next;
	        new->size = nextSize;
        	new->free = true;
	        new->ptr = roundPointer(cnt->ptr + size + sizeof(Node));

		cnt->next = new;
	        cnt->size = size;
	}
}


void free(void *ptr){
	Node *cnt = head;

	while (cnt != NULL){
		if (cnt->ptr == ptr){
			cnt->free = true;
			return;
		}

		cnt = cnt->next;
	}

	return;
}


void *malloc(size_t size){
	Node *cnt = head;
	Node *prev = NULL;
	int pageSize = sysconf(_SC_PAGESIZE);
	int totalSize = size + sizeof(Node);

	while (cnt != NULL){
		if (cnt->free){

			if (cnt->size >= size){

				if (cnt->size > size) splitMem(cnt, size);
				
				cnt->free = false;	
					
				return cnt->ptr;
			}
		}
		
		prev = cnt;
		cnt = cnt->next;
	}

	int loc = getWord(totalSize, pageSize);
	void *ptr = sbrk(loc);

	if (ptr != (void *)-1){
		cnt = (Node *)ptr;
		cnt->next = NULL;
		cnt->free = false;
		cnt->size = loc - sizeof(Node);
		cnt->ptr = roundPointer(ptr + sizeof(Node));
		
		if (pageSize > totalSize) splitMem(cnt, size);

		if (head == NULL) head = cnt;
                else prev->next = cnt;

		return cnt->ptr;
	}

	errno = ENOMEM;
	return NULL;
}

void *calloc(size_t elts, size_t size ){

	if (elts > 0 && size > 0){
		int total = elts * size;
		void *ptr = malloc(total);

		if (ptr != NULL) return memset(ptr, 0, total);
	}

	return NULL;
}

void *realloc(void *ptr, size_t size){
	if (ptr == NULL) return malloc(size);

	if (size == 0){
		free(ptr);
		return ptr;
	}

 	Node *cnt = head;

        while (cnt != NULL){
                if (cnt->ptr == ptr){
                       break;
                }

                cnt = cnt->next;
        }

	
	if (cnt != NULL){
		if (cnt->size > size && cnt->size - size > 8){
			splitMem(cnt, size);
		}

		else if (cnt->size < size){
			void *new = malloc(size);
			return memcpy(new, cnt->ptr, cnt->size);
		}

		else return ptr;
	}

        return NULL;
}

