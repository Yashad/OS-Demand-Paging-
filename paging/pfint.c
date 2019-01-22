/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>
#include <lab1.h>

/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{
	STATWORD ps;
	disable(ps);
	int store, pageth;
	/*Read CR2 register */
	unsigned long a = read_cr2();
	
	pd_t *page_dir = proctab[currpid].pdbr + sizeof(pd_t) * (a>> 22);
		/*check if the address is valid else kill the process which is trying to access illegal address */
	if ((bsm_lookup(currpid, a, &store, &pageth) == SYSERR) || (store == -1) || (pageth == -1)) {
		kill(currpid);
		restore(ps);
		return (SYSERR);
	}
	/*If no page table present, initialize one */
	if (page_dir->pd_pres == 0) {
		int new_frame = get_pageTable(currpid);
		if (new_frame == -1) {
			return SYSERR;
		}
		page_dir->pd_pres = 1;
		page_dir->pd_write = 1;
		page_dir->pd_base = new_frame + FRAME0;
		frm_tab[(unsigned int) page_dir / NBPG - FRAME0].fr_refcnt++;
	}
	int frame;
	get_frm(&frame);
	if (frame == -1) {
		kill(currpid);
		restore(ps);
		return SYSERR;
	}
	/*Initialize the frame */
	frm_tab[frame].fr_dirty = CLEAN;
	frm_tab[frame].fr_pid = currpid;
	frm_tab[frame].fr_refcnt = 1;
	frm_tab[frame].fr_status = MAPPED;
	frm_tab[frame].fr_type = FR_PAGE;
	frm_tab[frame].fr_vpno = (a/NBPG) & 0x000fffff;
	frm_tab[frame].pt_acc=1;

	read_bs((frame + FRAME0) * NBPG, store, pageth);	
	/*Update the SC queue */
	if (page_replace_policy == SC)
		insert_Frame_SC(frame);

	pt_t *pageTableEntry = (page_dir->pd_base) * NBPG + sizeof(pt_t) * ((a >> 12) & 0x000003ff);
	pageTableEntry->pt_pres = 1;
	pageTableEntry->pt_write = 1;
	pageTableEntry->pt_base = frame + FRAME0;
	frm_tab[(unsigned int) pageTableEntry / NBPG - FRAME0].fr_refcnt++;
	
	write2CR3(currpid);
	restore(ps);
	return OK;
}