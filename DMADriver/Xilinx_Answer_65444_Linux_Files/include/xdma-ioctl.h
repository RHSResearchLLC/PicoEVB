#ifndef _XDMA_IOCALLS_POSIX_H_
#define _XDMA_IOCALLS_POSIX_H_

#ifndef _WINDOWS
// TODO: Windows build support
#include <linux/ioctl.h>
#endif

/* Use 'x' as magic number */
#define XDMA_IOC_MAGIC	'x'
/* XL OpenCL X->58(ASCII), L->6C(ASCII), O->0 C->C L->6C(ASCII); */
#define XDMA_XCL_MAGIC 0X586C0C6C

#define IOCTL_XDMA_PERF_V1 (1)
#define XDMA_ADDRMODE_MEMORY (0)
#define XDMA_ADDRMODE_FIXED (1)

/*
 * S means "Set" through a ptr,
 * T means "Tell" directly with the argument value
 * G means "Get": reply by setting through a pointer
 * Q means "Query": response is on the return value
 * X means "eXchange": switch G and S atomically
 * H means "sHift": switch T and Q atomically
 *
 * _IO(type,nr)		    no arguments
 * _IOR(type,nr,datatype)   read data from driver
 * _IOW(type,nr.datatype)   write data to driver
 * _IORW(type,nr,datatype)  read/write data
 *
 * _IOC_DIR(nr)		    returns direction
 * _IOC_TYPE(nr)	    returns magic
 * _IOC_NR(nr)		    returns number
 * _IOC_SIZE(nr)	    returns size
 */

enum XDMA_IOC_TYPES {
	XDMA_IOC_NOP,
	XDMA_IOC_INFO,
	XDMA_IOC_ICAP_DOWNLOAD,
	XDMA_IOC_MCAP_DOWNLOAD,
	XDMA_IOC_HOT_RESET,
	XDMA_IOC_OCL_RESET,
	XDMA_IOC_OCL_FREQ_SCALING,
	XDMA_IOC_MAX
};

struct xdma_ioc_base {
	unsigned int magic;
	unsigned int command;
};

struct xdma_ioc_info {
        struct xdma_ioc_base base;
        unsigned short       vendor;
        unsigned short       device;
        unsigned short       subsystem_vendor;
        unsigned short       subsystem_device;
        unsigned             dma_engine_version;
        unsigned             driver_version;
        unsigned long long   feature_id;
        unsigned             ocl_frequency;
};

struct xdma_ioc_bitstream {
	struct xdma_ioc_base base;
	struct xclBin *xclbin;
};

struct xdma_ioc_freqscaling {
	struct xdma_ioc_base base;
        unsigned ocl_target_freq;
};


struct xdma_performance_ioctl
{
	/* IOCTL_XDMA_IOCTL_Vx */
	uint32_t version;
	uint32_t transfer_size;
	/* measurement */
	uint32_t stopped;
	uint32_t iterations;
	uint64_t clock_cycle_count;
	uint64_t data_cycle_count;
	uint64_t pending_count;
};

/* IOCTL codes */
#define XDMA_IOCINFO		_IOWR(XDMA_IOC_MAGIC, XDMA_IOC_INFO,			struct xdma_ioc_info)
#define XDMA_IOCICAPDOWNLOAD	_IOW(XDMA_IOC_MAGIC, XDMA_IOC_ICAP_DOWNLOAD,		struct xdma_ioc_bitstream)
#define XDMA_IOCMCAPDOWNLOAD	_IOW(XDMA_IOC_MAGIC, XDMA_IOC_MCAP_DOWNLOAD,		struct xdma_ioc_bitstream)
#define XDMA_IOCHOTRESET	_IOW(XDMA_IOC_MAGIC, XDMA_IOC_HOT_RESET,		struct xdma_ioc_base)
#define XDMA_IOCOCLRESET	_IOW(XDMA_IOC_MAGIC, XDMA_IOC_OCL_RESET,		struct xdma_ioc_base)
#define XDMA_IOCFREQSCALING	_IOWR(XDMA_IOC_MAGIC, XDMA_IOC_OCL_FREQ_SCALING,	struct xdma_ioc_freqscaling)
// Legacy IOCTL NAME
#define XDMA_IOCRESET		(XDMA_IOCHOTRESET)

#define IOCTL_XDMA_PERF_START	_IOW('q', 1, struct xdma_performance_ioctl *)
#define IOCTL_XDMA_PERF_STOP	_IOW('q', 2, struct xdma_performance_ioctl *)
#define IOCTL_XDMA_PERF_GET	_IOR('q', 3, struct xdma_performance_ioctl *)
#define IOCTL_XDMA_ADDRMODE_SET	_IOW('q', 4, int)
#define IOCTL_XDMA_ADDRMODE_GET	_IOR('q', 5, int)
#define IOCTL_XDMA_ALIGN_GET	_IOR('q', 6, int)

#endif /* _XDMA_IOCALLS_POSIX_H_ */

