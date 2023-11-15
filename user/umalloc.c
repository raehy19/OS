#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

// Memory allocator by Kernighan and Ritchie,
// The C programming Language, 2nd ed.  Section 8.7.

typedef long Align;

union header {
  struct {
    union header *ptr;
    uint size;
  } s;
  Align x;
};

typedef union header Header;

static Header base;
static Header *freep;

void
freelist(void) { // Show the chain of free blocks.
  int i = 0;
  Header* p = &base;

  printf("Free list:\n");
  if(!freep) { printf("--\n"); return; } // Free list hasn't been created.
  for(p = p->s.ptr; p != &base; p = p->s.ptr) {
    printf("[%d] p = %p, p->s.size = %d bytes, p->s.ptr = %p\n",
           ++i, p, sizeof(Header) * p->s.size, p->s.ptr);
  } printf("\n");
}

void
free(void *ap)
{
  Header *bp, *p;

  bp = (Header*)ap - 1;
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
      break;
  if(bp + bp->s.size == p->s.ptr){
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr = p->s.ptr->s.ptr;
  } else
    bp->s.ptr = p->s.ptr;
  if(p + p->s.size == bp){
    p->s.size += bp->s.size;
    p->s.ptr = bp->s.ptr;
  } else
    p->s.ptr = bp;
  freep = p;
}

static Header*
morecore(uint nu)
{
  char *p;
  Header *hp;

  if(nu < 4096)
    nu = 4096;
  p = sbrk(nu * sizeof(Header));
  if(p == (char*)-1)
    return 0;
  hp = (Header*)p;
  hp->s.size = nu;
  free((void*)(hp + 1));
  return freep;
}

//////////     Assignment 4 : Free List     //////////

void *
malloc(uint nbytes) {
	// Pointer for iterating Free List
	Header *p, *prevp;
	uint nunits;

	// Variables for Best Fit
	Header *best_fit = 0;          // Pointer of best fit block
	Header *best_fit_prev = 0;     // Pointer of previous block of best fit block

	// Calculate size of memory block
	nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;

	// If freep == 0, the memory is not allocated, initialize base (Free List)
	if ((prevp = freep) == 0) {
		base.s.ptr = freep = prevp = &base;
		base.s.size = 0;
	}

	// Infinite iterating Free List to find best fit
	// If we can't find big enough memory block in Free List,
	// Request additional memory allocation to find new best fit
	for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {

		// If memory block is big enough
		if (p->s.size >= nunits) {

			// If it is first block or smaller than the current best fit
			if (best_fit == 0 || p->s.size < best_fit->s.size) {

				// Update best fit block and previous block of best fit block
				best_fit = p;
				best_fit_prev = prevp;
			}
		}

		// If iterated(searched) for whole Free List
		if (p == freep) {

			// If big enough best fit block has been found
			if (best_fit != 0) {

				// If block size is exactly equal to requested
				if (best_fit->s.size == nunits) {

					// Use block and remove from Free List
					best_fit_prev->s.ptr = best_fit->s.ptr;
				}
					// If block is bigger than requested
				else {

					// Reduce size, and move pointer to end of free part of the block
					best_fit->s.size -= nunits;
					best_fit += best_fit->s.size;

					// The size of the block to be returned is exactly what was requested
					best_fit->s.size = nunits;
				}

				// Update the start of the free list
				freep = best_fit_prev;

				// Return the block, excluding the header
				return (void *)(best_fit + 1);
			}

			// If no memory block is greater than or equal to the requested size, request additional memory
			// If additional memory request failed, return 0
			if ((p = morecore(nunits)) == 0)
				return 0;
		}
	}
}

//////////     //////////    //////////     //////////
