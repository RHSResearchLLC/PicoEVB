#ifndef XDMA_CORE_H
#define XDMA_CORE_H

#include <linux/types.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/poll.h>
#include <linux/pci.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/workqueue.h>
#include <linux/aio.h>
#include <linux/splice.h>
#include <linux/version.h>
#include <linux/uio.h>

/* SECTION: Preprocessor switches */

/* Switch debug printing on/off */
#define XDMA_DEBUG 0

/* Switch to enable/disable SD Accel extensions */
#define SD_ACCEL 0

/*
 * optimistic back-to-back I/O chaining
 * this is not compatible with descriptors above 32-bit address range,
 * as the implementation depends on atomically writing 32-bits in host
 * memory to link descriptors
 */
#define CHAIN_MULTIPLE_TRANSFERS 0

/* Enable/disable XDMA FPGA status during operation */
#define XDMA_STATUS_DUMPS 0

/* Use this definition to poll several times between calls to schedule */
#define NUM_POLLS_PER_SCHED 100

/* for test purposes only, not in default IP! */
#define DESC_COUNTER 0

/* testing purposes; request interrupt on each descriptor */
#define FORCE_IR_DESC_COMPLETED 0

/* Switch to control module licence */
#define XDMA_GPL 1

/* SECTION: Preprocessor macros/constants */

#define DRV_NAME "xdma"
#define XDMA_MINOR_BASE (0)
#define XDMA_MINOR_COUNT (255)

#define XDMA_KNOWN_REVISION (0x01)
#define XDMA_BAR_NUM (6)

/* maximum amount of register space to map */
#define XDMA_BAR_SIZE (0x8000UL)

#define XDMA_CHANNEL_NUM_MAX (4)
/*
 * interrupts per engine, rad2_vul.sv:237
 * .REG_IRQ_OUT	(reg_irq_from_ch[(channel*2) +: 2]),
 */
#define XDMA_ENG_IRQ_NUM (1)
#define MAX_EXTRA_ADJ (15)
#define RX_STATUS_EOP (1)

/* Target internal components on XDMA control BAR */
#define XDMA_OFS_INT_CTRL	(0x2000UL)
#define XDMA_OFS_CONFIG		(0x3000UL)

/* maximum number of bytes per transfer request */
#define XDMA_TRANSFER_MAX_BYTES (2048 * 4096)

/* maximum size of a single DMA transfer descriptor */
#define XDMA_DESC_MAX_BYTES ((1 << 18) - 1)

/* bits of the SG DMA control register */
#define XDMA_CTRL_RUN_STOP			(1UL << 0)
#define XDMA_CTRL_IE_DESC_STOPPED		(1UL << 1)
#define XDMA_CTRL_IE_DESC_COMPLETED		(1UL << 2)
#define XDMA_CTRL_IE_DESC_ALIGN_MISMATCH	(1UL << 3)
#define XDMA_CTRL_IE_MAGIC_STOPPED		(1UL << 4)
#define XDMA_CTRL_IE_IDLE_STOPPED		(1UL << 6)
#define XDMA_CTRL_IE_READ_ERROR			(0x1FUL << 9)
#define XDMA_CTRL_IE_DESC_ERROR			(0x1FUL << 19)
#define XDMA_CTRL_NON_INCR_ADDR			(1UL << 25)
#define XDMA_CTRL_POLL_MODE_WB			(1UL << 26)

/* bits of the SG DMA status register */
#define XDMA_STAT_BUSY			(1UL << 0)
#define XDMA_STAT_DESC_STOPPED		(1UL << 1)
#define XDMA_STAT_DESC_COMPLETED	(1UL << 2)
#define XDMA_STAT_ALIGN_MISMATCH	(1UL << 3)
#define XDMA_STAT_MAGIC_STOPPED		(1UL << 4)
#define XDMA_STAT_FETCH_STOPPED		(1UL << 5)
#define XDMA_STAT_IDLE_STOPPED		(1UL << 6)
#define XDMA_STAT_READ_ERROR		(0x1FUL << 9)
#define XDMA_STAT_DESC_ERROR		(0x1FUL << 19)

/* bits of the SGDMA descriptor control field */
#define XDMA_DESC_STOPPED	(1UL << 0)
#define XDMA_DESC_COMPLETED	(1UL << 1)
#define XDMA_DESC_EOP		(1UL << 4)

#define XDMA_PERF_RUN	(1UL << 0)
#define XDMA_PERF_CLEAR	(1UL << 1)
#define XDMA_PERF_AUTO	(1UL << 2)

#define MAGIC_ENGINE	0xEEEEEEEEUL
#define MAGIC_DEVICE	0xDDDDDDDDUL
#define MAGIC_CHAR	0xCCCCCCCCUL
#define MAGIC_BITSTREAM 0xBBBBBBBBUL

/* upper 16-bits of engine identifier register */
#define XDMA_ID_H2C 0x1fc0U
#define XDMA_ID_C2H 0x1fc1U

/* Specifies buffer size used for C2H AXI-ST mode */
#define RX_BUF_BLOCK 4096
#define RX_BUF_PAGES 256
#define RX_BUF_SIZE (RX_BUF_PAGES * RX_BUF_BLOCK)
#define RX_RESULT_BUF_SIZE (RX_BUF_PAGES * sizeof(struct xdma_result))

#define LS_BYTE_MASK 0x000000FFUL

#define BLOCK_ID_MASK 0xFFF00000UL
#define BLOCK_ID_HEAD 0x1FC00000UL

#define IRQ_BLOCK_ID 0x1fc20000UL
#define CONFIG_BLOCK_ID 0x1fc30000UL

#define WB_COUNT_MASK 0x00ffffffUL
#define WB_ERR_MASK (1UL << 31)
#define POLL_TIMEOUT_SECONDS 10

#define MAX_USER_IRQ 16

#define MAX_DESC_BUS_ADDR (0xffffffffULL)

#define DESC_MAGIC 0xAD4B0000UL

#define C2H_WB 0x52B4UL

#define MAX_NUM_ENGINES (XDMA_CHANNEL_NUM_MAX * 2)
#define H2C_CHANNEL_OFFSET 0x1000
#define SGDMA_OFFSET_FROM_CHANNEL 0x4000
#define CHANNEL_SPACING 0x100
#define TARGET_SPACING 0x1000

#define BYPASS_MODE_SPACING 0x0100

/* obtain the 32 most significant (high) bits of a 32-bit or 64-bit address */
#define PCI_DMA_H(addr) ((addr >> 16) >> 16)
/* obtain the 32 least significant (low) bits of a 32-bit or 64-bit address */
#define PCI_DMA_L(addr) (addr & 0xffffffffUL)

#ifndef VM_RESERVED
	#define VMEM_FLAGS (VM_IO | VM_DONTEXPAND | VM_DONTDUMP)
#else
	#define VMEM_FLAGS (VM_IO | VM_RESERVED)
#endif

/* disable debugging */
#if (XDMA_DEBUG == 0)
	#define dbg_desc(...)
	#define dbg_io(...)
	#define dbg_fops(...)
	#define dbg_perf(fmt, ...)
	#define dbg_sg(...)
	#define dbg_tfr(...)
	#define dbg_irq(...)
	#define dbg_init(...)

#else
	/* descriptor, ioread/write, scatter-gather, transfer debugging */
	#define dbg_desc(fmt, ...) pr_debug("%s():" fmt, \
		__func__, ##__VA_ARGS__)

	#define dbg_io(fmt, ...) pr_debug("%s():" fmt, \
		__func__, ##__VA_ARGS__)

	#define dbg_fops(fmt, ...) pr_debug("%s():" fmt, \
		__func__, ##__VA_ARGS__)

	#define dbg_perf(fmt, ...) pr_debug("%s():" fmt, \
		__func__, ##__VA_ARGS__)

	#define dbg_sg(fmt, ...) pr_debug("%s():" fmt, \
		__func__, ##__VA_ARGS__)

	#define dbg_irq(fmt, ...) pr_debug("%s():" fmt, \
		__func__, ##__VA_ARGS__)

	#define dbg_init(fmt, ...) pr_debug("%s():" fmt, \
		__func__, ##__VA_ARGS__)

	#define dbg_tfr(fmt, ...) pr_debug("%s(): %s%c: " fmt, \
		__func__, \
		engine ? (engine->number_in_channel ? "C2H" : "H2C") \
		: "---", engine ? '0' + engine->channel : '-', ##__VA_ARGS__)
#endif

#if DESC_COUNTER
	#define	INSERT_DESC_COUNT(count) ((count & 0xf) << 12)
#else
	#define	INSERT_DESC_COUNT(count) (0)
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0)
	#define AIO_COMPLETE(iocb, done) iocb->ki_complete(iocb, done, 0)
#else
	#define AIO_COMPLETE(iocb, done) aio_complete(iocb, done, 0)
#endif

#if SD_ACCEL
	#define NODE_PREFIX "xcldma"
#else
	#define NODE_PREFIX DRV_NAME
#endif 

/* SECTION: Enum definitions */

enum chardev_type {
	CHAR_USER,
	CHAR_CTRL,
	CHAR_EVENTS,
	CHAR_XDMA_H2C,
	CHAR_XDMA_C2H,
	CHAR_BYPASS_H2C,
	CHAR_BYPASS_C2H,
	CHAR_BYPASS
};

enum transfer_state {
	TRANSFER_STATE_NEW = 0,
	TRANSFER_STATE_SUBMITTED,
	TRANSFER_STATE_COMPLETED,
	TRANSFER_STATE_FAILED
};

enum shutdown_state {
	ENGINE_SHUTDOWN_NONE = 0,	/* No shutdown in progress */
	ENGINE_SHUTDOWN_REQUEST = 1,	/* engine requested to shutdown */
	ENGINE_SHUTDOWN_IDLE = 2	/* engine has shutdown and is idle */
};

enum dev_capabilities {
	CAP_64BIT_DMA = 2,
	CAP_64BIT_DESC = 4,
	CAP_ENGINE_WRITE = 8,
	CAP_ENGINE_READ = 16
};

/* SECTION: Structure definitions */

struct config_regs {
	u32 identifier;
	u32 reserved_1[4];
	u32 msi_enable;
};

/**
 * SG DMA Controller status and control registers
 *
 * These registers make the control interface for DMA transfers.
 *
 * It sits in End Point (FPGA) memory BAR[0] for 32-bit or BAR[0:1] for 64-bit.
 * It references the first descriptor which exists in Root Complex (PC) memory.
 *
 * @note The registers must be accessed using 32-bit (PCI DWORD) read/writes,
 * and their values are in little-endian byte ordering.
 */
struct engine_regs {
	u32 identifier;
	u32 control;
	u32 control_w1s;
	u32 control_w1c;
	u32 reserved_1[12];	/* padding */

	u32 status;
	u32 status_rc;
	u32 completed_desc_count;
	u32 alignments;
	u32 reserved_2[14];	/* padding */

	u32 poll_mode_wb_lo;
	u32 poll_mode_wb_hi;
	u32 interrupt_enable_mask;
	u32 interrupt_enable_mask_w1s;
	u32 interrupt_enable_mask_w1c;
	u32 reserved_3[9];	/* padding */

	u32 perf_ctrl;
	u32 perf_cyc_lo;
	u32 perf_cyc_hi;
	u32 perf_dat_lo;
	u32 perf_dat_hi;
	u32 perf_pnd_lo;
	u32 perf_pnd_hi;
} __packed;

struct engine_sgdma_regs {
	u32 identifier;
	u32 reserved_1[31];	/* padding */

	/* bus address to first descriptor in Root Complex Memory */
	u32 first_desc_lo;
	u32 first_desc_hi;
	/* number of adjacent descriptors at first_desc */
	u32 first_desc_adjacent;
	u32 credits;
} __packed;

struct msix_vec_table_entry {
	u32 msi_vec_addr_lo;
	u32 msi_vec_addr_hi;
	u32 msi_vec_data_lo;
	u32 msi_vec_data_hi;
} __packed;

struct msix_vec_table {
	struct msix_vec_table_entry entry_list[32];
} __packed;

/* Performance counter for AXI Streaming */
struct performance_regs {
	u32 identifier;
	u32 control;
	u32 status;
	u32 period_low;		/* period in 8 ns units (low 32-bit) */
	u32 period_high;	/* period (high 32-bit) */
	u32 performance_low;	/* perf count in 8-byte units (low 32-bit) */
	u32 performance_high;	/* perf count (high 32-bit) */
	u32 wait_low;		/* wait count in 8-byte units (low 32-bit) */
	u32 wait_high;		/* wait (high 32-bit) */
} __packed;

struct interrupt_regs {
	u32 identifier;
	u32 user_int_enable;
	u32 user_int_enable_w1s;
	u32 user_int_enable_w1c;
	u32 channel_int_enable;
	u32 channel_int_enable_w1s;
	u32 channel_int_enable_w1c;
	u32 reserved_1[9];	/* padding */

	u32 user_int_request;
	u32 channel_int_request;
	u32 user_int_pending;
	u32 channel_int_pending;
	u32 reserved_2[12];	/* padding */

	u32 user_msi_vector[8];
	u32 channel_msi_vector[8];
} __packed;

struct sgdma_common_regs {
	u32 padding[9];
	u32 credit_feature_enable;
} __packed;

/**
 * Descriptor for a single contiguous memory block transfer.
 *
 * Multiple descriptors are linked by means of the next pointer. An additional
 * extra adjacent number gives the amount of extra contiguous descriptors.
 *
 * The descriptors are in root complex memory, and the bytes in the 32-bit
 * words must be in little-endian byte ordering.
 */
struct xdma_desc {
	u32 control;
	u32 bytes;		/* transfer length in bytes */
	u32 src_addr_lo;	/* source address (low 32-bit) */
	u32 src_addr_hi;	/* source address (high 32-bit) */
	u32 dst_addr_lo;	/* destination address (low 32-bit) */
	u32 dst_addr_hi;	/* destination address (high 32-bit) */
	/*
	 * next descriptor in the single-linked list of descriptors;
	 * this is the PCIe (bus) address of the next descriptor in the
	 * root complex memory
	 */
	u32 next_lo;		/* next desc address (low 32-bit) */
	u32 next_hi;		/* next desc address (high 32-bit) */
} __packed;

/* 32 bytes (four 32-bit words) or 64 bytes (eight 32-bit words) */
struct xdma_result {
	u32 status;
	u32 length;
	u32 reserved_1[6];	/* padding */
} __packed;

/* Structure for polled mode descriptor writeback */
struct xdma_poll_wb {
	u32 completed_desc_count;
	u32 reserved_1[7];
} __packed;

/* Describes a (SG DMA) single transfer for the engine */
struct xdma_transfer {
	struct list_head entry;		/* queue of non-completed transfers */
	struct xdma_desc *desc_virt;	/* virt addr of the 1st descriptor */
	dma_addr_t desc_bus;		/* bus addr of the first descriptor */
	int desc_adjacent;		/* adjacent descriptors at desc_bus */
	int desc_num;			/* number of descriptors in transfer */
	int dir_to_dev;			/* specify transfer direction */
	wait_queue_head_t wq;		/* wait queue for transfer completion */
	struct kiocb *iocb;		/* completion buffer for async I/O */
	int sgl_nents;			/* adjacent descriptors at desc_virt */
	struct sg_mapping_t *sgm;	/* user space scatter-gather mapper */
	int userspace;			/* flag if user space pages are got */
	enum transfer_state state;	/* state of the transfer */
	int cyclic;			/* flag if transfer is cyclic */
	int last_in_request;		/* flag if last within request */
	ssize_t size_of_request;	/* request size */
};

struct xdma_engine {
	unsigned long magic;	/* structure ID for sanity checks */
	struct xdma_dev *lro;	/* parent device */
	char *name;		/* name of this engine */
	int version;		/* version of this engine */
	dev_t cdevno;		/* character device major:minor */
	struct cdev cdev;	/* character device (embedded struct) */

	/* HW register address offsets */
	struct engine_regs *regs;		/* Control reg BAR offset */
	struct engine_sgdma_regs *sgdma_regs;	/* SGDAM reg BAR offset */
	u32 bypass_offset;			/* Bypass mode BAR offset */

	/* Engine state, configuration and flags */
	enum shutdown_state shutdown;	/* engine shutdown mode */
	int running;		/* flag if the driver started engine */
	int streaming;		/* flag if AXI-ST engine */
	int non_incr_addr;	/* flag if non-incremental addressing used */
	int dir_to_dev;		/* direction of this engine */
	int addr_align;		/* source/dest alignment in bytes */
	int len_granularity;	/* transfer length multiple */
	int addr_bits;		/* HW datapath address width */
	int channel;		/* engine indices */
	int number_in_channel;	/* engine indices */
	int max_extra_adj;	/* descriptor prefetch capability */
	int desc_dequeued;	/* num descriptors of completed transfers */
	u32 status;		/* last known status of device */
	u32 interrupt_enable_mask_value;/* only used for MSIX mode to store per-engine interrupt mask value */

	/* Transfer list management */
	struct list_head transfer_list;	/* queue of transfers */
	struct sg_mapping_t *sgm;	/* user space scatter gather mapper */
	int rx_tail;	/* follows the HW */
	int rx_head;	/* where the SW reads from */
	int rx_overrun;	/* flag if overrun occured */

	/* Members applicable to AXI-ST C2H (cyclic) transfers */
	void *rx_buffer;	/* Kernel buffer for transfers */
	struct xdma_transfer *rx_transfer_cyclic;	/* Transfer list */
	u8 *rx_result_buffer_virt;		/* virt addr for transfer */
	dma_addr_t rx_result_buffer_bus;	/* bus addr for transfer */

	/* Members associated with polled mode support */
	u8 *poll_mode_addr_virt;	/* virt addr for descriptor writeback */
	dma_addr_t poll_mode_bus;	/* bus addr for descriptor writeback */

	/* Members associated with interrupt mode support */
	wait_queue_head_t shutdown_wq;	/* wait queue for shutdown sync */
	spinlock_t lock;		/* protects concurrent access */
	int prev_cpu;			/* remember CPU# of (last) locker */
	int msix_irq_line;		/* MSI-X vector for this engine */
	u32 irq_bitmask;		/* IRQ bit mask for this engine */
	struct work_struct work;	/* Work queue for interrupt handling */

	/* Members associated with performance test support */
	struct xdma_performance_ioctl *xdma_perf;	/* perf test control */
	wait_queue_head_t xdma_perf_wq;			/* Perf test sync */
	u8 eop_found; /* used only for cyclic(rx:c2h) */
	u32 user_buffer_index;
};

struct xdma_ocl_clockwiz {
	/* target frequency */
	unsigned ocl;
	/* clockout divider */
	unsigned divide;
	/* clockout divider fractional part */
	unsigned divide_frac;
};

struct xdma_bitstream_container {
	/* MAGIC_BITSTREAM == 0xBBBBBBBBUL */
	unsigned long magic;
	char *clear_bitstream;
	u32 clear_bitstream_length;
};

/*
 * XDMA character device specific book keeping. Each bus has a character device,
 * the control bus has no XDMA engines attached to it.
 */
struct xdma_char {
	unsigned long magic;		/* structure ID for sanity checks */
	struct xdma_dev *lro;		/* parent device */
	dev_t cdevno;			/* character device major:minor */
	struct cdev cdev;		/* character device embedded struct */
	int bar;			/* PCIe BAR for HW access, if needed */
	struct xdma_engine *engine;	/* engine instance, if needed */
	struct xdma_irq *user_irq;	/* IRQ value, if needed */
	struct device *sys_device;	/* sysfs device */
};

struct xdma_irq {
	struct xdma_dev *lro;		/* parent device */
	u8 events_irq;			/* accumulated IRQs */
	spinlock_t events_lock;		/* lock to safely update events_irq */
	wait_queue_head_t events_wq;	/* wait queue to sync waiting threads */
};

/* XDMA PCIe device specific book-keeping */
struct xdma_dev {
	unsigned long magic;		/* structure ID for sanity checks */
	struct pci_dev *pci_dev;	/* pci device struct from probe() */
	int major;		/* major number */
	int instance;		/* instance number */
	dev_t cdevno_base;	/* character device major:minor base */

	/* character device structures */
	struct xdma_char *user_char_dev;
	struct xdma_char *ctrl_char_dev;
	struct xdma_char *bypass_char_dev[XDMA_CHANNEL_NUM_MAX][2];
	struct xdma_char *bypass_char_dev_base;
	struct xdma_char *sgdma_char_dev[XDMA_CHANNEL_NUM_MAX][2];
	struct xdma_char *events_char_dev[16];

	/* PCIe BAR management */
	void *__iomem bar[XDMA_BAR_NUM];	/* addresses for mapped BARs */
	int user_bar_idx;	/* BAR index of user logic */
	int config_bar_idx;	/* BAR index of XDMA config logic */
	int bypass_bar_idx;	/* BAR index of XDMA bypass logic */
	int regions_in_use;	/* flag if dev was in use during probe() */
	int got_regions;	/* flag if probe() obtained the regions */

	/* Interrupt management */
	int irq_count;		/* interrupt counter */
	int irq_line;		/* flag if irq allocated successfully */
	int msi_enabled;	/* flag if msi was enabled for the device */
	int msix_enabled;	/* flag if msi-x was enabled for the device */
	int irq_user_count;	/* user interrupt count */     
	struct msix_entry entry[32];	/* msi-x vector/entry table */
	struct xdma_irq user_irq[16];	/* user IRQ management */

	/* XDMA engine management */
	int engines_num;	/* Total engine count */
	struct xdma_engine *engine[XDMA_CHANNEL_NUM_MAX][2];	/* instances */

	/* SD_Accel specific */
	enum dev_capabilities capabilities;
	struct xdma_bitstream_container stash;
	int axi_gate_frozen;
	unsigned long user_char_dev_opened;
	int mcap_base;
	u64 feature_id;
};

void write_register(u32 value, void *iomem);
u32 read_register(void *iomem);

#if SD_ACCEL
long char_ctrl_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
long load_boot_firmware(struct xdma_dev *lro);
long bitstream_clear_icap(struct xdma_dev *lro);
int interrupts_enable(struct xdma_dev *lro, int interrupts_offset, u32 mask);
void engine_reinit(const struct xdma_engine *engine);
long bitstream_ioctl(struct xdma_char *lro_char, unsigned int cmd, const void __user *arg);
void freezeAXIGate(struct xdma_dev *lro);
void freeAXIGate(struct xdma_dev *lro);
long reset_device_if_running(struct xdma_dev *lro);
u64 featureid(struct xdma_dev *lro);
#endif

#endif /* XDMA_CORE_H */
