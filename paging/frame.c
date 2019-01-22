/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>
#include <lab1.h>

fr_map_t frm_tab[NFRAMES];

void frm_invalidate_TLB(int frm_num);


/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
	int i;
	for (i = 0; i < NFRAMES; i++) {
		frm_tab[i].fr_dirty = 0;
		frm_tab[i].pt_acc = 1;	
		frm_tab[i].fr_pid = -1;
		frm_tab[i].fr_status = UNMAPPED;
		frm_tab[i].fr_refcnt = 0;
		frm_tab[i].fr_type = -1;
		frm_tab[i].fr_vpno = -1;
	}
	return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
	int i;
	for (i = 0; i < NFRAMES; i++) {
		if (frm_tab[i].fr_status == UNMAPPED) {
			*avail=i;
			return OK;
		}
	}
	if (grpolicy() == SC) {
		return getFrame_SC();
	}
	*avail=-1;
	return OK;
}



int getFrame_SC() {

int flag=0;
	SCQueue *SC_pick=head_SC;
	SCQueue  *SC_prev=head_SC;
	while(frm_tab[SC_pick->frameID].pt_acc!=0||frm_tab[SC_pick->frameID].fr_type != FR_PAGE) {
		SC_prev=SC_pick;
		frm_tab[SC_pick->frameID].pt_acc=0;
		SC_pick=SC_pick->nextFrame;	
	}		
	SC_prev->nextFrame=SC_pick->nextFrame;
	head_SC=SC_pick->nextFrame;
	int swpframe=SC_pick->frameID;
	free_frm(swpframe);
	freemem(SC_pick, sizeof(SCQueue));
	return swpframe;
}

void free_page_frame(int i){
	
	int store, pageth;
	if ((bsm_lookup(frm_tab[i].fr_pid, frm_tab[i].fr_vpno * NBPG, &store, &pageth) == SYSERR) || (store == -1) || (pageth == -1)) {
					return SYSERR;
	}
	write_bs((i + FRAME0) * NBPG, store, pageth);
	pd_t *pd_entry = proctab[frm_tab[i].fr_pid].pdbr + (frm_tab[i].fr_vpno / 1024) * sizeof(pd_t);
	pt_t *pt_entry = (pd_entry->pd_base) * NBPG + (frm_tab[i].fr_vpno % 1024) * sizeof(pt_t);
	//clearing page table entry 
	pt_entry->pt_acc = 0;
	pt_entry->pt_avail = 0;
	pt_entry->pt_base = 0;
	pt_entry->pt_dirty = 0;
	pt_entry->pt_global = 0;
	pt_entry->pt_mbz = 0;
	pt_entry->pt_pcd = 0;
	pt_entry->pt_pres = 0;
	pt_entry->pt_pwt = 0;
	pt_entry->pt_write = 1;
	pt_entry->pt_user = 0;
	
	//clearing frame entry from Inverted frame table
	frm_tab[i].fr_dirty = CLEAN;
	frm_tab[i].fr_pid = -1;
	frm_tab[i].fr_refcnt = 0;
	frm_tab[i].fr_status = UNMAPPED;
	frm_tab[i].fr_type = -1;
	frm_tab[i].fr_vpno = -1;
	
	frm_tab[pd_entry->pd_base - FRAME0].fr_refcnt--;
	if (frm_tab[pd_entry->pd_base - FRAME0].fr_refcnt <= 0)
	{
			free_frm(pd_entry->pd_base - FRAME0);
		}
}

void free_TBL_frame(int i){
	
	int j;
		for (j = 0; j < NFRAMES; j++) {
			pt_t *pt_entry = (i + FRAME0) * NBPG + j * sizeof(pt_t);
			if (pt_entry->pt_pres == 1) {
				free_frm(pt_entry->pt_base - FRAME0);
			}
		}
		for (j = 0; j < NFRAMES; j++) {
			pd_t *pd_entry = proctab[frm_tab[i].fr_pid].pdbr + j * sizeof(pd_t);
			if (pd_entry->pd_base - FRAME0 == i) {
				pd_entry->pd_pres = 0;
				pd_entry->pd_write = 1;
				pd_entry->pd_user = 0;
				pd_entry->pd_pwt = 0;
				pd_entry->pd_pcd = 0;
				pd_entry->pd_acc = 0;
				pd_entry->pd_mbz = 0;
				pd_entry->pd_fmb = 0;
				pd_entry->pd_global = 0;
				pd_entry->pd_avail = 0;
				pd_entry->pd_base = 0;
			}
		}

	frm_tab[i].fr_dirty = CLEAN;
	frm_tab[i].fr_pid = -1;
	frm_tab[i].fr_refcnt = 0;
	frm_tab[i].fr_status = UNMAPPED;
	frm_tab[i].fr_type = -1;
	frm_tab[i].fr_vpno = -1;
}

void free_DIR_frame(int i){
	int j;
		for (j = 4; j < NFRAMES; j++) {
			pd_t *pd_entry = proctab[frm_tab[i].fr_pid].pdbr + j * sizeof(pd_t);
			if (pd_entry->pd_pres == 1) {
				free_frm(pd_entry->pd_base - FRAME0);
			}
		}
	frm_tab[i].fr_dirty = CLEAN;
	frm_tab[i].fr_pid = -1;
	frm_tab[i].fr_refcnt = 0;
	frm_tab[i].fr_status = UNMAPPED;
	frm_tab[i].fr_type = -1;
	frm_tab[i].fr_vpno = -1;
}
/*-------------------------------------------------------------------------
 * free_frm - free a frame
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i) {

	if (is_invalid_frame(i)) {
		return SYSERR;
	}
	int frame_type=frm_tab[i].fr_type;
	switch (frame_type) {
	case FR_PAGE:{
		free_page_frame(i);
		return OK;
	}
	case FR_TBL:{
		free_TBL_frame(i);
		return OK;
	}
	case FR_DIR:{
		free_DIR_frame(i);
		return OK;
	}
	}
	return SYSERR;
}

void insert_Frame_SC(int i) {
	if(head_SC->frameID==-1){
		head_SC->frameID = i;
	}
	SCQueue *SC_tmp = (SCQueue *) getmem(sizeof(SCQueue));
	SC_tmp->frameID = i;
	SC_tmp->nextFrame=NULL;
	tail_SC->nextFrame=SC_tmp;
	tail_SC=SC_tmp;
	tail_SC->nextFrame=head_SC;
	n_SCPages++;
}




