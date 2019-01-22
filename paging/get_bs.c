#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>
#include <lab1.h>

int get_bs(bsd_t bs_id, unsigned int npages) {

	STATWORD ps;
	disable(ps);
	if (is_invalid_bs(bs_id) || is_invalid_page(npages)||((bsm_tab[bs_id].bs_is_Private==PRIVATE_HEAP) && (bsm_tab[bs_id].bs_status==MAPPED))) {
	//	kprintf("GET_BS: Invalid No. of pages or store is passed\n");
		restore (ps);
		return SYSERR;
	}
	if (bsm_tab[bs_id].bs_status == UNMAPPED) {
		if(bsm_map(currpid,4096,bs_id,npages)==SYSERR)
 		{
			restore(ps);
			return SYSERR;
		}
		restore(ps);
		return npages;
	 }else{
		restore(ps);
		return bsm_tab[bs_id].bs_npages;
	}
}


