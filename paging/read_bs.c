#include <conf.h>
#include <kernel.h>
#include <mark.h>
#include <bufpool.h>
#include <proc.h>
#include <paging.h>
#include <lab1.h>

SYSCALL read_bs(char *dst, bsd_t bs_id, int page) {

	if (is_invalid_bs(bs_id) || is_invalid_page(page)) {
		return SYSERR;
	}
/* fetch page page from map map_id
     and write beginning at dst.
  */
   	void* phy_addr = BACKING_STORE_BASE + bs_id*BACKING_STORE_UNIT_SIZE + page*NBPG;
	bcopy(phy_addr, (void*)dst, NBPG);
	return OK;
}

