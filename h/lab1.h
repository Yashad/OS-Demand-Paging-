#define NPG			256
#define NUM_BACKING_STORES		8
#define NFRAMES 	1024

extern int is_valid_bs(int bs);
extern int is_valid_page(int pgno);
extern int is_valid_frame(int frmno);
extern int write2CR3(int pid);
extern int get_pageDirectory(int pid);
extern int get_pageTable(int pid);

	
