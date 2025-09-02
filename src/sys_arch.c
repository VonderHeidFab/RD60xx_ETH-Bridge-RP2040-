// filepath: src/sys_arch.c
#include "lwip/sys.h"
#include "lwip/opt.h"
#include <stdint.h>

// Zeitgeber für lwIP (muss später korrekt implementiert werden)
u32_t sys_now(void) {
    return 0;
}

// Schutzmechanismen (Dummy)
sys_prot_t sys_arch_protect(void) { return 0; }
void sys_arch_unprotect(sys_prot_t pval) { (void)pval; }

// Mutex-Funktionen (Dummy)
void sys_mutex_lock(sys_mutex_t *mutex) { (void)mutex; }
void sys_mutex_unlock(sys_mutex_t *mutex) { (void)mutex; }

// Mailbox-Funktionen (Dummy)
int sys_mbox_valid(sys_mbox_t *mbox) { return 1; }
err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg) { return ERR_OK; }