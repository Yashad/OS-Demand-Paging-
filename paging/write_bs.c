#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <mark.h>
#include <bufpool.h>
#include <paging.h>
#include <lab1.h>

int write_bs(char *src, bsd_t bs_id, int page) {
	STATWORD ps;
	disable(ps);
	if (is_invalid_bs(bs_id) || is_invalid_page(page)) {
		restore(ps);
		return SYSERR;
	}

	char * phy_addr = BACKING_STORE_BASE + bs_id*BACKING_STORE_UNIT_SIZE + page*NBPG;
	bcopy((void*) src, phy_addr, NBPG);
	restore(ps);
	return OK;
}

