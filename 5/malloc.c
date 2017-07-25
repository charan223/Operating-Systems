#include "threads/malloc.h"
#include <debug.h>
#include <list.h>
#include <round.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/vaddr.h"


/* A simple implementation of malloc().

   The size of each request, in bytes, is rounded up to a power
   of 2 and assigned to the "descriptor" that manages blocks of
   that size.  The descriptor keeps a list of free blocks.  If
   the free list is nonempty, one of its blocks is used to
   satisfy the request.

   Otherwise, a new page of memory, called an "arena", is
   obtained from the page allocator (if none is available,
   malloc() returns a null pointer).  The new arena is divided
   into blocks, all of which are added to the descriptor's free
   list.  Then we return one of the new blocks.

   When we free a block, we add it to its descriptor's free list.
   But if the arena that the block was in now has no in-use
   blocks, we remove all of the arena's blocks from the free list
   and give the arena back to the page allocator.

   We can't handle blocks bigger than 2 kB using this scheme,
   because they're too big to fit in a single page with a
   descriptor.  We handle those by allocating contiguous pages
   with the page allocator and sticking the allocation size at
   the beginning of the allocated block's arena header. */

/* Descriptor. */
struct desc
  {
    size_t block_size;          /* Size of each element in bytes. */
    size_t blocks_per_arena;    /* Number of blocks in an arena. */
    struct list free_list;      /* List of free blocks. */
    struct lock lock;           /* Lock. */
  };

/* Magic number for detecting arena corruption. */
#define ARENA_MAGIC 0x9a548eed

/* Arena. */
struct arena 
  {
    unsigned magic;             /* Always set to ARENA_MAGIC. */
    struct desc *desc;          /* Owning descriptor, null for big block. */
    size_t free_cnt;            /* Free blocks; pages in big block. */
    struct block* addr[8];
  };

/* Free block. */
struct block 
  {
    struct list_elem free_elem; /* Free list element. */
  };

/* Our set of descriptors. */
static struct desc descs[10];   /* Descriptors. */
static size_t desc_cnt;         /* Number of descriptors. */

static struct arena* pages[1000];
static int pagenum;


static struct arena *block_to_arena (struct block *);
static struct block *arena_to_block (struct arena *, size_t idx);

int power(int a,int b){

int res=1;
while(b--){

 res*=a;

}

return res;

}


int logarithm(int b){

int res=0;

while(b!=1){

 b=b/2;
 res++;

}

return res-4;

}


/* Initializes the malloc() descriptors. */
void
malloc_init (void) 
{

  size_t block_size;
  //pagenum=0;
  for (block_size = 16; block_size <= PGSIZE / 2; block_size *= 2)
    {
      struct desc *d = &descs[desc_cnt++];
      ASSERT (desc_cnt <= sizeof descs / sizeof *descs);
      d->block_size = block_size;
      d->blocks_per_arena = (PGSIZE - sizeof (struct arena)) / block_size;

      //printf("hello=%d\n",d->block_size);
      list_init (&d->free_list);
      lock_init (&d->lock);
    }
}

/* Obtains and returns a new block of at least SIZE bytes.
   Returns a null pointer if memory is not available. */



void *
malloc (size_t size) 
{

  struct desc *d,*oldd;
  struct block *b,*b1;
  struct arena *a,*temp;
  int id,i;
  //int size=(int)sizerd;

  //printf("hello\n");

  /* A null pointer satisfies a request for 0 bytes. */
  if (size == 0)
    return NULL;

  /* Find the smallest descriptor that satisfies a SIZE-byte
     request. */
  for (d = descs; d < descs + desc_cnt; d++)
    if (d->block_size >= size)
      break;

 // printf("checkp1\n");
  
  if (d == descs + desc_cnt) 
    {
      return NULL;
    }

//printf("checkp2\n");

  lock_acquire (&d->lock);
  
  oldd=d;
  /* If the free list is empty, create a new arena. */
  
  if (!list_empty (&d->free_list))
    {

       
       b = list_entry (list_pop_front (&d->free_list), struct block, free_elem);
       lock_release (&oldd->lock);
   //    printMemory();
       return b;


    }
   
  else{
  // search for other upper free lists

     for(d=oldd;d<descs + desc_cnt; d++){

          if(!list_empty(&d->free_list))break;

     }

     

//printf("get=%d\n",d->block_size);
//printf("checkp3\n");

       if(d < descs + desc_cnt){

            //find non empty list
            //oldd--> required to return

           int k=1;

           while(d!=oldd){


               b = list_entry (list_pop_front (&d->free_list), struct block, free_elem);
               int h=d->block_size;
               b1=(struct block*) (b+h/2);
               // b , b1 -->buddys               
              

               temp=block_to_arena(b);


               if(k==1){ 
              temp->addr[logarithm(d->block_size)]=NULL;
                      k=0;
                       }

               d--;
               list_push_back (&d->free_list, &b1->free_elem);
               list_push_back (&d->free_list, &b->free_elem);

               temp->addr[logarithm(d->block_size)]=b;

           }

  //        printf("checkp4\n");

          b = list_entry (list_pop_front (&d->free_list), struct block, free_elem);
          lock_release (&oldd->lock);
         // printMemory();
          return b;          



       }

      
    
       else{

        // all are empty lists create a page
 //printf("checkp5\n");
       d--;
 //printf("checkd=%d\n",descs[0].block_size);
      /* Allocate a page. */
      a = palloc_get_page (0);
      if (a == NULL) 
        {
          lock_release (&oldd->lock);
          //printMemory();
          return NULL; 
        }

//printf("checkp6\n");

      
      pages[pagenum]=a;
      pagenum = pagenum +1;
  //    printf("pagenum=%d\n",pagenum);
      /* Initialize arena and add its blocks to the free list. */
      
      a->magic = ARENA_MAGIC;
      a->desc = d;
      a->free_cnt = d->blocks_per_arena;

  //  printf("new=%d\n",d->block_size);
  //  printf("checkp7\n");
      for(id=0;id < 8;id++)a->addr[id]=(struct block*)NULL;//..........................................................
       

  //  printf("checkp8\n");


    //printf("arena_blks=%d\n",d->blocks_per_arena);

      for (i = 0; i < 1; i++) 
        {
          struct block *b = arena_to_block (a, i);
         // b1=(struct block*) ((uint8_t*)b +(d->block_size/2));
         // if(id!=-1){

             a->addr[--id]=NULL; 
         //    id=-1;

         // }

          list_push_back (&d->free_list, &b->free_elem);
        }


//printf("checkp9\n");
      
        while(d!=oldd){

               b = list_entry (list_pop_front (&d->free_list), struct block, free_elem);
               if(b==NULL) printf("b is null\n");
               int k=d->block_size;
               b1=(struct block*) ((uint8_t*)b +(k/2));
               // b , b1 -->buddys               
               //temp->magic=ARENA_MAGIC;
               temp=block_to_arena(b);
               

               d--;

               list_push_back (&d->free_list, &b1->free_elem);
               list_push_back (&d->free_list, &b->free_elem);
              
               temp->addr[logarithm(d->block_size)]=b;
               
            //   printf("logarithm=%d\n",logarithm(d->block_size));

           }
        
          b = list_entry (list_pop_front (&d->free_list), struct block, free_elem);
          
          lock_release (&oldd->lock);
         // printMemory();
             
          return b;  

     }

}

}


/* Allocates and return A times B bytes initialized to zeroes.
   Returns a null pointer if memory is not available. */
void *
calloc (size_t a, size_t b) 
{
  void *p;
  size_t size;

  /* Calculate block size and make sure it fits in size_t. */
  size = a * b;
  if (size < a || size < b)
    return NULL;

  /* Allocate and zero memory. */
  p = malloc (size);
  if (p != NULL)
    memset (p, 0, size);

  return p;
}

/* Returns the number of bytes allocated for BLOCK. */

/*
static size_t
block_size (void *block) 
{
  struct block *b = block;
  struct arena *a = block_to_arena (b);
  struct desc *d = a->desc;

  return d != NULL ? d->block_size : PGSIZE * a->free_cnt - pg_ofs (block);
} */

/* Attempts to resize OLD_BLOCK to NEW_SIZE bytes, possibly
   moving it in the process.
   If successful, returns the new block; on failure, returns a
   null pointer.
   A call with null OLD_BLOCK is equivalent to malloc(NEW_SIZE).
   A call with zero NEW_SIZE is equivalent to free(OLD_BLOCK). */



void *
realloc (void *old_block, size_t new_size) 
{

int i,zim,vis[10];
struct block* barr[10],*b=NULL;
  if (new_size == 0) 
    {
      free (old_block);
      return NULL;
    }
  else 
    {
      void *new_block = malloc (new_size);
      if (old_block != NULL && new_block != NULL)
        {

          struct arena* a=block_to_arena(old_block);
         zim=0;

          for(i=0;i<8;i++)
      {
        if (a->addr[i]==NULL)continue;
        else{

          barr[zim]=a->addr[i];
          vis[zim]=i;
          zim++;

        }
      }
      
      for(i=zim-1;i>0;i--)
      {
       
        if(b>=a->addr[i])
        {
          
          if(b<a->addr[i-1])break;
          
        }
      }
          
        i=vis[i];

          size_t old_size = power(2,i+4);
          size_t min_size = new_size < old_size ? new_size : old_size;
          memcpy (new_block, old_block, min_size);
          free (old_block);
        }
      return new_block;
    }
}

/* Frees block P, which must have been previously allocated with
   malloc(), calloc(), or realloc(). */




void
free (void *p) 
{
  int i,zim,exp,recomm,vis[10];

  struct block*h,*buddy,*b,*barr[100];
  struct arena* s,*a;
  struct desc *f;
  

  if (p != NULL)
    {
      zim=0;
      b = p;
      buddy=p;
      a = block_to_arena (b);
/*    
      for(i=7;i>0;i--)
      {
        if (a->addr[i]==NULL)continue;
        if(b>=a->addr[i])
        {
          if((a->addr[i-1])==NULL)break;
          if(b<a->addr[i-1])break;
          
        }
      }
      */

  
      for(i=0;i<8;i++)
      {
        if (a->addr[i]==NULL)continue;
        else{

          barr[zim]=a->addr[i];
          vis[zim]=i;
          zim++;
        }

      }
      
      for(i=zim-1;i>0;i--)
      {
       
        if(b>=a->addr[i])
        {
          
          if(b<a->addr[i-1])break;
          
        }
      }

      i=vis[i];

      s=a+1;


//   printf("here\n");
//printf("here is %d\n",power(2,i));


    while(i < 8){

    exp=0;
    recomm=0;
    a = block_to_arena (b);
    f =&descs[i];
    int blksz=power(2,i+4);

    lock_acquire (&f->lock);
 //  printf("b=%p\n",(void*)b);
//  if(list_empty (&f->free_list))printf("yes\n");
    while(!list_empty (&f->free_list))////-............................................................
    {

      h = list_entry (list_pop_front (&f->free_list), struct block, free_elem);
      buddy=h;
      s=block_to_arena(h);
           
 //     printf("buddy=%p\n",(void*)h);
      //if(a==s)printf("same page\n");
      if(a==s && (  ((uint8_t* )b+blksz == (uint8_t*)h)  || ((uint8_t* )b-blksz == (uint8_t*)h)    ) ){
        recomm=1;
   //     printf("tis %d\n",i );
        break;
      }

       barr[exp++]=h;
      
    }


   if(recomm==0){
   // this means no buddy in the list
//printf("this %d\n",i );
   //this means buddy doesn't have to be there
if(i == 7){
// remove page with start address b--> remove page a


for(zim=exp-1;zim>=0;zim--){

h=barr[zim];
list_push_front (&f->free_list, &h->free_elem);

}


a=block_to_arena(b);
a->magic=ARENA_MAGIC;
a->desc=&descs[i];
a->free_cnt = descs[i].blocks_per_arena;


for(zim=0;zim<pagenum;zim++){

if(pages[zim]==a)break;

}



if(zim!=pagenum)pages[zim]=NULL;

palloc_free_page (a);

lock_release(&f->lock);

return;

}

    
//printf("here1\n");


   for(zim=0;zim<exp;zim++){

      h=barr[zim];
      list_push_back (&f->free_list, &h->free_elem);

         }

   list_push_back (&f->free_list, &b->free_elem);

   lock_release(&f->lock);   

   break;

   }

   else{
  //     printf("herelol\n");
     // this means buddy in the list
        
        for(zim=exp-1;zim>=0;zim--){
 
           h=barr[zim];
           list_push_front (&f->free_list, &h->free_elem);           

        }



     // merge buddies b and s , join to b
     
     if( (uint8_t*)b - blksz == (uint8_t*)buddy ){
           // s is left buddy of b
           b=buddy;
           
     }
     // if s is a right buddy we don't need to rename b but change a
     
      //printf("b is %p , buddy is %p\n",(void*)b,(void*)buddy );

   }

lock_release (&f->lock);

i++;

}


  return;        


    }



}



/* Returns the arena that block B is inside. */
static struct arena *
block_to_arena (struct block *b)
{
  struct arena *a = pg_round_down (b);

  /* Check that the arena is valid. */
  ASSERT (a != NULL);
  //ASSERT (a->magic == ARENA_MAGIC);

  /* Check that the block is properly aligned for the arena. */
  //ASSERT (a->desc == NULL
  //        || (pg_ofs (b) - sizeof *a) % a->desc->block_size == 0);
  //ASSERT (a->desc != NULL || pg_ofs (b) == sizeof *a);

  return a;
}

/* Returns the (IDX - 1)'th block within arena A. */
static struct block *
arena_to_block (struct arena *a, size_t idx) 
{
  ASSERT (a != NULL);
  ASSERT (a->magic == ARENA_MAGIC);
  ASSERT (idx < a->desc->blocks_per_arena);
  return (struct block *) ((uint8_t *) a
                           + sizeof *a
                           + idx * a->desc->block_size);
}



void printMemory(){


int i,j,k=0,p,q,s,exp;
struct block* arr[100],*brr[100],*b,*min;
struct arena *a,*temp;
struct desc* d;


for(i=0;i<pagenum;i++){


 if (pages[i]!=NULL)
    printf("page_%d:\n",i+1);
 else continue;


a=pages[i];
  


for(j=0;j< (int)desc_cnt;j++){
//iterate through 16,32,64,.... free lists and store blocks belonging to page a to arr


d=&(descs[j]);

if(d==NULL)continue;


k=0;
exp=0;


if(list_empty(&d->free_list))continue;


while(1){

if(list_empty(&d->free_list))break;


     b = list_entry (list_pop_front(&d->free_list), struct block, free_elem);
  
      temp=block_to_arena(b);

       brr[exp++]=b;

      if(a==temp)arr[k++]=b;

}

for(p=0;p<exp;p++){

list_push_back(&d->free_list,&brr[p]->free_elem);

}


//sort the arr in asc order of block addresses

if(k==0)continue;
printf("starting_address_%d_block:\n",power(2,j+4));

for(p=0;p<k;p++){

min=arr[p];
s=p;

for(q=p+1;q<k;q++){

if(min > arr[q]){
     
     min=arr[q];
     s=q;

}

}


if(s!=p){

arr[s]=arr[p];
arr[p]=min;

}

printf("%p\n",(void*)arr[p]);

}



}




}



}
