/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>
#include <lab1.h>
/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages) {
	
	STATWORD ps;
	if ((virtpage < 4096)||(source < 0)||(source > 7)||(npages < 1)||(npages > 256)) {
		kprintf("\n XMMAP: invalid parameters\n");
		return SYSERR;
	}
	disable(ps);
	if ((bsm_tab[source].bs_status == UNMAPPED) || (bsm_tab[source].bs_is_Private == PRIVATE_HEAP)) {
		restore(ps);
		return OK;
	}
	if (npages > bsm_tab[source].bs_npages) {
		restore(ps);
		return SYSERR;
	}
	if (bsm_map(currpid, virtpage, source, npages) == SYSERR) {
		restore(ps);
		return SYSERR;
	}
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage) {
	
	STATWORD ps;
	if ((virtpage < 4096)) {
		
		return SYSERR;
	}
	disable(ps);
	if (bsm_unmap(currpid, virtpage, 0) == SYSERR) {
		restore(ps);
		return SYSERR;
	}
	write2CR3(currpid);
	restore(ps);
	return OK;
}