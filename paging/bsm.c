/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>
#include <lab1.h>

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
bs_map_t bsm_tab[NBS];
SYSCALL init_bsm()
{
	/*initializing bsm table  */
	int i;
	for (i = 0; i < NBS; i++) 
	{
	bsm_tab[i].bs_is_Private = 0;
	bsm_tab[i].bs_npages = 0;
	bsm_tab[i].bs_pid = -1;
	bsm_tab[i].bs_refcnt = 0;
	bsm_tab[i].bs_sem = -1;
	bsm_tab[i].bs_status = UNMAPPED;
	bsm_tab[i].bs_vpno = -1;
	}
	return OK;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
	int i;
	for (i = 0; i < NBS; i++) {
		if (bsm_tab[i].bs_status == UNMAPPED)
		*avail =i;
		return OK;
	}
	*avail=-1;

	return OK;
}

/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
	if (bsm_tab[i].bs_refcnt != 0 || is_invalid_bs(i)) {
		return SYSERR;
	}
	bsm_tab[i].bs_is_Private = 0;
	bsm_tab[i].bs_npages = 0;
	bsm_tab[i].bs_pid = -1;
	bsm_tab[i].bs_refcnt = 0;
	bsm_tab[i].bs_sem = -1;
	bsm_tab[i].bs_status = UNMAPPED;
	bsm_tab[i].bs_vpno = -1;
	return OK;

}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
	unsigned int vpno =(vaddr/NBPG)&(0x000fffff);
	struct pentry *pptr = &proctab[pid];
	if((pptr->vhpno>vpno)||(pptr->vhpno+pptr->vhpnpages<vpno)||(bsm_tab[pptr->store].bs_status!=MAPPED)){
		*store = -1;
		*pageth = -1;
	 	return SYSERR;
	}
	*store=pptr->store;
	*pageth=vpno-pptr->vhpno;
	return OK;
}

/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
	if (vpno < NBPG || is_invalid_bs(source) || is_invalid_page(npages)) {
		return SYSERR;
	}
	proctab[pid].vhpno=vpno;
	proctab[pid].vhpnpages=npages;
	proctab[pid].store=source;
	if (bsm_tab[source].bs_status == UNMAPPED) {
		bsm_tab[source].bs_status = MAPPED;
		bsm_tab[source].bs_npages = npages;
		}
	bsm_tab[source].bs_refcnt++;
	return OK;
}

/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
	
	if (vpno < NBPG) {
		return SYSERR;
	}
	int store, pageth;
	unsigned int vaddress =vpno*NBPG;
	int status = bsm_lookup(pid, vaddress, &store, &pageth);
	if ((status ==-1) || (store == -1) || (pageth < 0)) {
		return SYSERR;
	}
	if (flag == PRIVATE_HEAP) {
		proctab[pid].vmemlist = NULL;	
	}
	else
	{
		int temp_vpno = vpno;
		while (temp_vpno < proctab[pid].vhpnpages + proctab[pid].vhpno) {
			unsigned int virt_Addr = temp_vpno * NBPG;
			pd_t *page_dir_ptr = proctab[pid].pdbr + (virt_Addr >> 22) * sizeof(pd_t);
			pt_t *page_table_ptr = (page_dir_ptr->pd_base) * NBPG + sizeof(pt_t) * ((virt_Addr >> 12)&0x000003ff);
			if (frm_tab[page_table_ptr->pt_base - FRAME0].fr_status == MAPPED)
				free_frm(page_table_ptr->pt_base - FRAME0);

			if (page_dir_ptr->pd_pres == 0)
				break;
			temp_vpno++;
		}
	}
		proctab[pid].store=-1;
		proctab[pid].vhpno=-1;
		proctab[pid].vhpnpages=0;
	    bsm_tab[store].bs_refcnt--;
	return OK;
}