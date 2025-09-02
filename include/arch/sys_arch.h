// filepath: include/arch/sys_arch.h
#ifndef SYS_ARCH_H
#define SYS_ARCH_H

#include "lwip/opt.h"
#include <stdint.h>
#include "lwip_errno.h"

// Type definitions required by lwIP
typedef void* sys_mutex_t;
typedef void* sys_sem_t;
typedef void* sys_mbox_t;
typedef void* sys_thread_t;
typedef int sys_prot_t;

#define SYS_ARCH_TIMEOUT 0xFFFFFFFF

// Minimal stubs for required functions
sys_prot_t sys_arch_protect(void);
void sys_arch_unprotect(sys_prot_t p);

#endif // SYS_ARCH_H