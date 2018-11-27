#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)      ((b) + 1)
#define BLOCK_HEADER(ptr)   ((struct block *)(ptr) - 1)


static int atexit_registered = 0;
static int num_mallocs       = 0;
static int num_frees         = 0;
static int num_reuses        = 0;
static int num_grows         = 0;
static int num_splits        = 0;
static int num_coalesces     = 0;
static int num_blocks        = 0; //will increment for each heap mang. strategy sperratly 
static int num_requested     = 0;
static int max_heap          = 0;

/*
 *  \brief printStatistics
 *
 *  \param none
 *
 *  Prints the heap statistics upon process exit.  Registered
 *  via atexit()
 *
 *  \return none
 */
void printStatistics( void )
{
  printf("\nheap management statistics\n");
  printf("mallocs:\t%d\n", num_mallocs );
  printf("frees:\t\t%d\n", num_frees );
  printf("reuses:\t\t%d\n", num_reuses );
  printf("grows:\t\t%d\n", num_grows );
  printf("splits:\t\t%d\n", num_splits );
  printf("coalesces:\t%d\n", num_coalesces );
  printf("blocks:\t\t%d\n", num_blocks );
  printf("requested:\t%d\n", num_requested );
  printf("max heap:\t%d\n", max_heap );
}

struct block 
{
   size_t      size;  /* Size of the allocated block of memory in bytes */
   struct block *next;  /* Pointer to the next block of allcated memory   */
   bool        free;  /* Is this block free?                     */
};


struct block *FreeList = NULL; /* Free list to track the blocks available */

/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free blocks
 * \param size size of the block needed in bytes 
 *
 * \return a block that fits the request or NULL if no free block matches
 *
 * \TODO Implement Next Fit
 * \TODO Implement Best Fit
 * \TODO Implement Worst Fit
 */
struct block *findFreeBlock(struct block **last, size_t size) 
{
   num_requested = num_requested + size; //amt of mem requested
   struct block *curr = FreeList;

#if defined FIT && FIT == 0
   /* First fit */
   while (curr && !(curr->free && curr->size >= size)) 
   {
      num_blocks++;
      *last = curr;
      curr  = curr->next;
   }
#endif

#if defined BEST && BEST == 0
   //printf("TODO: Implement best fit here\n");

   int min = INT_MAX; //set min to the max num
   struct block* hit = NULL; //will store the block which has the smalled num after subtracting the curr->size and size
   while(curr)
   {
     if(curr->free)
     {
	num_blocks++;
       if(curr->size - size < min)
       {
         min = curr->size - size;
         hit = curr; 
       }
     }
     curr = curr->next;
   }
   curr = hit;
   if(curr)
   {
     *last = curr; //link the last block to the current
   }

#endif

#if defined WORST && WORST == 0
   //printf("TODO: Implement worst fit here\n");

   int max = 0;
   struct block* hit = NULL;
   while(curr)
   {
     if(curr->free)
     {
       num_blocks++;
       if(curr->size - size > max)
       {
         max = curr->size - size;
         hit = curr;
       }
     }
     curr = curr->next;
   }
   curr = hit;

   if(curr)
   {
     *last = curr;
   }

#endif

#if defined NEXT && NEXT == 0
   printf("TODO: Implement next fit here\n");
#endif

   return curr;
}

/*
 * \brief growheap
 *
 * Given a requested size of memory, use sbrk() to dynamically 
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated block of NULL if failed
 */
struct block *growHeap(struct block *last, size_t size) 
{
    num_grows++; //gorws heap
    max_heap = max_heap + size;
   /* Request more space from OS */
   struct block *curr = (struct block *)sbrk(0);
   struct block *prev = (struct block *)sbrk(sizeof(struct block) + size);

   assert(curr == prev);

   /* OS allocation failed */
   if (curr == (struct block *)-1) 
   {
      return NULL;
   }

   /* Update FreeList if not set */
   if (FreeList == NULL) 
   {
      FreeList = curr;
   }

   /* Attach new block to prev block */
   if (last) 
   {
      last->next = curr;
   }

   /* Update block metadata */
   curr->size = size;
   curr->next = NULL;
   curr->free = false;
   return curr;
}

/*
 * \brief malloc
 *
 * finds a free block of heap memory for the calling process.
 * if there is no free block that satisfies the request then grows the 
 * heap and returns a new block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */
void *malloc(size_t size) 
{

   if( atexit_registered == 0 )
   {
      atexit_registered = 1;
      atexit( printStatistics );
   }

   /* Align to multiple of 4 */
   size = ALIGN4(size);

   /* Handle 0 size */
   if (size == 0) 
   {
      return NULL;
   }

   /* Look for free block */
   struct block *last = FreeList;//this mean this is the last free( and first free block), after this there could be more free blocks
   struct block *next = findFreeBlock(&last, size);

   /* TODO: Split free block if possible */
   if(last != NULL)
   {
      if(last->size > size)
      {
	 size_t size_diff = last->size - size;
	 //last =(char*) (size);
	 //next = (struct block*)(size_diff);
         //struct block *store_diff = (struct block*)((last->size) - (size));
	 	  
      }      
   }
   
   /* Could not find free block, so grow heap */
   if (next == NULL) 
   {
      next = growHeap(last, size);
//      num_grows++;   
   }

   /* Could not find free block or grow heap, so just return NULL */
   if (next == NULL) 
   {
      return NULL;
   }
   else
   {
      num_mallocs++;
   }
   
   /* Mark block as in use */
   next->free = false;
   //num_mallocs++;
   /* Return data address associated with block */
   return BLOCK_DATA(next);
}

/*
 * \brief free
 *
 * frees the memory block pointed to by pointer. if the block is adjacent
 * to another block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr) 
{
   if (ptr == NULL) 
   {
      return;
   }

   /* Make block as free */
   struct block *curr = BLOCK_HEADER(ptr);
   num_frees++; //num of times free is called

   assert(curr->free == 0); //if current block is not free then exit
   curr->free = true;

   /* TODO: Coalesce free blocks if needed */
   struct block *s;
   int i;
   for (i = 0; i < 2; i++) 
   {
        for (s = curr->next; s != NULL; s = s->next) 
        {
            if (s->next != NULL) 
            {
                if (((((char*) s) + s->size) == (char*) s->next) && (s->size == s->next->size)) 
                {
			printf("inside 2nd if\n");
                    s->size = s->size + s->next->size; //update the size
                    s->next = s->next->next; //merge them
		    num_coalesces++;                
		}
            }
        }
    }
}

/* vim: set expandtab sts=3 sw=3 ts=6 ft=cpp: --------------------------------*/
