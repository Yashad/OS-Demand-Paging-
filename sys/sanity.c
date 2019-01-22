#include<lab1.h>
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


int globalPageTable[4];

int is_invalid_bs(int bs){
  if(bs<0||bs>= NUM_BACKING_STORES)
	  return 1;
  return 0;
}
int is_invalid_page(int pgno){
	if(pgno<0||pgno>=NPG)
		return 1;
	return 0;
	
}
int is_invalid_frame(int frmno){
	if(frmno<0||frmno>=NFRAMES)
		return 1;
	return 0;

}

int write2CR3(int pid) {
	unsigned int pdbr = (proctab[pid].pdbr) / NBPG;

	if ((frm_tab[pdbr - FRAME0].fr_status != MAPPED) || (frm_tab[pdbr - FRAME0].fr_type != FR_DIR) || (frm_tab[pdbr - FRAME0].fr_pid != pid)) {
		return SYSERR;
	}
	write_cr3(proctab[pid].pdbr);
	return OK;
}

int init_GlobalPageTable() {
	int i, j, k;
	for (i = 0; i < 4; i++) {
		k = get_pageTable(NULLPROC);
		if (k == -1) {
			return SYSERR;
		}
		globalPageTable[i] = FRAME0 + k;

		for (j = 0; j < 1024; j++) {
			pt_t *pt = globalPageTable[i] * NBPG + j * sizeof(pt_t);
			pt->pt_pres = 1;
			pt->pt_write = 1;
			pt->pt_base = j + i * 1024;

			frm_tab[k].fr_refcnt++;
		}
	}
	return OK;
}

int get_pageTable(int pid) {

	int i;
	int frame;
 	get_frm(&frame);
	if (frame == -1) {
		return SYSERR;
	}
	frm_tab[frame].fr_refcnt = 0;
	frm_tab[frame].fr_type = FR_TBL;
	frm_tab[frame].fr_dirty = CLEAN;

	frm_tab[frame].fr_status = MAPPED;
	frm_tab[frame].fr_pid = pid;
	frm_tab[frame].fr_vpno = -1;
	for (i = 0; i < 1024; i++) {
		pt_t *pt = (FRAME0 + frame) * NBPG + i * sizeof(pt_t);
		pt->pt_dirty = CLEAN;
		pt->pt_mbz = 0;
		pt->pt_global = 0;
		pt->pt_avail = 0;
		pt->pt_base = 0;
		pt->pt_pres = 0;
		pt->pt_write = 1;
		pt->pt_user = 0;
		pt->pt_pwt = 0;
		pt->pt_pcd = 0;
		pt->pt_acc = 0;
	}
	return frame;
}

int get_pageDirectory(int pid) {
	int i;
	int frame;
 	get_frm(&frame);

	if (frame == -1) {
		return -1;
	}
	frm_tab[frame].fr_dirty = CLEAN;
	frm_tab[frame].fr_pid = pid;
	frm_tab[frame].fr_refcnt = 4;
	frm_tab[frame].fr_status = MAPPED;
	frm_tab[frame].fr_type = FR_DIR;
	frm_tab[frame].fr_vpno = -1;

	proctab[pid].pdbr = (FRAME0 + frame) * NBPG;
	for (i = 0; i < 1024; i++) {
		pd_t *pd_entry = proctab[pid].pdbr + (i * sizeof(pd_t));
		pd_entry->pd_pcd = 0;
		pd_entry->pd_acc = 0;
		pd_entry->pd_mbz = 0;
		pd_entry->pd_fmb = 0;
		pd_entry->pd_global = 0;
		pd_entry->pd_avail = 0;
		pd_entry->pd_base = 0;
		pd_entry->pd_pres = 0;
		pd_entry->pd_write = 1;
		pd_entry->pd_user = 0;
		pd_entry->pd_pwt = 0;

		if (i < 4) {
			pd_entry->pd_pres = 1;
			pd_entry->pd_write = 1;
			pd_entry->pd_base = globalPageTable[i];
		}
	}
	return frame;
}