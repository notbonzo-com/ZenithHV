#include <mm/kalloc.h>
#include <stdint.h>
#include <debug.h>
#include <mem.h>

#define ALIGNMENT   16ul
#define ALIGN_TYPE  uint8_t
#define ALIGN_INFO  sizeof(ALIGN_TYPE)*16

#define USE_CASE1
#define USE_CASE2
#define USE_CASE3
#define USE_CASE4
#define USE_CASE5

#define ALIGN( ptr )													\
		if ( ALIGNMENT > 1 )											\
		{																\
			uintptr_t diff;												\
			ptr = (void*)((uintptr_t)ptr + ALIGN_INFO);					\
			diff = (uintptr_t)ptr & (ALIGNMENT-1);						\
			if ( diff != 0 )											\
			{															\
				diff = ALIGNMENT - diff;								\
				ptr = (void*)((uintptr_t)ptr + diff);					\
			}															\
			*((ALIGN_TYPE*)((uintptr_t)ptr - ALIGN_INFO)) = 			\
				diff + ALIGN_INFO;										\
		}

#define UNALIGN( ptr )													\
		if ( ALIGNMENT > 1 )											\
		{																\
			uintptr_t diff = *((ALIGN_TYPE*)((uintptr_t)ptr - ALIGN_INFO));	\
			if ( diff < (ALIGNMENT + ALIGN_INFO) )						\
			{															\
				ptr = (void*)((uintptr_t)ptr - diff);					\
			}															\
		}

#define KALLOC_MAGIC	0xc001c0de
#define KALLOC_DEAD	0xdeaddead

#define FLUSH()		    __asm__ ("nop\n")

struct major_block
{
	struct major_block *prev;
	struct major_block *next;
	unsigned int pages;
	unsigned int size;
	unsigned int usage;
	struct minor_block *first;
};

struct	minor_block
{
	struct minor_block *prev;
	struct minor_block *next;
	struct major_block *block;
	unsigned int magic;
	unsigned int size;
	unsigned int req_size;
};

static struct major_block *l_memRoot = NULL;
static struct major_block *l_bestBet = NULL;

static unsigned int l_pageSize  = 4096;
static unsigned int l_pageCount = 16;
static unsigned long long l_allocated = 0;
static unsigned long long l_inuse	 = 0;


static long long l_warningCount = 0;
static long long l_errorCount = 0;
static long long l_possibleOverruns = 0;

static void kalloc_dump()
{
	struct major_block *maj = l_memRoot;
	struct minor_block *min = NULL;

	log_info("Kernel Allocator", "------ Memory data ---------------\n");
	log_info("Kernel Allocator", "System memory allocated: %i bytes\n", l_allocated );
	log_info("Kernel Allocator", "Memory in used (malloc'ed): %i bytes\n", l_inuse );
	log_info("Kernel Allocator", "Warning count: %i\n", l_warningCount );
	log_info("Kernel Allocator", "Error count: %i\n", l_errorCount );
	log_info("Kernel Allocator", "Possible overruns: %i\n", l_possibleOverruns );

    while ( maj != NULL )
    {
        log_info("Kernel Allocator",  "0x%x: total = %i, used = %i\n",
                    maj,
                    maj->size,
                    maj->usage );

        min = maj->first;
        while ( min != NULL )
        {
            log_info("Kernel Allocator",  "0x%x: %i bytes\n",
                        min,
                        min->size );
            min = min->next;
        }

        maj = maj->next;
    }

	FLUSH();
}

static struct major_block *allocate_new_page( unsigned int size )
{
	unsigned int st;
	struct major_block *maj;

		st  = size + sizeof(struct major_block);
		st += sizeof(struct minor_block);

		if ( (st % l_pageSize) == 0 )
			st  = st / (l_pageSize);
		else
			st  = st / (l_pageSize) + 1;

		if ( st < l_pageCount ) st = l_pageCount;

		maj = (struct major_block*)kalloc_alloc( st );

		if ( maj == NULL )
		{
			l_warningCount += 1;
			return NULL;
		}

		maj->prev 	= NULL;
		maj->next 	= NULL;
		maj->pages 	= st;
		maj->size 	= st * l_pageSize;
		maj->usage 	= sizeof(struct major_block);
		maj->first 	= NULL;

		l_allocated += maj->size;

      return maj;
}

void *kmalloc(size_t req_size)
{
	int startedBet = 0;
	unsigned long long bestSize = 0;
	void *p = NULL;
	uintptr_t diff;
	struct major_block *maj;
	struct minor_block *min;
	struct minor_block *new_min;
	unsigned long size = req_size;

	if ( ALIGNMENT > 1 )
	{
		size += ALIGNMENT + ALIGN_INFO;
	}

	kalloc_lock();

	if ( size == 0 )
	{
		l_warningCount += 1;
		
		log_info("Kernel Allocator",  "WARNING: alloc( 0 ) called from 0x%x\n",
							__builtin_return_address(0) );
		FLUSH();
	
		kalloc_unlock();
		return kmalloc(1);
	}


	if ( l_memRoot == NULL )
	{
		FLUSH();

		l_memRoot = allocate_new_page( size );
		if ( l_memRoot == NULL )
		{
		  kalloc_unlock();
		  log_info("Kernel Allocator",  "initial l_memRoot initialization failed\n", p);
		  FLUSH();
		  return NULL;
		}

		log_info("Kernel Allocator",  "set up first memory major 0x%x\n", l_memRoot );
		FLUSH();
	
	}



	log_info("Kernel Allocator", "Call from \'0x%x\' kmalloc( %i ): ",
					__builtin_return_address(0),
					size);
	FLUSH();

	maj = l_memRoot;
	startedBet = 0;

	if ( l_bestBet != NULL )
	{
		bestSize = l_bestBet->size - l_bestBet->usage;

		if ( bestSize > (size + sizeof(struct minor_block)))
		{
			maj = l_bestBet;
			startedBet = 1;
		}
	}

	while ( maj != NULL )
	{
		diff  = maj->size - maj->usage;

		if ( bestSize < diff )
		{
			l_bestBet = maj;
			bestSize = diff;
		}


#ifdef USE_CASE1

		if ( diff < (size + sizeof( struct minor_block )) )
		{
		
			log_info("Kernel Allocator",  "CASE 1: Insufficient space in block 0x%x\n", maj);
			FLUSH();
		
			if ( maj->next != NULL )
			{
				maj = maj->next;
				continue;
			}

			if ( startedBet == 1 ) {
				maj = l_memRoot;
				startedBet = 0;
				continue;
			}

			maj->next = allocate_new_page( size );
			if ( maj->next == NULL ) break;
			maj->next->prev = maj;
			maj = maj->next;
		}

#endif

#ifdef USE_CASE2

		if ( maj->first == NULL )
		{
			maj->first = (struct minor_block*)((uintptr_t)maj + sizeof(struct major_block) );


			maj->first->magic 		= KALLOC_MAGIC;
			maj->first->prev 		= NULL;
			maj->first->next 		= NULL;
			maj->first->block 		= maj;
			maj->first->size 		= size;
			maj->first->req_size 	= req_size;
			maj->usage 	+= size + sizeof( struct minor_block );


			l_inuse += size;


			p = (void*)((uintptr_t)(maj->first) + sizeof( struct minor_block ));

			ALIGN( p );

		
			log_info("Kernel Allocator",  "CASE 2: returning 0x%x\n", p);
			FLUSH();
		
			kalloc_unlock();
			return p;
		}

#endif

#ifdef USE_CASE3

		diff =  (uintptr_t)(maj->first);
		diff -= (uintptr_t)maj;
		diff -= sizeof(struct major_block);

		if ( diff >= (size + sizeof(struct minor_block)) )
		{
			maj->first->prev = (struct minor_block*)((uintptr_t)maj + sizeof(struct major_block) );
			maj->first->prev->next = maj->first;
			maj->first = maj->first->prev;

			maj->first->magic 	= KALLOC_MAGIC;
			maj->first->prev 	= NULL;
			maj->first->block 	= maj;
			maj->first->size 	= size;
			maj->first->req_size 	= req_size;
			maj->usage 			+= size + sizeof( struct minor_block );

			l_inuse += size;

			p = (void*)((uintptr_t)(maj->first) + sizeof( struct minor_block ));
			ALIGN( p );

		
			log_info("Kernel Allocator",  "CASE 3: returning 0x%x\n", p);
			FLUSH();
		
			kalloc_unlock();
			return p;
		}

#endif


#ifdef USE_CASE4
		min = maj->first;
		while ( min != NULL )
		{
				if ( min->next == NULL )
				{
					diff = (uintptr_t)(maj) + maj->size;
					diff -= (uintptr_t)min;
					diff -= sizeof( struct minor_block );
					diff -= min->size;

					if ( diff >= (size + sizeof( struct minor_block )) )
					{
						min->next = (struct minor_block*)((uintptr_t)min + sizeof( struct minor_block ) + min->size);
						min->next->prev = min;
						min = min->next;
						min->next = NULL;
						min->magic = KALLOC_MAGIC;
						min->block = maj;
						min->size = size;
						min->req_size = req_size;
						maj->usage += size + sizeof( struct minor_block );

						l_inuse += size;

						p = (void*)((uintptr_t)min + sizeof( struct minor_block ));
						ALIGN( p );

					
						log_info("Kernel Allocator",  "CASE 4.1: returning 0x%x\n", p);
						FLUSH();
					
						kalloc_unlock();
						return p;
					}
				}

				if ( min->next != NULL )
				{
					diff  = (uintptr_t)(min->next);
					diff -= (uintptr_t)min;
					diff -= sizeof( struct minor_block );
					diff -= min->size;

					if ( diff >= (size + sizeof( struct minor_block )) )
					{
						new_min = (struct minor_block*)((uintptr_t)min + sizeof( struct minor_block ) + min->size);

						new_min->magic = KALLOC_MAGIC;
						new_min->next = min->next;
						new_min->prev = min;
						new_min->size = size;
						new_min->req_size = req_size;
						new_min->block = maj;
						min->next->prev = new_min;
						min->next = new_min;
						maj->usage += size + sizeof( struct minor_block );

						l_inuse += size;

						p = (void*)((uintptr_t)new_min + sizeof( struct minor_block ));
						ALIGN( p );


					
						log_info("Kernel Allocator",  "CASE 4.2: returning 0x%x\n", p);
						FLUSH();
					

						kalloc_unlock();
						return p;
					}
				}

				min = min->next;
		}


#endif

#ifdef USE_CASE5

		if ( maj->next == NULL )
		{
		
			log_info("Kernel Allocator", "CASE 5: block full\n");
			FLUSH();
		

			if ( startedBet == 1 )
			{
				maj = l_memRoot;
				startedBet = 0;
				continue;
			}

			maj->next = allocate_new_page( size );
			if ( maj->next == NULL ) break;
			maj->next->prev = maj;

		}

#endif

		maj = maj->next;
	}



	kalloc_unlock();


	log_info("Kernel Allocator",  "All cases exhausted. No memory available.\n");
	FLUSH();

	
	log_info("Kernel Allocator",  "WARNING: kmalloc( %i ) returning NULL.\n", size);
	kalloc_dump();
	FLUSH();

	return NULL;
}

void kfree(void *ptr)
{
	struct minor_block *min;
	struct major_block *maj;

	if ( ptr == NULL )
	{
		l_warningCount += 1;
		log_info("Kernel Allocator",  "WARNING: kfree( NULL ) called from 0x%x\n",
							__builtin_return_address(0) );
		FLUSH();
		return;
	}

	UNALIGN( ptr );

	kalloc_lock();


	min = (struct minor_block*)((uintptr_t)ptr - sizeof( struct minor_block ));


	if ( min->magic != KALLOC_MAGIC )
	{
		l_errorCount += 1;

		if (
			((min->magic & 0xFFFFFF) == (KALLOC_MAGIC & 0xFFFFFF)) ||
			((min->magic & 0xFFFF) == (KALLOC_MAGIC & 0xFFFF)) ||
			((min->magic & 0xFF) == (KALLOC_MAGIC & 0xFF))
		   )
		{
			l_possibleOverruns += 1;
			log_info("Kernel Allocator", "ERROR: Possible 1-3 byte overrun for magic 0x%x != 0x%x\n",
								min->magic,
								KALLOC_MAGIC );
			FLUSH();
		}


		if ( min->magic == KALLOC_DEAD )
		{
			log_info("Kernel Allocator", "ERROR: multiple kfree() attempt on 0x%x from 0x%x.\n",
									ptr,
									__builtin_return_address(0) );
			FLUSH();
		}
		else
		{
			log_info("Kernel Allocator", "ERROR: Bad kfree( 0x%x ) called from 0x%x\n",
								ptr,
								__builtin_return_address(0) );
			FLUSH();
		}

		kalloc_unlock();
		return;
	}

	log_info("Kernel Allocator", "Called from: \'0x%x\' kfree( 0x%x ): ",
				__builtin_return_address( 0 ),
				ptr );
	FLUSH();


	maj = min->block;

	l_inuse -= min->size;

	maj->usage -= (min->size + sizeof( struct minor_block ));
	min->magic  = KALLOC_DEAD;

	if ( min->next != NULL ) min->next->prev = min->prev;
	if ( min->prev != NULL ) min->prev->next = min->next;

	if ( min->prev == NULL ) maj->first = min->next;



	if ( maj->first == NULL )
	{
		if ( l_memRoot == maj ) l_memRoot = maj->next;
		if ( l_bestBet == maj ) l_bestBet = NULL;
		if ( maj->prev != NULL ) maj->prev->next = maj->next;
		if ( maj->next != NULL ) maj->next->prev = maj->prev;
		l_allocated -= maj->size;

		kalloc_free( maj, maj->pages );
	}
	else
	{
		if ( l_bestBet != NULL )
		{
			int bestSize = l_bestBet->size  - l_bestBet->usage;
			int majSize = maj->size - maj->usage;

			if ( majSize > bestSize ) l_bestBet = maj;
		}

	}

	kalloc_unlock();
}


void* kcalloc(size_t nobj, size_t size)
{
       int real_size;
       void *p;

       real_size = nobj * size;

       p = kmalloc( real_size );

       memset( p, 0, real_size );

       return p;
}


void* krealloc(void *p, size_t size)
{
	void *ptr;
	struct minor_block *min;
	unsigned int real_size;

	if ( size == 0 )
	{
		kfree( p );
		return NULL;
	}

	if ( p == NULL ) return kmalloc( size );

	ptr = p;
	UNALIGN(ptr);

	kalloc_lock();

		min = (struct minor_block*)((uintptr_t)ptr - sizeof( struct minor_block ));

		if ( min->magic != KALLOC_MAGIC )
		{
			l_errorCount += 1;

			if (
				((min->magic & 0xFFFFFF) == (KALLOC_MAGIC & 0xFFFFFF)) ||
				((min->magic & 0xFFFF) == (KALLOC_MAGIC & 0xFFFF)) ||
				((min->magic & 0xFF) == (KALLOC_MAGIC & 0xFF))
			   )
			{
				l_possibleOverruns += 1;
				log_info("Kernel Allocator",  "ERROR: Possible 1-3 byte overrun for magic 0x%x != 0x%x\n",
									min->magic,
									KALLOC_MAGIC );
				FLUSH();
			}


			if ( min->magic == KALLOC_DEAD )
			{
				log_info("Kernel Allocator",  "ERROR: multiple kfree() attempt on 0x%x from 0x%x.\n",
										ptr,
										__builtin_return_address(0) );
				FLUSH();
			}
			else
			{
				log_info("Kernel Allocator",  "ERROR: Bad kfree( 0x%x ) called from 0x%x\n",
									ptr,
									__builtin_return_address(0) );
				FLUSH();
			}

			kalloc_unlock();
			return NULL;
		}

		real_size = min->req_size;

		if ( real_size >= size )
		{
			min->req_size = size;
			kalloc_unlock();
			return p;
		}

	kalloc_unlock();

	ptr = kmalloc( size );
	memcpy( ptr, p, real_size );
	kfree( p );

	return ptr;
}