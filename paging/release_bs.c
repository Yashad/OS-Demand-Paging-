#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>
#include <lab1.h>

SYSCALL release_bs(bsd_t bs_id) {

	if (is_invalid_bs(bs_id)) {
		return SYSERR;
	}
	free_bsm(bs_id);
	write2CR3(currpid);
	return OK;
}

