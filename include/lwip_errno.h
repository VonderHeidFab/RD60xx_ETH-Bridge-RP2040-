// filepath: include/lwip_errno.h
#ifndef LWIP_ERRNO_H
#define LWIP_ERRNO_H

// Minimal POSIX error codes for lwIP
#define ENOMEM        12
#define ENOBUFS       105
#define EWOULDBLOCK   11
#define EHOSTUNREACH  113
#define EINPROGRESS   115
#define EINVAL        22
#define EADDRINUSE    98
#define EALREADY      114
#define EISCONN       106
#define ENOTCONN      107
#define ECONNABORTED  103
#define ECONNRESET    104
#define EIO           5
#define ENXIO         6
#define EBADF         9
#define EOPNOTSUPP    95
#define ENFILE        23
#define EAGAIN        11
#define EBUSY         16
#define ENODEV        19
#define ENOSYS        38
#define ENOSPC        28
#define EAFNOSUPPORT  97
#define EFAULT        14
#define ENOPROTOOPT   92
#define EMSGSIZE      90

// Provide a dummy errno
extern int errno;

#endif // LWIP_ERRNO_H