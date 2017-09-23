/*
 * Driver for XDMA for Xilinx XDMA IP core
 *
 * Copyright (C) 2007-2015 Sidebranch
 * Copyright (C) 2015 Xilinx, Inc.
 *
 * Leon Woestenberg <leon@sidebranch.com>
 * Richard Tobin <richard.tobin@xilinx.com>
 * Sonal Santan <sonal.santan@xilinx.com>
 */

/* SECTION: Header includes */

#include <linux/ioctl.h>
#include <linux/types.h>
/* include early, to verify it depends only on the headers above */
#include "xdma-ioctl.h"
#include "xdma-core.h"
#include "xdma-sgm.h"
#include "xbar_sys_parameters.h"
#include "version.h"

/* SECTION: Module licensing */

#if XDMA_GPL
MODULE_LICENSE("GPL v2");
#else
MODULE_LICENSE("Copyright (C) 2009-2017 Sidebranch and Xilinx, Inc.");
#endif
MODULE_AUTHOR(	"Leon Woestenberg <leon@sidebranch.com>,"
		"Sonal Santan <sonal.santan@xilinx.com>,"
		"Richard Tobin <richard.tobin@xilinx.com>");

MODULE_VERSION(DRV_MODULE_VERSION);

/* SECTION: Module parameters */

static unsigned int major;
module_param(major, uint, 0644);
MODULE_PARM_DESC(major, "Device major number, default is 0 (dynamic value)");

static unsigned int poll_mode;
module_param(poll_mode, uint, 0644);
MODULE_PARM_DESC(poll_mode, "Set 1 for hw polling, default is 0 (interrupts)");

static unsigned int enable_credit_mp;
module_param(enable_credit_mp, uint, 0644);
MODULE_PARM_DESC(enable_credit_mp, "Set 1 to enable creidt feature, default is 0 (no credit control)");

#if SD_ACCEL
/* SD_Accel Specific */
static bool load_firmware = true;
module_param(load_firmware, bool, 0644);
MODULE_PARM_DESC(load_firmware, "For UltraScale boards load xclbin firmware file from /lib/firmware/xilinx directory (default: true)");
#endif

/* SECTION: Module global variables */

struct class *g_xdma_class;	/* sys filesystem */

static const struct pci_device_id pci_ids[] = {
	{ PCI_DEVICE(0x10ee, 0x9011), },
	{ PCI_DEVICE(0x10ee, 0x9012), },
	{ PCI_DEVICE(0x10ee, 0x9014), },
	{ PCI_DEVICE(0x10ee, 0x9018), },
	{ PCI_DEVICE(0x10ee, 0x901F), },
	{ PCI_DEVICE(0x10ee, 0x9021), },
	{ PCI_DEVICE(0x10ee, 0x9022), },
	{ PCI_DEVICE(0x10ee, 0x9024), },
	{ PCI_DEVICE(0x10ee, 0x9028), },
	{ PCI_DEVICE(0x10ee, 0x902F), },
	{ PCI_DEVICE(0x10ee, 0x9031), },
	{ PCI_DEVICE(0x10ee, 0x9032), },
	{ PCI_DEVICE(0x10ee, 0x9034), },
	{ PCI_DEVICE(0x10ee, 0x9038), },
	{ PCI_DEVICE(0x10ee, 0x903F), },
	{ PCI_DEVICE(0x10ee, 0x8011), },
	{ PCI_DEVICE(0x10ee, 0x8012), },
	{ PCI_DEVICE(0x10ee, 0x8014), },
	{ PCI_DEVICE(0x10ee, 0x8018), },
	{ PCI_DEVICE(0x10ee, 0x8021), },
	{ PCI_DEVICE(0x10ee, 0x8022), },
	{ PCI_DEVICE(0x10ee, 0x8024), },
	{ PCI_DEVICE(0x10ee, 0x8028), },
	{ PCI_DEVICE(0x10ee, 0x8031), },
	{ PCI_DEVICE(0x10ee, 0x8032), },
	{ PCI_DEVICE(0x10ee, 0x8034), },
	{ PCI_DEVICE(0x10ee, 0x8038), },
	{ PCI_DEVICE(0x10ee, 0x7011), },
	{ PCI_DEVICE(0x10ee, 0x7012), },
	{ PCI_DEVICE(0x10ee, 0x7014), },
	{ PCI_DEVICE(0x10ee, 0x7018), },
	{ PCI_DEVICE(0x10ee, 0x7021), },
	{ PCI_DEVICE(0x10ee, 0x7022), },
	{ PCI_DEVICE(0x10ee, 0x7024), },
	{ PCI_DEVICE(0x10ee, 0x7028), },
	{ PCI_DEVICE(0x10ee, 0x7031), },
	{ PCI_DEVICE(0x10ee, 0x7032), },
	{ PCI_DEVICE(0x10ee, 0x7034), },
	{ PCI_DEVICE(0x10ee, 0x7038), },
	{0,}
};
MODULE_DEVICE_TABLE(pci, pci_ids);

static const char * const devnode_names[] = {
	NODE_PREFIX "%d_user",
	NODE_PREFIX "%d_control",
	NODE_PREFIX "%d_events_%d",
	NODE_PREFIX "%d_h2c_%d",
	NODE_PREFIX "%d_c2h_%d",
	NODE_PREFIX "%d_bypass_h2c_%d",
	NODE_PREFIX "%d_bypass_c2h_%d",
	NODE_PREFIX "%d_bypass"
};

/* SECTION: Function prototypes */

/* PCIe HW register access */
static inline u32 build_u32(u32 hi, u32 lo);
static inline u64 build_u64(u64 hi, u64 lo);
static u64 find_feature_id(const struct xdma_dev *lro);
static void interrupt_status(struct xdma_dev *lro);
static void channel_interrupts_enable(struct xdma_dev *lro, u32 mask);
static void channel_interrupts_disable(struct xdma_dev *lro, u32 mask);
static void user_interrupts_enable(struct xdma_dev *lro, u32 mask);
static void user_interrupts_disable(struct xdma_dev *lro, u32 mask);
static u32 read_interrupts(struct xdma_dev *lro);
static void *rvmalloc(unsigned long size);
static void rvfree(void *mem, unsigned long size);
static int xdma_performance_submit(struct xdma_dev *lro,
		struct xdma_engine *engine);
static void engine_reg_dump(struct xdma_engine *engine);
static u32 engine_status_read(struct xdma_engine *engine, int clear);
static void xdma_engine_stop(struct xdma_engine *engine);
static struct xdma_transfer *engine_cyclic_stop(struct xdma_engine *engine);
static inline void dump_engine_status(struct xdma_engine *engine);
static void engine_start_mode_config(struct xdma_engine *engine);
static struct xdma_transfer *engine_start(struct xdma_engine *engine);
static int engine_initialize(struct xdma_dev *lro, int interrupts_offset);
static int engine_version(struct xdma_dev *lro, int engine_offset);
static void engine_service_shutdown(struct xdma_engine *engine);
static void engine_transfer_dequeue(struct xdma_engine *engine);
static int engine_ring_process(struct xdma_engine *engine);
static int engine_service_cyclic_polled(struct xdma_engine *engine);
static int engine_service_cyclic_interrupt(struct xdma_engine *engine);
static int engine_service_cyclic(struct xdma_engine *engine);
struct xdma_transfer *engine_transfer_completion(struct xdma_engine *engine,
            struct xdma_transfer *transfer);
struct xdma_transfer *engine_service_transfer_list(struct xdma_engine *engine,
			struct xdma_transfer *transfer, u32 *pdesc_completed);
static void engine_err_handle(struct xdma_engine *engine,
		struct xdma_transfer *transfer, u32 desc_completed);
struct xdma_transfer *engine_service_final_transfer(struct xdma_engine *engine,
			struct xdma_transfer *transfer, u32 *pdesc_completed);
static void engine_service_perf(struct xdma_engine *engine, u32 desc_completed);
static void engine_service_resume(struct xdma_engine *engine);
static int engine_service(struct xdma_engine *engine, int desc_writeback);
static void engine_service_work(struct work_struct *work);
static int engine_service_poll(struct xdma_engine *engine,
		u32 expected_desc_count);
static void user_irq_service(struct xdma_irq *user_irq);
static irqreturn_t xdma_isr(int irq, void *dev_id);
static irqreturn_t xdma_user_irq(int irq, void *dev_id);
static irqreturn_t xdma_channel_irq(int irq, void *dev_id);
static void unmap_bars(struct xdma_dev *lro, struct pci_dev *dev);
static int map_single_bar(struct xdma_dev *lro, struct pci_dev *dev, int idx);
static int is_config_bar(struct xdma_dev *lro, int idx);
static void identify_bars(struct xdma_dev *lro, int *bar_id_list, int num_bars,
	int config_bar_pos);
static int map_bars(struct xdma_dev *lro, struct pci_dev *dev);
static void dump_desc(struct xdma_desc *desc_virt);
static void transfer_dump(struct xdma_transfer *transfer);
static struct xdma_desc *xdma_desc_alloc(struct pci_dev *dev, int number,
		dma_addr_t *desc_bus_p, struct xdma_desc **desc_last_p);
static void xdma_desc_link(struct xdma_desc *first, struct xdma_desc *second,
		dma_addr_t second_bus);
static void xdma_transfer_cyclic(struct xdma_transfer *transfer);
static void xdma_desc_adjacent(struct xdma_desc *desc, int next_adjacent);
static void xdma_desc_control(struct xdma_desc *first, u32 control_field);
static void xdma_desc_control_clear(struct xdma_desc *first, u32 clear_mask);
static void xdma_desc_control_set(struct xdma_desc *first, u32 set_mask);
static void xdma_desc_free(struct pci_dev *dev, int number,
		struct xdma_desc *desc_virt, dma_addr_t desc_bus);
static void xdma_desc_set(struct xdma_desc *desc, dma_addr_t rc_bus_addr,
		u64 ep_addr, int len, int dir_to_dev);
static void xdma_desc_set_source(struct xdma_desc *desc, u64 source);
static void transfer_set_result_addresses(struct xdma_transfer *transfer,
		u64 result_bus);
static void transfer_set_all_control(struct xdma_transfer *transfer,
		u32 control);
static void chain_transfers(struct xdma_engine *engine,
		struct xdma_transfer *transfer);
static int transfer_queue(struct xdma_engine *engine,
		struct xdma_transfer *transfer);
void engine_reinit(const struct xdma_engine *engine);
static void engine_alignments(struct xdma_engine *engine);
static void engine_destroy(struct xdma_dev *lro, struct xdma_engine *engine);
static void engine_msix_teardown(struct xdma_engine *engine);
static int engine_msix_setup(struct xdma_engine *engine);
static void engine_writeback_teardown(struct xdma_engine *engine);
static int engine_writeback_setup(struct xdma_engine *engine);
static struct xdma_engine *engine_create(struct xdma_dev *lro, int offset,
		int dir_to_dev, int channel);
static void transfer_destroy(struct xdma_dev *lro,
		struct xdma_transfer *transfer);
static inline void xdma_desc_force_complete(struct xdma_desc *transfer);
static void transfer_perf(struct xdma_transfer *transfer, int last);
static int transfer_build(struct xdma_transfer *transfer, u64 ep_addr,
		int dir_to_dev, int non_incr_addr, int force_new_desc,
		int userspace);
static struct xdma_transfer *transfer_create(struct xdma_dev *lro,
		const char *start, size_t cnt, u64 ep_addr, int dir_to_dev,
		int non_incr_addr, int force_new_desc, int userspace);
static int check_transfer_align(struct xdma_engine *engine,
	const char __user *buf, size_t count, loff_t pos, int sync);
static ssize_t sg_aio_read_write(struct kiocb *iocb, const struct iovec *iov,
		unsigned long nr_segs, loff_t pos, int dir_to_dev);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,15,0)
static ssize_t sg_read_iter (struct kiocb *iocb, struct iov_iter *);
static ssize_t sg_write_iter (struct kiocb *iocb, struct iov_iter *);
#else
static ssize_t sg_aio_read(struct kiocb *iocb, const struct iovec *iov,
		unsigned long nr_segs, loff_t pos);
static ssize_t sg_aio_write(struct kiocb *iocb, const struct iovec *iov,
		unsigned long nr_segs, loff_t pos);
#endif
static loff_t char_sgdma_llseek(struct file *file, loff_t off, int whence);
static int transfer_monitor(struct xdma_engine *engine,
	struct xdma_transfer *transfer);
static ssize_t transfer_data(struct xdma_engine *engine, char *transfer_addr,
		ssize_t remaining, loff_t *pos, int seq);
static ssize_t char_sgdma_read_write(struct file *file, char __user *buf,
		size_t count, loff_t *pos, int dir_to_dev);
static int transfer_monitor_cyclic(struct xdma_engine *engine,
	struct xdma_transfer *transfer);
static int copy_cyclic_to_user(struct xdma_engine *engine, int pkt_length,
	int head, char __user *buf);
static int complete_cyclic(struct xdma_engine *engine, char __user *buf);
static ssize_t char_sgdma_read_cyclic(struct file *file, char __user *buf);
static void get_perf_stats(struct xdma_engine *engine);
static int ioctl_do_perf_start(struct xdma_engine *engine, unsigned long arg);
static int ioctl_do_perf_stop(struct xdma_engine *engine, unsigned long arg);
static int ioctl_do_perf_get(struct xdma_engine *engine, unsigned long arg);
static int ioctl_do_addrmode_set(struct xdma_engine *engine, unsigned long arg);
static int ioctl_do_addrmode_get(struct xdma_engine *engine, unsigned long arg);
static int ioctl_do_align_get(struct xdma_engine *engine, unsigned long arg);
static long char_sgdma_ioctl(struct file *file, unsigned int cmd,
		unsigned long arg);
static ssize_t char_sgdma_write(struct file *file, const char __user *buf,
		size_t count, loff_t *pos);
static ssize_t char_sgdma_read(struct file *file, char __user *buf,
		size_t count, loff_t *pos);
static int char_open(struct inode *inode, struct file *file);
static int char_close(struct inode *inode, struct file *file);
static int cyclic_transfer_setup(struct xdma_engine *engine);
static int char_sgdma_open(struct inode *inode, struct file *file);
static int cyclic_shutdown_polled(struct xdma_engine *engine);
static int cyclic_shutdown_interrupt(struct xdma_engine *engine);
static int cyclic_transfer_teardown(struct xdma_engine *engine);
static int char_sgdma_close(struct inode *inode, struct file *file);
static int msi_msix_capable(struct pci_dev *dev, int type);
static struct xdma_dev *alloc_dev_instance(struct pci_dev *pdev);
static int probe_scan_for_msi(struct xdma_dev *lro, struct pci_dev *pdev);
static int request_regions(struct xdma_dev *lro, struct pci_dev *pdev);
static int set_dma_mask(struct pci_dev *pdev);
static u32 build_vector_reg(u32 a, u32 b, u32 c, u32 d);
static void write_msix_vectors(struct xdma_dev *lro);
static int msix_irq_setup(struct xdma_dev *lro);
static void irq_teardown(struct xdma_dev *lro);
static int irq_setup(struct xdma_dev *lro, struct pci_dev *pdev);
static void enable_credit_feature(struct xdma_dev *lro);
static u32 get_engine_type(struct engine_regs *regs);
static u32 get_engine_channel_id(struct engine_regs *regs);
static u32 get_engine_id(struct engine_regs *regs);
static void remove_engines(struct xdma_dev *lro);
static int probe_for_engine(struct xdma_dev *lro, int dir_to_dev, int channel);
static void destroy_interfaces(struct xdma_dev *lro);
static int create_engine_interface(struct xdma_engine *engine, int channel,
		int dir_to_dev);
static int create_bypass_interface(struct xdma_engine *engine, int channel,
		int dir_to_dev);
static int create_interfaces(struct xdma_dev *lro);
static int probe_engines(struct xdma_dev *lro);
static void enable_pcie_relaxed_ordering(struct pci_dev *dev);
static int probe(struct pci_dev *pdev, const struct pci_device_id *id);
static void remove(struct pci_dev *pdev);
static int bridge_mmap(struct file *file, struct vm_area_struct *vma);
static ssize_t char_ctrl_read(struct file *file, char __user *buf, size_t count,
		loff_t *pos);
static ssize_t char_ctrl_write(struct file *file, const char __user *buf,
		size_t count, loff_t *pos);
static ssize_t char_events_read(struct file *file, char __user *buf,
		size_t count, loff_t *pos);
static unsigned int char_events_poll(struct file *file, poll_table *wait);
static int copy_desc_data(struct xdma_transfer *transfer, char __user *buf,
		size_t *buf_offset, size_t buf_size);
static ssize_t char_bypass_read(struct file *file, char __user *buf,
		size_t count, loff_t *pos);
static ssize_t char_bypass_write(struct file *file, const char __user *buf,
		size_t count, loff_t *pos);
static int destroy_sg_char(struct xdma_char *lro_char);
static int gen_dev_major(struct xdma_char *lro_char);
static int gen_dev_minor(struct xdma_engine *engine, enum chardev_type type,
		int event_id);
static const struct file_operations *select_file_ops(enum chardev_type type);
static int config_kobject(struct xdma_char *lro_char, enum chardev_type type);
static int create_dev(struct xdma_char *lro_char, enum chardev_type type);
static struct xdma_char *create_sg_char(struct xdma_dev *lro, int bar,
		struct xdma_engine *engine, enum chardev_type type);
static int __init xdma_init(void);
static void __exit xdma_exit(void);

#define MAX_XDMA_DEVICES 64
static char dev_present[MAX_XDMA_DEVICES];

ssize_t show_device_numbers(struct device *dev, struct device_attribute *attr, char *buf){
	struct xdma_dev *lro;
	lro = (struct xdma_dev *)dev_get_drvdata(dev);
	return snprintf(buf, PAGE_SIZE, "%d\t%d\n", lro->major, lro->instance);
}

static DEVICE_ATTR(xdma_dev_instance, S_IRUGO, show_device_numbers, NULL);

/* SECTION: Callback tables */

/*
 * character device file operations for SG DMA engine
 */
static const struct file_operations sg_polled_fops = {
	.owner = THIS_MODULE,
	.open = char_sgdma_open,
	.release = char_sgdma_close,
	.read = char_sgdma_read,
	.write = char_sgdma_write,
	.unlocked_ioctl = char_sgdma_ioctl,
	.llseek = char_sgdma_llseek,
};

static const struct file_operations sg_interrupt_fops = {
	.owner = THIS_MODULE,
	.open = char_sgdma_open,
	.release = char_sgdma_close,
	.read = char_sgdma_read,
	.write = char_sgdma_write,
	.unlocked_ioctl = char_sgdma_ioctl,
	.llseek = char_sgdma_llseek,
#if !defined(XDMA_NEW_AIO)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,15,0)
	.read_iter = sg_read_iter,
	.write_iter = sg_write_iter,
#else
	.aio_read = sg_aio_read,
	.aio_write = sg_aio_write,
#endif
#endif
};

/*
 * character device file operations for control bus (through control bridge)
 */
static const struct file_operations ctrl_fops = {
	.owner = THIS_MODULE,
	.open = char_open,
	.release = char_close,
	.read = char_ctrl_read,
	.write = char_ctrl_write,
	.mmap = bridge_mmap,
#if SD_ACCEL
	.unlocked_ioctl = char_ctrl_ioctl,
#endif
};

/*
 * character device file operations for bypass operation
 */
static const struct file_operations bypass_fops = {
	.owner = THIS_MODULE,
	.open = char_open,
	.release = char_close,
	.read = char_bypass_read,
	.write = char_bypass_write,
	.mmap = bridge_mmap,	
};


/*
 * character device file operations for the irq events
 */
static const struct file_operations events_fops = {
	.owner = THIS_MODULE,
	.open = char_open,
	.release = char_close,
	.read = char_events_read,
	.poll = char_events_poll,
};

static struct pci_driver pci_driver = {
	.name = DRV_NAME,
	.id_table = pci_ids,
	.probe = probe,
	.remove = remove,
};

/* SECTION: Function definitions */

inline void write_register(u32 value, void *iomem)
{
	iowrite32(value, iomem);
}

inline u32 read_register(void *iomem)
{
	return ioread32(iomem);
}

static inline u32 build_u32(u32 hi, u32 lo)
{
	return ((hi & 0xFFFFUL) << 16) | (lo & 0xFFFFUL);
}
static inline u64 build_u64(u64 hi, u64 lo)
{
	return ((hi & 0xFFFFFFFULL) << 32) | (lo & 0xFFFFFFFFULL);
}

#if SD_ACCEL
static u64 find_feature_id(const struct xdma_dev *lro)
{
	u64 low = 0;
	u64 high = 0;

	low = ioread32(lro->bar[lro->user_bar_idx] + FEATURE_ID);
	high = ioread32(lro->bar[lro->user_bar_idx] + FEATURE_ID + 8);
	return low | (high << 32);
}
#else
static inline u64 find_feature_id(const struct xdma_dev *lro)
{
	return 0;
}
#endif

#if XDMA_STATUS_DUMPS
static void interrupt_status(struct xdma_dev *lro)
{
	struct interrupt_regs *reg = (struct interrupt_regs *)
		(lro->bar[lro->config_bar_idx] + XDMA_OFS_INT_CTRL);
	u32 w;

	dbg_irq("reg = %p\n", reg);
	dbg_irq("&reg->user_int_enable = %p\n", &reg->user_int_enable);

	w = read_register(&reg->user_int_enable);
	dbg_irq("user_int_enable = 0x%08x\n", w);
	w = read_register(&reg->channel_int_enable);
	dbg_irq("channel_int_enable = 0x%08x\n", w);

	w = read_register(&reg->user_int_request);
	dbg_irq("user_int_request = 0x%08x\n", w);
	w = read_register(&reg->channel_int_request);
	dbg_irq("channel_int_request = 0x%08x\n", w);

	w = read_register(&reg->user_int_pending);
	dbg_irq("user_int_pending = 0x%08x\n", w);
	w = read_register(&reg->channel_int_pending);
	dbg_irq("channel_int_pending = 0x%08x\n", w);
}
#else
static void interrupt_status(struct xdma_dev *lro)
{
}
#endif

/* channel_interrupts_enable -- Enable interrupts we are interested in */
static void channel_interrupts_enable(struct xdma_dev *lro, u32 mask)
{
	struct interrupt_regs *reg = (struct interrupt_regs *)
		(lro->bar[lro->config_bar_idx] + XDMA_OFS_INT_CTRL);

	write_register(mask, &reg->channel_int_enable_w1s);
}

/* channel_interrupts_disable -- Disable interrupts we not interested in */
static void channel_interrupts_disable(struct xdma_dev *lro, u32 mask)
{
	struct interrupt_regs *reg = (struct interrupt_regs *)
		(lro->bar[lro->config_bar_idx] + XDMA_OFS_INT_CTRL);

	write_register(mask, &reg->channel_int_enable_w1c);
}

/* user_interrupts_enable -- Enable interrupts we are interested in */
static void user_interrupts_enable(struct xdma_dev *lro, u32 mask)
{
	struct interrupt_regs *reg = (struct interrupt_regs *)
		(lro->bar[lro->config_bar_idx] + XDMA_OFS_INT_CTRL);

	write_register(mask, &reg->user_int_enable_w1s);
}

/* user_interrupts_disable -- Disable interrupts we not interested in */
static void user_interrupts_disable(struct xdma_dev *lro, u32 mask)
{
	struct interrupt_regs *reg = (struct interrupt_regs *)
		(lro->bar[lro->config_bar_idx] + XDMA_OFS_INT_CTRL);

	write_register(mask, &reg->user_int_enable_w1c);
}

/* read_interrupts -- Print the interrupt controller status */
static u32 read_interrupts(struct xdma_dev *lro)
{
	struct interrupt_regs *reg = (struct interrupt_regs *)
		(lro->bar[lro->config_bar_idx] + XDMA_OFS_INT_CTRL);
	u32 lo;
	u32 hi;

	/* extra debugging; inspect complete engine set of registers */
	hi = read_register(&reg->user_int_request);
	dbg_io("ioread32(0x%p) returned 0x%08x (user_int_request).\n",
		&reg->user_int_request, hi);
	lo = read_register(&reg->channel_int_request);
	dbg_io("ioread32(0x%p) returned 0x%08x (channel_int_request)\n",
		&reg->channel_int_request, lo);

	/* return interrupts: user in upper 16-bits, channel in lower 16-bits */
	return build_u32(hi, lo);
}

static void *rvmalloc(unsigned long size)
{
	void *mem;
	unsigned long adr;

	size = PAGE_ALIGN(size);
	mem = vmalloc_32(size);
	if (!mem)
		return NULL;

	adr = (unsigned long)mem;
	while (size > 0) {
		SetPageReserved(vmalloc_to_page((void *)adr));
		adr += PAGE_SIZE;
		size -= PAGE_SIZE;
	}
	return mem;
}

/* free reserved vmalloc()ed memory */
static void rvfree(void *mem, unsigned long size)
{
	unsigned long adr;

	if (!mem)
		return;

	adr = (unsigned long)mem;

	while ((long) size > 0) {
		ClearPageReserved(vmalloc_to_page((void *)adr));
		adr += PAGE_SIZE;
		size -= PAGE_SIZE;
	}
	vfree(mem);
}

static int xdma_performance_submit(struct xdma_dev *lro,
		struct xdma_engine *engine)
{
	u8 *buffer_virt;
	u32 max_consistent_size = 128 * 32 * 1024; /* 1024 pages, 4MB */
	dma_addr_t buffer_bus;	/* bus address */
	struct xdma_transfer *transfer;
	u64 ep_addr = 0;
	int num_desc_in_a_loop = 128;
	int size_in_desc = engine->xdma_perf->transfer_size;
	int size = size_in_desc * num_desc_in_a_loop;
	int i;

	BUG_ON(size_in_desc > max_consistent_size);

	if (size > max_consistent_size) {
		size = max_consistent_size;
		num_desc_in_a_loop = size / size_in_desc;
	}

	buffer_virt = pci_alloc_consistent(lro->pci_dev, size, &buffer_bus);

	/* allocate transfer data structure */
	transfer = kzalloc(sizeof(struct xdma_transfer), GFP_KERNEL);
	BUG_ON(!transfer);

	/* 0 = write engine (to_dev=0) , 1 = read engine (to_dev=1) */
	transfer->dir_to_dev = engine->dir_to_dev;
	/* set number of descriptors */
	transfer->desc_num = num_desc_in_a_loop;
	/* allocate descriptor list */
	transfer->desc_virt = xdma_desc_alloc(lro->pci_dev,
				transfer->desc_num,
				&transfer->desc_bus, NULL);
	BUG_ON(!transfer->desc_virt);

	for (i = 0; i < transfer->desc_num; i++) {
		struct xdma_desc *desc = transfer->desc_virt + i;
		dma_addr_t rc_bus_addr = buffer_bus + size_in_desc * i;

		/* fill in descriptor entry with transfer details */
		xdma_desc_set(desc, rc_bus_addr, ep_addr, size_in_desc,
			engine->dir_to_dev);
	}

	/* stop engine and request interrupt on last descriptor */
	xdma_desc_control(transfer->desc_virt, 0);
	/* create a linked loop */
	xdma_desc_link(transfer->desc_virt + transfer->desc_num - 1,
		transfer->desc_virt, transfer->desc_bus);
	transfer->cyclic = 1;

	/* initialize wait queue */
	init_waitqueue_head(&transfer->wq);

	//printk("=== Descriptor print for PERF \n");
	//transfer_dump(transfer);

	dbg_perf("Queueing XDMA I/O %s request for performance measurement.\n",
		engine->dir_to_dev ? "write (to dev)" : "read (from dev)");
	transfer_queue(engine, transfer);
	return 0;
}

#if XDMA_STATUS_DUMPS
static void engine_reg_dump(struct xdma_engine *engine)
{
	u32 w;

	BUG_ON(!engine);

	dbg_tfr("ioread32(0x%p).\n", &engine->regs->identifier);
	w = read_register(&engine->regs->identifier);
	dbg_tfr("ioread32(0x%p) returned 0x%08x.\n", &engine->regs->identifier,
		w);
	w &= BLOCK_ID_MASK;
	if (w != BLOCK_ID_HEAD) {
		dbg_tfr("engine identifier not found\n");
		dbg_tfr("(found 0x%08x expected 0xad4bXX01)\n", w);
		return;
	}
	/* extra debugging; inspect complete engine set of registers */
	w = read_register(&engine->regs->status);
	dbg_tfr("ioread32(0x%p) returned 0x%08x (status).\n",
		&engine->regs->status, w);
	w = read_register(&engine->regs->control);
	dbg_tfr("ioread32(0x%p) returned 0x%08x (control)\n",
		&engine->regs->control, w);
	w = read_register(&engine->sgdma_regs->first_desc_lo);
	dbg_tfr("ioread32(0x%p) returned 0x%08x (first_desc_lo)\n",
		&engine->sgdma_regs->first_desc_lo, w);
	w = read_register(&engine->sgdma_regs->first_desc_hi);
	dbg_tfr("ioread32(0x%p) returned 0x%08x (first_desc_hi)\n",
		&engine->sgdma_regs->first_desc_hi, w);
	w = read_register(&engine->sgdma_regs->first_desc_adjacent);
	dbg_tfr("ioread32(0x%p) returned 0x%08x (first_desc_adjacent).\n",
		&engine->sgdma_regs->first_desc_adjacent, w);
	w = read_register(&engine->regs->completed_desc_count);
	dbg_tfr("ioread32(0x%p) returned 0x%08x (completed_desc_count).\n",
		&engine->regs->completed_desc_count, w);
	w = read_register(&engine->regs->interrupt_enable_mask);
	dbg_tfr("ioread32(0x%p) returned 0x%08x (interrupt_enable_mask)\n",
		&engine->regs->interrupt_enable_mask, w);
}
#else
static void engine_reg_dump(struct xdma_engine *engine)
{
}
#endif

/**
 * engine_status_read() - read status of SG DMA engine (optionally reset)
 *
 * Stores status in engine->status.
 *
 * @return -1 on failure, status register otherwise
 */
static u32 engine_status_read(struct xdma_engine *engine, int clear)
{
	u32 value;

	BUG_ON(!engine);

	engine_reg_dump(engine);

	/* read status register */
	dbg_tfr("Status of SG DMA %s engine:\n", engine->name);
	dbg_tfr("ioread32(0x%p).\n", &engine->regs->status);
	if (clear) {
		value = engine->status =
			read_register(&engine->regs->status_rc);
	} else {
		value = engine->status = read_register(&engine->regs->status);
	}
	dbg_tfr("status = 0x%08x: %s%s%s%s%s%s%s%s%s\n", (u32)engine->status,
		(value & XDMA_STAT_BUSY) ? "BUSY " : "IDLE ",
		(value & XDMA_STAT_DESC_STOPPED) ? "DESC_STOPPED " : "",
		(value & XDMA_STAT_DESC_COMPLETED) ? "DESC_COMPLETED " : "",
		(value & XDMA_STAT_ALIGN_MISMATCH) ? "ALIGN_MISMATCH " : "",
		(value & XDMA_STAT_MAGIC_STOPPED) ? "MAGIC_STOPPED " : "",
		(value & XDMA_STAT_FETCH_STOPPED) ? "FETCH_STOPPED " : "",
		(value & XDMA_STAT_READ_ERROR) ? "READ_ERROR " : "",
		(value & XDMA_STAT_DESC_ERROR) ? "DESC_ERROR " : "",
		(value & XDMA_STAT_IDLE_STOPPED) ? "IDLE_STOPPED " : "");

	return value;
}

/**
 * xdma_engine_stop() - stop an SG DMA engine
 *
 */
static void xdma_engine_stop(struct xdma_engine *engine)
{
	u32 w;

	BUG_ON(!engine);
	dbg_tfr("xdma_engine_stop(engine=%p)\n", engine);

	w = 0;
	w |= (u32)XDMA_CTRL_IE_DESC_ALIGN_MISMATCH;
	w |= (u32)XDMA_CTRL_IE_MAGIC_STOPPED;
	w |= (u32)XDMA_CTRL_IE_READ_ERROR;
	w |= (u32)XDMA_CTRL_IE_DESC_ERROR;

	if (poll_mode) {
		w |= (u32) XDMA_CTRL_POLL_MODE_WB;
	} else {
		w |= (u32)XDMA_CTRL_IE_DESC_STOPPED;
		w |= (u32)XDMA_CTRL_IE_DESC_COMPLETED;

		/* Disable IDLE STOPPED for MM */
		if ((engine->streaming && (engine->dir_to_dev == 0)) ||
			(engine->xdma_perf))
			w |= (u32)XDMA_CTRL_IE_IDLE_STOPPED;
	}

	dbg_tfr("Stopping SG DMA %s engine; writing 0x%08x to 0x%p.\n",
			engine->name, w, (u32 *)&engine->regs->control);
	write_register(w, &engine->regs->control);
	/* dummy read of status register to flush all previous writes */
	dbg_tfr("xdma_engine_stop(%s) done\n", engine->name);
}

/**
 * engine_cyclic_stop() - stop a cyclic transfer running on an SG DMA engine
 *
 * engine->lock must be taken
 */
static struct xdma_transfer *engine_cyclic_stop(struct xdma_engine *engine)
{
	struct xdma_transfer *transfer = 0;

	/* transfers on queue? */
	if (!list_empty(&engine->transfer_list)) {
		/* pick first transfer on the queue (was submitted to engine) */
		transfer = list_entry(engine->transfer_list.next,
					struct xdma_transfer, entry);
		BUG_ON(!transfer);

		xdma_engine_stop(engine);

		if (transfer->cyclic) {
			if (engine->xdma_perf)
				dbg_perf("Stopping perf transfer on %s\n",
					engine->name);
			else
				dbg_perf("Stopping cyclic transfer on %s\n",
					engine->name);
			/* make sure the handler sees correct transfer state */
			transfer->cyclic = 1;
			/*
			 * set STOP flag and interrupt on completion, on the
			 * last descriptor
			 */
			xdma_desc_control(
				transfer->desc_virt + transfer->desc_num - 1,
				XDMA_DESC_COMPLETED | XDMA_DESC_STOPPED);
		} else {
			dbg_sg("(engine=%p) running transfer is not cyclic\n",
				engine);
		}
	} else {
		dbg_sg("(engine=%p) found not running transfer.\n", engine);
	}
	return transfer;
}

#if XDMA_STATUS_DUMPS
static inline void dump_engine_status(struct xdma_engine *engine)
{
	engine_status_read(engine, 0);
}
#else
static inline void dump_engine_status(struct xdma_engine *engine)
{
}
#endif

static void engine_start_mode_config(struct xdma_engine *engine)
{
	u32 w;

	BUG_ON(!engine);

	/* If a perf test is running, enable the engine interrupts */
	if (engine->xdma_perf) {
		w = XDMA_CTRL_IE_DESC_STOPPED;
		w |= XDMA_CTRL_IE_DESC_COMPLETED;
		w |= XDMA_CTRL_IE_DESC_ALIGN_MISMATCH;
		w |= XDMA_CTRL_IE_MAGIC_STOPPED;
		w |= XDMA_CTRL_IE_IDLE_STOPPED;
		w |= XDMA_CTRL_IE_READ_ERROR;
		w |= XDMA_CTRL_IE_DESC_ERROR;

		write_register(w, &engine->regs->interrupt_enable_mask);
	}

	/* write control register of SG DMA engine */
	w = (u32)XDMA_CTRL_RUN_STOP;
	w |= (u32)XDMA_CTRL_IE_READ_ERROR;
	w |= (u32)XDMA_CTRL_IE_DESC_ERROR;
	w |= (u32)XDMA_CTRL_IE_DESC_ALIGN_MISMATCH;
	w |= (u32)XDMA_CTRL_IE_MAGIC_STOPPED;

	if (poll_mode) {
		w |= (u32)XDMA_CTRL_POLL_MODE_WB;
	} else {
		w |= (u32)XDMA_CTRL_IE_DESC_STOPPED;
		w |= (u32)XDMA_CTRL_IE_DESC_COMPLETED;

		/* enable IE_IDLE_STOP only for AXI ST C2H and for perf test */
		if (engine->streaming && !engine->dir_to_dev)
			w |= (u32)XDMA_CTRL_IE_IDLE_STOPPED;

		if (engine->xdma_perf)
			w |= (u32)XDMA_CTRL_IE_IDLE_STOPPED;

		/* set non-incremental addressing mode */
		if (engine->non_incr_addr)
			w |= (u32)XDMA_CTRL_NON_INCR_ADDR;
	}

	dbg_tfr("iowrite32(0x%08x to 0x%p) (control)\n", w,
			(void *)&engine->regs->control);
	/* start the engine */
	write_register(w, &engine->regs->control);

	/* dummy read of status register to flush all previous writes */
	w = read_register(&engine->regs->status);
	dbg_tfr("ioread32(0x%p) = 0x%08x (dummy read flushes writes).\n",
			&engine->regs->status, w);
}

/**
 * engine_start() - start an idle engine with its first transfer on queue
 *
 * The engine will run and process all transfers that are queued using
 * transfer_queue() and thus have their descriptor lists chained.
 *
 * During the run, new transfers will be processed if transfer_queue() has
 * chained the descriptors before the hardware fetches the last descriptor.
 * A transfer that was chained too late will invoke a new run of the engine
 * initiated from the engine_service() routine.
 *
 * The engine must be idle and at least one transfer must be queued.
 * This function does not take locks; the engine spinlock must already be
 * taken.
 *
 */
static struct xdma_transfer *engine_start(struct xdma_engine *engine)
{
	struct xdma_transfer *transfer;
	u32 w;
	int extra_adj = 0;

	/* engine must be idle */
	BUG_ON(engine->running);
	/* engine transfer queue must not be empty */
	BUG_ON(list_empty(&engine->transfer_list));
	/* inspect first transfer queued on the engine */
	transfer = list_entry(engine->transfer_list.next, struct xdma_transfer,
				entry);
	BUG_ON(!transfer);

	/* engine is no longer shutdown */
	engine->shutdown = ENGINE_SHUTDOWN_NONE;

	dbg_tfr("engine_start(%s): transfer=0x%p.\n", engine->name, transfer);

	/* initialize number of descriptors of dequeued transfers */
	engine->desc_dequeued = 0;

	/* write lower 32-bit of bus address of transfer first descriptor */
	w = cpu_to_le32(PCI_DMA_L(transfer->desc_bus));
	dbg_tfr("iowrite32(0x%08x to 0x%p) (first_desc_lo)\n", w,
			(void *)&engine->sgdma_regs->first_desc_lo);
	write_register(w, &engine->sgdma_regs->first_desc_lo);
	/* write upper 32-bit of bus address of transfer first descriptor */
	w = cpu_to_le32(PCI_DMA_H(transfer->desc_bus));
	dbg_tfr("iowrite32(0x%08x to 0x%p) (first_desc_hi)\n", w,
			(void *)&engine->sgdma_regs->first_desc_hi);
	write_register(w, &engine->sgdma_regs->first_desc_hi);

	if (transfer->desc_adjacent > 0) {
		extra_adj = transfer->desc_adjacent - 1;
		if (extra_adj > MAX_EXTRA_ADJ)
			extra_adj = MAX_EXTRA_ADJ;
	}
	dbg_tfr("iowrite32(0x%08x to 0x%p) (first_desc_adjacent)\n",
		extra_adj, (void *)&engine->sgdma_regs->first_desc_adjacent);
	write_register(extra_adj, &engine->sgdma_regs->first_desc_adjacent);

	dbg_tfr("ioread32(0x%p) (dummy read flushes writes).\n",
		&engine->regs->status);
	mmiowb();

	engine_start_mode_config(engine);

	dump_engine_status(engine);

	dbg_tfr("%s engine 0x%p now running\n", engine->name, engine);
	/* remember the engine is running */
	engine->running = 1;
	return transfer;
}

#if SD_ACCEL
/* SD_Accel Specific */
/* engine_initialize -- Initialize the engine for use, read capabilities */
static int engine_initialize(struct xdma_dev *lro, int interrupts_offset)
{
	void *reg = lro->bar[lro->config_bar_idx] + interrupts_offset;
	u32 w;
	int rc = 0;
	printk(KERN_DEBUG "Read register at BAR %d, address 0x%p.\n", lro->config_bar_idx, reg);
	w = read_register(reg + 0x00);
	/* not a write nor a read engine found? */
	if (((w & 0x00ffff00UL) != 0x00c10000UL) && ((w & 0x00ffff00UL) != 0x00c20000UL)) {
		printk(KERN_DEBUG "Engine identifier not found (found 0x%08x expected 0xC100/0xC200).\n", w);
		rc = -1;
		goto fail_identifier;
	}
	printk(KERN_DEBUG "Engine identifier found 0x%08x with version %u.\n", w, w & 0xffU);

	/* before version 2, 64-bit DMA is not available */
	if ((w & 0xffUL) < 2UL) lro->capabilities &= ~(CAP_64BIT_DMA | CAP_64BIT_DESC);
	/* clear all interrupt event enables, stop engine */
	w = 0x0UL;
	printk(KERN_DEBUG "Set engine controller enable mask: 0x%08x.\n", w);
	write_register(w, reg + 0x04);
fail_identifier:
	return rc;
}
#else
static inline int engine_initialize(struct xdma_dev *lro, int interrupts_offset)
{
	return 0;
}
#endif

#if SD_ACCEL
/* SD_Accel Specific */
static int engine_version(struct xdma_dev *lro, int engine_offset)
{
	void *reg = lro->bar[lro->config_bar_idx] + engine_offset;
	u32 w;
	int rc = 0;
	w = read_register(reg + 0x00);
	/* not a write nor a read engine found? */
	if (((w & 0x00ffff00UL) != 0x00c10000UL) && ((w & 0x00ffff00UL) != 0x00c20000UL)) {
		printk(KERN_DEBUG "Engine identifier not found (found 0x%08x expected 0xC100/0xC200).\n", w);
		rc = -1;
		goto fail_identifier;
	}
	/* engine version */
	rc = w & 0xff;
fail_identifier:
	return rc;
}
#else
static inline int engine_version(struct xdma_dev *lro, int engine_offset)
{
	return 0;
}
#endif


/**
 * engine_service() - service an SG DMA engine
 *
 * must be called with engine->lock already acquired
 *
 * @engine pointer to struct xdma_engine
 *
 */
static void engine_service_shutdown(struct xdma_engine *engine)
{
	/* if the engine stopped with RUN still asserted, de-assert RUN now */
	dbg_tfr("engine just went idle, resetting RUN_STOP.\n");
	xdma_engine_stop(engine);
	engine->running = 0;

	/* awake task on engine's shutdown wait queue */
	wake_up_interruptible(&engine->shutdown_wq);
}

static void engine_transfer_dequeue(struct xdma_engine *engine)
{
	struct xdma_transfer *transfer;

	BUG_ON(!engine);

	/* pick first transfer on the queue (was submitted to the engine) */
	transfer = list_entry(engine->transfer_list.next, struct xdma_transfer,
		entry);
	BUG_ON(!transfer);
	BUG_ON(transfer != engine->rx_transfer_cyclic);
	dbg_tfr("%s engine completed cyclic transfer 0x%p (%d desc).\n",
		engine->name, transfer, transfer->desc_num);
	/* remove completed transfer from list */
	list_del(engine->transfer_list.next);
}

static int engine_ring_process(struct xdma_engine *engine)
{
	struct xdma_result *result;
	int start;
	int eop_count = 0;

	BUG_ON(!engine);
	result = (struct xdma_result *)engine->rx_result_buffer_virt;
	BUG_ON(!result);

	/* where we start receiving in the ring buffer */
	start = engine->rx_tail;

	/* iterate through all newly received RX result descriptors */
	while (result[engine->rx_tail].status && !engine->rx_overrun) {
		/* EOP bit set in result? */
		if (result[engine->rx_tail].status & RX_STATUS_EOP){
			eop_count++;
		}
		dbg_tfr("result[engine->rx_tail=%3d].status = 0x%08x\n",
			engine->rx_tail, (int)result[engine->rx_tail].status);
		dbg_tfr("result[engine->rx_tail=%3d].length = %d\n",
			engine->rx_tail, (int)result[engine->rx_tail].length);

		/* increment tail pointer */

		engine->rx_tail = (engine->rx_tail + 1) % RX_BUF_PAGES;

		/* overrun? */
		if (engine->rx_tail == engine->rx_head) {
			dbg_tfr("engine_service_cyclic(): overrun\n");
			/* flag to user space that overrun has occurred */
			engine->rx_overrun = 1;
		}
	
	}

	return eop_count;
}

static int engine_service_cyclic_polled(struct xdma_engine *engine)
{
	int eop_count = 0;
	int rc = 0;
	struct xdma_poll_wb *writeback_data;
	u32 sched_limit = 0;

	BUG_ON(!engine);
	BUG_ON(engine->magic != MAGIC_ENGINE);

	writeback_data = (struct xdma_poll_wb *)engine->poll_mode_addr_virt;

	while (eop_count == 0) {
		if (sched_limit != 0) {
			if ((sched_limit % NUM_POLLS_PER_SCHED) == 0)
				schedule();
		}
		sched_limit++;

		/* Monitor descriptor writeback address for errors */
		if ((writeback_data->completed_desc_count) & WB_ERR_MASK) {
			rc = -1;
			break;
		}

		eop_count = engine_ring_process(engine);
	}

	if (eop_count == 0) {
		engine_status_read(engine, 1);
		if ((engine->running) && !(engine->status & XDMA_STAT_BUSY)) {
			/* transfers on queue? */
			if (!list_empty(&engine->transfer_list))
				engine_transfer_dequeue(engine);

			engine_service_shutdown(engine);
		}
	}

	return rc;
}

static int engine_service_cyclic_interrupt(struct xdma_engine *engine)
{
	int eop_count = 0;

	BUG_ON(!engine);
	BUG_ON(engine->magic != MAGIC_ENGINE);

	engine_status_read(engine, 1);

	eop_count = engine_ring_process(engine);
	/*
	 * wake any reader on EOP, as one or more packets are now in
	 * the RX buffer
	 */
	if(enable_credit_mp){
		if (eop_count > 0) {
			//engine->eop_found = 1;
		}
		wake_up_interruptible(&engine->rx_transfer_cyclic->wq);
	}else{
		if (eop_count > 0) {
			/* awake task on transfer's wait queue */
			dbg_tfr("wake_up_interruptible() due to %d EOP's\n", eop_count);
			engine->eop_found = 1;
			wake_up_interruptible(&engine->rx_transfer_cyclic->wq);
		}
	}

	/* engine was running but is no longer busy? */
	if ((engine->running) && !(engine->status & XDMA_STAT_BUSY)) {
		/* transfers on queue? */
		if (!list_empty(&engine->transfer_list))
			engine_transfer_dequeue(engine);

		engine_service_shutdown(engine);
	}

	return 0;
}

/* must be called with engine->lock already acquired */
static int engine_service_cyclic(struct xdma_engine *engine)
{
	int rc = 0;

	dbg_tfr("engine_service_cyclic()");

	BUG_ON(!engine);
	BUG_ON(engine->magic != MAGIC_ENGINE);

	if (poll_mode)
		rc = engine_service_cyclic_polled(engine);
	else
		rc = engine_service_cyclic_interrupt(engine);

	return rc;
}

struct xdma_transfer *engine_transfer_completion(struct xdma_engine *engine,
		struct xdma_transfer *transfer)
{
	BUG_ON(!engine);
	BUG_ON(!transfer);

	if ((transfer->iocb) && (transfer->last_in_request)) {
		/* asynchronous I/O? */
		struct kiocb *iocb = transfer->iocb;
		ssize_t done = transfer->size_of_request;

		dbg_tfr("Freeing (async I/O req) last transfer %p, iocb %p\n",
			transfer, transfer->iocb);
		transfer_destroy(engine->lro, transfer);
		transfer = NULL;
		dbg_tfr("Completing async I/O iocb %p with size %d\n", iocb,
			(int)done);
		/* indicate I/O completion XXX res, res2 */
		AIO_COMPLETE(iocb, done);
	} else {
		/* synchronous I/O? */
		/* awake task on transfer's wait queue */
		wake_up_interruptible(&transfer->wq);
	}

	return transfer;
}

struct xdma_transfer *engine_service_transfer_list(struct xdma_engine *engine,
		struct xdma_transfer *transfer, u32 *pdesc_completed)
{
	BUG_ON(!engine);
	BUG_ON(!transfer);
	BUG_ON(!pdesc_completed);

	/*
	 * iterate over all the transfers completed by the engine,
	 * except for the last (i.e. use > instead of >=).
	 */
	while (transfer && (!transfer->cyclic) &&
		(*pdesc_completed > transfer->desc_num)) {
		/* remove this transfer from pdesc_completed */
		*pdesc_completed -= transfer->desc_num;
		dbg_tfr("%s engine completed non-cyclic xfer 0x%p (%d desc)\n",
			engine->name, transfer, transfer->desc_num);
		/* remove completed transfer from list */
		list_del(engine->transfer_list.next);
		/* add to dequeued number of descriptors during this run */
		engine->desc_dequeued += transfer->desc_num;
		/* mark transfer as succesfully completed */
		transfer->state = TRANSFER_STATE_COMPLETED;

		/* Complete transfer - sets transfer to NULL if an async
		 * transfer has completed */
		transfer = engine_transfer_completion(engine, transfer);

		/* if exists, get the next transfer on the list */
		if (!list_empty(&engine->transfer_list)) {
			transfer = list_entry(engine->transfer_list.next,
					struct xdma_transfer, entry);
			dbg_tfr("Non-completed transfer %p\n", transfer);
		} else {
			/* no further transfers? */
			transfer = NULL;
		}
	}

	return transfer;
}

static void engine_err_handle(struct xdma_engine *engine,
		struct xdma_transfer *transfer, u32 desc_completed)
{
	u32 value;

	/*
	 * The BUSY bit is expected to be clear now but older HW has a race
	 * condition which could cause it to be still set.  If it's set, re-read
	 * and check again.  If it's still set, log the issue.
	 */
	if (engine->status & XDMA_STAT_BUSY) {
		value = read_register(&engine->regs->status);
		if (value & XDMA_STAT_BUSY)
			dbg_tfr("%s engine has errors but is still BUSY\n",
				engine->name);
	}
	//printk("=== Descriptor print for PERF \n");
	//transfer_dump(transfer);

	dbg_tfr("Aborted %s engine transfer 0x%p\n", engine->name, transfer);
	dbg_tfr("%s engine was %d descriptors into transfer (with %d desc)\n",
		engine->name, desc_completed, transfer->desc_num);
	dbg_tfr("%s engine status = %d\n", engine->name, engine->status);
	
	/* mark transfer as failed */
	transfer->state = TRANSFER_STATE_FAILED;
	xdma_engine_stop(engine);
}

struct xdma_transfer *engine_service_final_transfer(struct xdma_engine *engine,
			struct xdma_transfer *transfer, u32 *pdesc_completed)
{
	u32 err_flags;
	BUG_ON(!engine);
	BUG_ON(!transfer);
	BUG_ON(!pdesc_completed);

	err_flags = XDMA_STAT_MAGIC_STOPPED;
	err_flags |= XDMA_STAT_ALIGN_MISMATCH;
	err_flags |= XDMA_STAT_READ_ERROR;
	err_flags |= XDMA_STAT_DESC_ERROR;

	/* inspect the current transfer */
	if (transfer) {
		if (engine->status & err_flags) {
			engine_err_handle(engine, transfer, *pdesc_completed);
			return transfer;
		}

		if (engine->status & XDMA_STAT_BUSY)
			dbg_tfr("Engine %s is unexpectedly busy - ignoring\n",
				engine->name);

		/* the engine stopped on current transfer? */
		if (*pdesc_completed < transfer->desc_num) {
			transfer->state = TRANSFER_STATE_FAILED;
			dbg_tfr("Engine stopped half-way\n");
			dbg_tfr("transfer %p\n", transfer);
			dbg_tfr("*pdesc_completed=%d, transfer->desc_num=%d",
				*pdesc_completed, transfer->desc_num);
		} else {
			dbg_tfr("engine %s completed transfer\n", engine->name);
			dbg_tfr("Completed transfer ID = 0x%p\n", transfer);
			dbg_tfr("*pdesc_completed=%d, transfer->desc_num=%d",
				*pdesc_completed, transfer->desc_num);

			if (!transfer->cyclic) {
				/*
				 * if the engine stopped on this transfer,
				 * it should be the last
				 */
				WARN_ON(*pdesc_completed > transfer->desc_num);
			}
			/* mark transfer as succesfully completed */
			transfer->state = TRANSFER_STATE_COMPLETED;
		}

		/* remove completed transfer from list */
		list_del(engine->transfer_list.next);
		/* add to dequeued number of descriptors during this run */
		engine->desc_dequeued += transfer->desc_num;

		/*
		 * Complete transfer - sets transfer to NULL if an asynchronous
		 * transfer has completed
		 */
		transfer = engine_transfer_completion(engine, transfer);
	}

	return transfer;
}

static void engine_service_perf(struct xdma_engine *engine, u32 desc_completed)
{
	BUG_ON(!engine);

	/* performance measurement is running? */
	if (engine->xdma_perf) {
		/* a descriptor was completed? */
		if (engine->status & XDMA_STAT_DESC_COMPLETED) {
			engine->xdma_perf->iterations = desc_completed;
			dbg_perf("transfer->xdma_perf->iterations=%d\n",
				engine->xdma_perf->iterations);
		}

		/* a descriptor stopped the engine? */
		if (engine->status & XDMA_STAT_DESC_STOPPED) {
			engine->xdma_perf->stopped = 1;
			/*
			 * wake any XDMA_PERF_IOCTL_STOP waiting for
			 * the performance run to finish
			 */
			wake_up_interruptible(&engine->xdma_perf_wq);
			dbg_perf("transfer->xdma_perf stopped\n");
		}
	}
}

static void engine_service_resume(struct xdma_engine *engine)
{
	struct xdma_transfer *transfer_started;

	BUG_ON(!engine);

	/* engine stopped? */
	if (!engine->running) {
		/* engine was requested to be shutdown? */
		if (engine->shutdown & ENGINE_SHUTDOWN_REQUEST) {
			engine->shutdown |= ENGINE_SHUTDOWN_IDLE;
			/* awake task on engine's shutdown wait queue */
			wake_up_interruptible(&engine->shutdown_wq);
		} else if (!list_empty(&engine->transfer_list)) {
			/* (re)start engine */
			transfer_started = engine_start(engine);
			dbg_tfr("re-started %s engine with pending xfer 0x%p\n",
				engine->name, transfer_started);
		} else {
			dbg_tfr("no pending transfers, %s engine stays idle.\n",
				engine->name);
		}
	} else {
		/* engine is still running? */
		if (list_empty(&engine->transfer_list)) {
			dbg_tfr("no queued transfers but %s engine running!\n",
				engine->name);
			WARN_ON(1);
		}
	}
}

/**
 * engine_service() - service an SG DMA engine
 *
 * must be called with engine->lock already acquired
 *
 * @engine pointer to struct xdma_engine
 *
 */
static int engine_service(struct xdma_engine *engine, int desc_writeback)
{
	struct xdma_transfer *transfer = NULL;
	u32 desc_count;
	u32 err_flag;
	int rc  = 0;
	struct xdma_poll_wb *wb_data;

	BUG_ON(!engine);

	desc_count = desc_writeback & WB_COUNT_MASK;
	err_flag = desc_writeback & WB_ERR_MASK;

	/* If polling detected an error, signal to the caller */
	if (err_flag)
		rc = -1;

	/* Service the engine */

	if (!engine->running) {
		dbg_tfr("Engine was not running!!! Clearing status\n");
		if (desc_writeback == 0)
			engine_status_read(engine, 1);
		return 0;
	}

	/*
	 * If called by the ISR or polling detected an error, read and clear
	 * engine status. For polled mode descriptor completion, this read is
	 * unnecessary and is skipped to reduce latency
	 */
	if ((desc_count == 0) || (err_flag != 0))
		engine_status_read(engine, 1);

	/*
	 * engine was running but is no longer busy, or writeback occurred,
	 * shut down
	 */
	if (((engine->running) && !(engine->status & XDMA_STAT_BUSY)) ||
		(desc_count != 0))
		engine_service_shutdown(engine);

	/*
	 * If called from the ISR, or if an error occurred, the descriptor
	 * count will be zero.  In this scenario, read the descriptor count
	 * from HW.  In polled mode descriptor completion, this read is
	 * unnecessary and is skipped to reduce latency
	 */
	if (desc_count == 0)
		desc_count = read_register(&engine->regs->completed_desc_count);

	dbg_tfr("desc_count = %d\n", desc_count);

	/* transfers on queue? */
	if (!list_empty(&engine->transfer_list)) {
		/* pick first transfer on queue (was submitted to the engine) */
		transfer = list_entry(engine->transfer_list.next,
				struct xdma_transfer, entry);

		dbg_tfr("head of queue transfer 0x%p has %d descriptors\n",
			transfer, (int)transfer->desc_num);

		dbg_tfr("Engine completed %d desc, %d not yet dequeued\n",
			(int)desc_count,
			(int)desc_count - engine->desc_dequeued);

		engine_service_perf(engine, desc_count);
	}

	/* account for already dequeued transfers during this engine run */
	desc_count -= engine->desc_dequeued;

	/* Process all but the last transfer */
	transfer = engine_service_transfer_list(engine, transfer, &desc_count);

	/*
	 * Process final transfer - includes checks of number of descriptors to
	 * detect faulty completion
	 */
	transfer = engine_service_final_transfer(engine, transfer, &desc_count);

	/* Before starting engine again, clear the writeback data */
	if (poll_mode) {
		wb_data = (struct xdma_poll_wb *)engine->poll_mode_addr_virt;
		wb_data->completed_desc_count = 0;
	}

	/* Restart the engine following the servicing */
	engine_service_resume(engine);

	return rc;
}

/* engine_service_work */
static void engine_service_work(struct work_struct *work)
{
	struct xdma_engine *engine;

	engine = container_of(work, struct xdma_engine, work);
	BUG_ON(engine->magic != MAGIC_ENGINE);

	/* lock the engine */
	spin_lock(&engine->lock);

	/* C2H streaming? */
	if (engine->rx_transfer_cyclic) {
		dbg_tfr("engine_service_cyclic() for %s engine %p\n",
			engine->name, engine);
		engine_service_cyclic(engine);
		/* no C2H streaming, default */
	} else {
		dbg_tfr("engine_service() for %s engine %p\n",
			engine->name, engine);
		engine_service(engine, 0);
	}

	/* re-enable interrupts for this engine */
	if(engine->lro->msix_enabled){
		write_register(engine->interrupt_enable_mask_value, &engine->regs->interrupt_enable_mask_w1s);
	}else{
		channel_interrupts_enable(engine->lro, engine->irq_bitmask);
	}
	/* unlock the engine */
	spin_unlock(&engine->lock);
}

static u32 engine_service_wb_monitor(struct xdma_engine *engine,
		u32 expected_wb)
{
	struct xdma_poll_wb *wb_data;
	u32 desc_wb = 0;
	u32 sched_limit = 0;
	unsigned long timeout;

	BUG_ON(!engine);
	wb_data = (struct xdma_poll_wb *)engine->poll_mode_addr_virt;

	/*
	 * Poll the writeback location for the expected number of
	 * descriptors / error events This loop is skipped for cyclic mode,
	 * where the expected_desc_count passed in is zero, since it cannot be
	 * determined before the function is called
	 */
	timeout = jiffies + (POLL_TIMEOUT_SECONDS * HZ);
	while (expected_wb != 0) {
		desc_wb = wb_data->completed_desc_count;

		if (desc_wb & WB_ERR_MASK)
			break;
		else if (desc_wb == expected_wb)
			break;

		/* RTO - prevent system from hanging in polled mode */
		if (time_after(jiffies, timeout)) {
			dbg_tfr("Polling timeout occurred");
			dbg_tfr("desc_wb = 0x%08x, expected 0x%08x\n", desc_wb,
				expected_wb);
			if ((desc_wb & WB_COUNT_MASK) > expected_wb)
				desc_wb = expected_wb | WB_ERR_MASK;

			break;
		}

		/*
		 * Define NUM_POLLS_PER_SCHED to limit how much time is spent
		 * in the scheduler
		 */
		if (sched_limit != 0) {
			if ((sched_limit % NUM_POLLS_PER_SCHED) == 0)
				schedule();
		}
		sched_limit++;
	}

	return desc_wb;
}

static int engine_service_poll(struct xdma_engine *engine,
		u32 expected_desc_count)
{
	struct xdma_poll_wb *writeback_data;
	u32 desc_wb = 0;
	int rc = 0;

	BUG_ON(!engine);
	BUG_ON(engine->magic != MAGIC_ENGINE);

	writeback_data = (struct xdma_poll_wb *)engine->poll_mode_addr_virt;

	if ((expected_desc_count & WB_COUNT_MASK) != expected_desc_count) {
		dbg_tfr("Queued descriptor count is larger than supported\n");
		return -1;
	}

	/*
	 * Poll the writeback location for the expected number of
	 * descriptors / error events This loop is skipped for cyclic mode,
	 * where the expected_desc_count passed in is zero, since it cannot be
	 * determined before the function is called
	 */
	desc_wb = engine_service_wb_monitor(engine, expected_desc_count);

	/* lock the engine */
	spin_lock(&engine->lock);
	/* C2H streaming? */
	if (engine->rx_transfer_cyclic) {
		dbg_tfr("Calling engine_service_cyclic() for %s engine %p\n",
			engine->name, engine);
		rc = engine_service_cyclic(engine);
		/* no C2H streaming, default */
	} else {
		dbg_tfr("Calling engine_service() for %s engine %p\n",
			engine->name, engine);
		rc = engine_service(engine, desc_wb);
	}
	/* unlock the engine */
	spin_unlock(&engine->lock);

	return rc;
}

static void user_irq_service(struct xdma_irq *user_irq)
{
	unsigned long flags;

	BUG_ON(!user_irq);

	spin_lock_irqsave(&(user_irq->events_lock), flags);
	if (!user_irq->events_irq) {
		user_irq->events_irq = 1;
		wake_up_interruptible(&(user_irq->events_wq));
	}
	spin_unlock_irqrestore(&(user_irq->events_lock), flags);
}

/*
 * xdma_isr() - Interrupt handler
 *
 * @dev_id pointer to xdma_dev
 */
static irqreturn_t xdma_isr(int irq, void *dev_id)
{
	u32 ch_irq;
	u32 user_irq;
	struct xdma_dev *lro;
	struct interrupt_regs *irq_regs;
	int user_irq_bit;
	struct xdma_engine *engine;
	int channel;

	dbg_irq("(irq=%d) <<<< INTERRUPT SERVICE ROUTINE\n", irq);
	BUG_ON(!dev_id);
	lro = (struct xdma_dev *)dev_id;

	if (!lro) {
		WARN_ON(!lro);
		dbg_irq("xdma_isr(irq=%d) lro=%p ??\n", irq, lro);
		return IRQ_NONE;
	}

	irq_regs = (struct interrupt_regs *)(lro->bar[lro->config_bar_idx] +
			XDMA_OFS_INT_CTRL);

	/* read channel interrupt requests */
	ch_irq = read_register(&irq_regs->channel_int_request);
	dbg_irq("ch_irq = 0x%08x\n", ch_irq);

	/*
	 * disable all interrupts that fired; these are re-enabled individually
	 * after the causing module has been fully serviced.
	 */
	channel_interrupts_disable(lro, ch_irq);

	/* read user interrupts - this read also flushes the above write */
	user_irq = read_register(&irq_regs->user_int_request);
	dbg_irq("user_irq = 0x%08x\n", user_irq);

	for (user_irq_bit = 0; user_irq_bit < MAX_USER_IRQ; user_irq_bit++) {
		if (user_irq & (1 << user_irq_bit))
			user_irq_service(&lro->user_irq[user_irq_bit]);
	}

	/* iterate over H2C (PCIe read) */
	for (channel = 0; channel < XDMA_CHANNEL_NUM_MAX; channel++) {
		engine = lro->engine[channel][0];
		/* engine present and its interrupt fired? */
		if (engine && (engine->irq_bitmask & ch_irq)) {
			dbg_tfr("schedule_work(engine=%p)\n", engine);
			schedule_work(&engine->work);
		}
	}

	/* iterate over C2H (PCIe write) */
	for (channel = 0; channel < XDMA_CHANNEL_NUM_MAX; channel++) {
		engine = lro->engine[channel][1];
		/* engine present and its interrupt fired? */
		if (engine && (engine->irq_bitmask & ch_irq)) {
			dbg_tfr("schedule_work(engine=%p)\n", engine);
			schedule_work(&engine->work);
		}
	}

	lro->irq_count++;
	return IRQ_HANDLED;
}

/*
 * xdma_user_irq() - Interrupt handler for user interrupts in MSI-X mode
 *
 * @dev_id pointer to xdma_dev
 */
static irqreturn_t xdma_user_irq(int irq, void *dev_id)
{
	struct xdma_irq *user_irq;

	dbg_irq("(irq=%d) <<<< INTERRUPT SERVICE ROUTINE\n", irq);

	BUG_ON(!dev_id);
	user_irq = (struct xdma_irq *)dev_id;

	user_irq_service(user_irq);

	return IRQ_HANDLED;
}

/*
 * xdma_channel_irq() - Interrupt handler for channel interrupts in MSI-X mode
 *
 * @dev_id pointer to xdma_dev
 */
static irqreturn_t xdma_channel_irq(int irq, void *dev_id)
{
	struct xdma_dev *lro;
	struct xdma_engine *engine;
	struct interrupt_regs *irq_regs;

	dbg_irq("(irq=%d) <<<< INTERRUPT service ROUTINE\n", irq);
	BUG_ON(!dev_id);

	engine = (struct xdma_engine *)dev_id;
	lro = engine->lro;

	if (!lro) {
		WARN_ON(!lro);
		dbg_irq("xdma_channel_irq(irq=%d) lro=%p ??\n", irq, lro);
		return IRQ_NONE;
	}

	irq_regs = (struct interrupt_regs *)(lro->bar[lro->config_bar_idx] +
			XDMA_OFS_INT_CTRL);

	/* Disable the interrupt for this engine */
	//channel_interrupts_disable(lro, engine->irq_bitmask);
	engine->interrupt_enable_mask_value = read_register(&engine->regs->interrupt_enable_mask);
	write_register(engine->interrupt_enable_mask_value, &engine->regs->interrupt_enable_mask_w1c);
	/* Dummy read to flush the above write */
	read_register(&irq_regs->channel_int_pending);
	/* Schedule the bottom half */
	schedule_work(&engine->work);

	/*
	 * RTO - need to protect access here if multiple MSI-X are used for
	 * user interrupts
	 */
	lro->irq_count++;
	return IRQ_HANDLED;
}

/*
 * Unmap the BAR regions that had been mapped earlier using map_bars()
 */
static void unmap_bars(struct xdma_dev *lro, struct pci_dev *dev)
{
	int i;

	for (i = 0; i < XDMA_BAR_NUM; i++) {
		/* is this BAR mapped? */
		if (lro->bar[i]) {
			/* unmap BAR */
			pci_iounmap(dev, lro->bar[i]);
			/* mark as unmapped */
			lro->bar[i] = NULL;
		}
	}
}

static int map_single_bar(struct xdma_dev *lro, struct pci_dev *dev, int idx)
{
	resource_size_t bar_start;
	resource_size_t bar_len;
	resource_size_t map_len;

	bar_start = pci_resource_start(dev, idx);
	bar_len = pci_resource_len(dev, idx);
	map_len = bar_len;

	lro->bar[idx] = NULL;

	/* do not map BARs with length 0. Note that start MAY be 0! */
	if (!bar_len) {
		dbg_init("BAR #%d is not present - skipping\n", idx);
		return 0;
	}

	/* BAR size exceeds maximum desired mapping? */
	if (bar_len > INT_MAX) {
		dbg_init("Limit BAR %d mapping from %llu to %d bytes\n", idx,
			(u64)bar_len, INT_MAX);
		map_len = (resource_size_t)INT_MAX;
	}
	/*
	 * map the full device memory or IO region into kernel virtual
	 * address space
	 */
	dbg_init("BAR%d: %llu bytes to be mapped.\n", idx, (u64)map_len);
	lro->bar[idx] = pci_iomap(dev, idx, map_len);

	if (!lro->bar[idx]) {
		dbg_init("Could not map BAR %d", idx);
		return -1;
	}

	dbg_init("BAR%d at 0x%llx mapped at 0x%p, length=%llu(/%llu)\n", idx,
		(u64)bar_start, lro->bar[idx], (u64)map_len, (u64)bar_len);

	return (int)map_len;
}

static int is_config_bar(struct xdma_dev *lro, int idx)
{
	u32 irq_id = 0;
	u32 cfg_id = 0;
	int flag = 0;
	u32 mask = 0xffff0000; /* Compare only XDMA ID's not Version number */
	struct interrupt_regs *irq_regs =
		(struct interrupt_regs *) (lro->bar[idx] + XDMA_OFS_INT_CTRL);
	struct config_regs *cfg_regs =
		(struct config_regs *)(lro->bar[idx] + XDMA_OFS_CONFIG);

	irq_id = read_register(&irq_regs->identifier);
	cfg_id = read_register(&cfg_regs->identifier);

	if (((irq_id & mask)== IRQ_BLOCK_ID) && ((cfg_id & mask)== CONFIG_BLOCK_ID)) {
		dbg_init("BAR %d is the XDMA config BAR\n", idx);
		flag = 1;
	} else {
		dbg_init("BAR %d is not XDMA config BAR, irq_id = %x, cfg_id = %x\n", idx, irq_id, cfg_id);
		flag = 0;
	}

	return flag;
}

static void identify_bars(struct xdma_dev *lro, int *bar_id_list, int num_bars,
	int config_bar_pos)
{
	/*
	 * The following logic identifies which BARs contain what functionality
	 * based on the position of the XDMA config BAR and the number of BARs
	 * detected. The rules are that the user logic and bypass logic BARs
	 * are optional.  When both are present, the XDMA config BAR will be the
	 * 2nd BAR detected (config_bar_pos = 1), with the user logic being
	 * detected first and the bypass being detected last. When one is
	 * omitted, the type of BAR present can be identified by whether the
	 * XDMA config BAR is detected first or last.  When both are omitted,
	 * only the XDMA config BAR is present.  This somewhat convoluted
	 * approach is used instead of relying on BAR numbers in order to work
	 * correctly with both 32-bit and 64-bit BARs.
	 */

	BUG_ON(!lro);
	BUG_ON(!bar_id_list);

	switch (num_bars) {
	case 1:
		/* Only one BAR present - no extra work necessary */
		break;

	case 2:
		if (config_bar_pos == 0) {
			lro->bypass_bar_idx = bar_id_list[1];
		} else if (config_bar_pos == 1) {
			lro->user_bar_idx = bar_id_list[0];
		} else {
			dbg_init("case 2\n");
			dbg_init("XDMA config BAR in unexpected position (%d)",
				config_bar_pos);
		}
		break;

	case 3:
		if (config_bar_pos == 1) {
			lro->user_bar_idx = bar_id_list[0];
			lro->bypass_bar_idx = bar_id_list[2];
		} else {
			dbg_init("case 3\n");
			dbg_init("XDMA config BAR in unexpected position (%d)",
				config_bar_pos);
		}
		break;

	default:
		/* Should not occur - warn user but safe to continue */
		dbg_init("Unexpected number of BARs (%d)\n", num_bars);
		dbg_init("Only XDMA config BAR accessible\n");
		break;

	}
}

/* map_bars() -- map device regions into kernel virtual address space
 *
 * Map the device memory regions into kernel virtual address space after
 * verifying their sizes respect the minimum sizes needed
 */
static int map_bars(struct xdma_dev *lro, struct pci_dev *dev)
{
	int rc;
	int i;
	int bar_id_list[XDMA_BAR_NUM];
	int bar_id_idx = 0;
	int config_bar_pos = 0;

	/* iterate through all the BARs */
	for (i = 0; i < XDMA_BAR_NUM; i++) {
		int bar_len;

		bar_len = map_single_bar(lro, dev, i);
		if (bar_len == 0) {
			continue;
		} else if (bar_len < 0) {
			rc = -1;
			goto fail;
		}

		/* Try to identify BAR as XDMA control BAR */
		if ((bar_len >= XDMA_BAR_SIZE) && (lro->config_bar_idx < 0)) {

			if (is_config_bar(lro, i)) {
				lro->config_bar_idx = i;
				config_bar_pos = bar_id_idx;
			}
		}

		bar_id_list[bar_id_idx] = i;
		bar_id_idx++;
	}

	/* The XDMA config BAR must always be present */
	if (lro->config_bar_idx < 0) {
		dbg_init("Failed to detect XDMA config BAR\n");
		rc = -1;
		goto fail;
	}

	identify_bars(lro, bar_id_list, bar_id_idx, config_bar_pos);

	/* successfully mapped all required BAR regions */
	rc = 0;
	goto success;
fail:
	/* unwind; unmap any BARs that we did map */
	unmap_bars(lro, dev);
success:
	return rc;
}

static void dump_desc(struct xdma_desc *desc_virt)
{
	int j;
	u32 *p = (u32 *)desc_virt;
	static char * const field_name[] = {
		"magic|extra_adjacent|control", "bytes", "src_addr_lo",
		"src_addr_hi", "dst_addr_lo", "dst_addr_hi", "next_addr",
		"next_addr_pad"};
	char *dummy;

	/* remove warning about unused variable when debug printing is off */
	dummy = field_name[0];

	for (j = 0; j < 8; j += 1) {
		dbg_desc("0x%08lx/0x%02lx: 0x%08x 0x%08x %s\n",
			 (uintptr_t)p, (uintptr_t)p & 15, (int)*p,
			 le32_to_cpu(*p), field_name[j]);
		p++;
	}
	dbg_desc("\n");
}

static void transfer_dump(struct xdma_transfer *transfer)
{
	int i;
	struct xdma_desc *desc_virt = transfer->desc_virt;

	dbg_desc("Descriptor Entry (Pre-Transfer)\n");
	for (i = 0; i < transfer->desc_num; i += 1)
		dump_desc(desc_virt + i);
}

/* xdma_desc_alloc() - Allocate cache-coherent array of N descriptors.
 *
 * Allocates an array of 'number' descriptors in contiguous PCI bus addressable
 * memory. Chains the descriptors as a singly-linked list; the descriptor's
 * next * pointer specifies the bus address of the next descriptor.
 *
 *
 * @dev Pointer to pci_dev
 * @number Number of descriptors to be allocated
 * @desc_bus_p Pointer where to store the first descriptor bus address
 * @desc_last_p Pointer where to store the last descriptor virtual address,
 * or NULL.
 *
 * @return Virtual address of the first descriptor
 *
 */
static struct xdma_desc *xdma_desc_alloc(struct pci_dev *dev, int number,
		dma_addr_t *desc_bus_p, struct xdma_desc **desc_last_p)
{
	struct xdma_desc *desc_virt;	/* virtual address */
	dma_addr_t desc_bus;		/* bus address */
	int i;
	int adj = number - 1;
	int extra_adj;
	u32 temp_control;

	BUG_ON(number < 1);

	/* allocate a set of cache-coherent contiguous pages */
	desc_virt = (struct xdma_desc *)pci_alloc_consistent(dev,
			number * sizeof(struct xdma_desc), desc_bus_p);
	if (!desc_virt)
		return NULL;
	/* get bus address of the first descriptor */
	desc_bus = *desc_bus_p;

	/* create singly-linked list for SG DMA controller */
	for (i = 0; i < number - 1; i++) {
		/* increment bus address to next in array */
		desc_bus += sizeof(struct xdma_desc);

		/* singly-linked list uses bus addresses */
		desc_virt[i].next_lo = cpu_to_le32(PCI_DMA_L(desc_bus));
		desc_virt[i].next_hi = cpu_to_le32(PCI_DMA_H(desc_bus));
		desc_virt[i].bytes = cpu_to_le32(0);

		/* any adjacent descriptors? */
		if (adj > 0) {
			extra_adj = adj - 1;
			if (extra_adj > MAX_EXTRA_ADJ)
				extra_adj = MAX_EXTRA_ADJ;

			adj--;
		} else {
			extra_adj = 0;
		}

		temp_control = DESC_MAGIC | (extra_adj << 8);

		temp_control |= INSERT_DESC_COUNT(i);

		desc_virt[i].control = cpu_to_le32(temp_control);
	}
	/* { i = number - 1 } */
	/* zero the last descriptor next pointer */
	desc_virt[i].next_lo = cpu_to_le32(0);
	desc_virt[i].next_hi = cpu_to_le32(0);
	desc_virt[i].bytes = cpu_to_le32(0);

	temp_control = DESC_MAGIC;

	temp_control |= INSERT_DESC_COUNT(i);

	desc_virt[i].control = cpu_to_le32(temp_control);

	/* caller wants a pointer to last descriptor? */
	if (desc_last_p)
		*desc_last_p = desc_virt + i;

	/* return the virtual address of the first descriptor */
	return desc_virt;
}

/* xdma_desc_link() - Link two descriptors
 *
 * Link the first descriptor to a second descriptor, or terminate the first.
 *
 * @first first descriptor
 * @second second descriptor, or NULL if first descriptor must be set as last.
 * @second_bus bus address of second descriptor
 */
static void xdma_desc_link(struct xdma_desc *first, struct xdma_desc *second,
		dma_addr_t second_bus)
{
	/*
	 * remember reserved control in first descriptor, but zero
	 * extra_adjacent!
	 */
	 /* RTO - what's this about?  Shouldn't it be 0x0000c0ffUL? */
	u32 control = le32_to_cpu(first->control) & 0x0000f0ffUL;
	/* second descriptor given? */
	if (second) {
		/*
		 * link last descriptor of 1st array to first descriptor of
		 * 2nd array
		 */
		first->next_lo = cpu_to_le32(PCI_DMA_L(second_bus));
		first->next_hi = cpu_to_le32(PCI_DMA_H(second_bus));
		WARN_ON(first->next_hi);
		/* no second descriptor given */
	} else {
		/* first descriptor is the last */
		first->next_lo = 0;
		first->next_hi = 0;
	}
	/* merge magic, extra_adjacent and control field */
	control |= DESC_MAGIC;

	/* write bytes and next_num */
	first->control = cpu_to_le32(control);
}

/* makes an existing transfer cyclic */
static void xdma_transfer_cyclic(struct xdma_transfer *transfer)
{
	/* link last descriptor to first descriptor */
	xdma_desc_link(transfer->desc_virt + transfer->desc_num - 1,
			transfer->desc_virt, transfer->desc_bus);
	/* remember transfer is cyclic */
	transfer->cyclic = 1;
}

/* xdma_desc_adjacent -- Set how many descriptors are adjacent to this one */
static void xdma_desc_adjacent(struct xdma_desc *desc, int next_adjacent)
{
	int extra_adj = 0;
	/* remember reserved and control bits */
	u32 control = le32_to_cpu(desc->control) & 0x0000f0ffUL;
	u32 max_adj_4k = 0;

	if (next_adjacent > 0) {
		extra_adj =  next_adjacent - 1;
		if (extra_adj > MAX_EXTRA_ADJ){
			extra_adj = MAX_EXTRA_ADJ;
		}
		max_adj_4k = (0x1000 - ((le32_to_cpu(desc->next_lo))&0xFFF))/32 - 1;
		if (extra_adj>max_adj_4k) {
			extra_adj = max_adj_4k;
		}
		if(extra_adj<0){
			printk("Warning: extra_adj<0, converting it to 0\n");
			extra_adj = 0;
		}
	}
	/* merge adjacent and control field */
	control |= 0xAD4B0000UL | (extra_adj << 8);
	/* write control and next_adjacent */
	desc->control = cpu_to_le32(control);
}

/* xdma_desc_control -- Set complete control field of a descriptor. */
static void xdma_desc_control(struct xdma_desc *first, u32 control_field)
{
	/* remember magic and adjacent number */
	u32 control = le32_to_cpu(first->control) & ~(LS_BYTE_MASK);

	BUG_ON(control_field & ~(LS_BYTE_MASK));
	/* merge adjacent and control field */
	control |= control_field;
	/* write control and next_adjacent */
	first->control = cpu_to_le32(control);
}

/* xdma_desc_clear -- Clear bits in control field of a descriptor. */
static void xdma_desc_control_clear(struct xdma_desc *first, u32 clear_mask)
{
	/* remember magic and adjacent number */
	u32 control = le32_to_cpu(first->control);

	BUG_ON(clear_mask & ~(LS_BYTE_MASK));
	/* merge adjacent and control field */
	control &= (~clear_mask);
	/* write control and next_adjacent */
	first->control = cpu_to_le32(control);
}

/* xdma_desc_clear -- Set bits in control field of a descriptor. */
static void xdma_desc_control_set(struct xdma_desc *first, u32 set_mask)
{
	/* remember magic and adjacent number */
	u32 control = le32_to_cpu(first->control);

	BUG_ON(set_mask & ~(LS_BYTE_MASK));
	/* merge adjacent and control field */
	control |= set_mask;
	/* write control and next_adjacent */
	first->control = cpu_to_le32(control);
}

/* xdma_desc_free - Free cache-coherent linked list of N descriptors.
 *
 * @dev Pointer to pci_dev
 * @number Number of descriptors to be allocated
 * @desc_virt Pointer to (i.e. virtual address of) first descriptor in list
 * @desc_bus Bus address of first descriptor in list
 */
static void xdma_desc_free(struct pci_dev *dev, int number,
		struct xdma_desc *desc_virt, dma_addr_t desc_bus)
{
	BUG_ON(!desc_virt);
	BUG_ON(number < 0);
	/* free contiguous list */
	pci_free_consistent(dev, number * sizeof(struct xdma_desc), desc_virt,
		desc_bus);
}

/* xdma_desc() - Fill a descriptor with the transfer details
 *
 * @desc pointer to descriptor to be filled
 * @addr root complex address
 * @ep_addr end point address
 * @len number of bytes, must be a (non-negative) multiple of 4.
 * @dir_to_dev If non-zero, source is root complex address and destination
 * is the end point address. If zero, vice versa.
 *
 * Does not modify the next pointer
 */
static void xdma_desc_set(struct xdma_desc *desc, dma_addr_t rc_bus_addr,
		u64 ep_addr, int len, int dir_to_dev)
{
	#if SD_ACCEL
	/* length (in bytes) must be a non-negative multiple of four */
	BUG_ON(len & 3);
	#endif

	/* transfer length */
	desc->bytes = cpu_to_le32(len);
	if (dir_to_dev) {
		/* read from root complex memory (source address) */
		desc->src_addr_lo = cpu_to_le32(PCI_DMA_L(rc_bus_addr));
		desc->src_addr_hi = cpu_to_le32(PCI_DMA_H(rc_bus_addr));
		/* write to end point address (destination address) */
		desc->dst_addr_lo = cpu_to_le32(PCI_DMA_L(ep_addr));
		desc->dst_addr_hi = cpu_to_le32(PCI_DMA_H(ep_addr));
	} else {
		/* read from end point address (source address) */
		desc->src_addr_lo = cpu_to_le32(PCI_DMA_L(ep_addr));
		desc->src_addr_hi = cpu_to_le32(PCI_DMA_H(ep_addr));
		/* write to root complex memory (destination address) */
		desc->dst_addr_lo = cpu_to_le32(PCI_DMA_L(rc_bus_addr));
		desc->dst_addr_hi = cpu_to_le32(PCI_DMA_H(rc_bus_addr));
	}
}

static void xdma_desc_set_source(struct xdma_desc *desc, u64 source)
{
	/* read from end point address (source address) */
	desc->src_addr_lo = cpu_to_le32(PCI_DMA_L(source));
	desc->src_addr_hi = cpu_to_le32(PCI_DMA_H(source));
}

static void transfer_set_result_addresses(struct xdma_transfer *transfer,
		u64 result_bus)
{
	int i;

	/* iterate over transfer descriptor list */
	for (i = 0; i < transfer->desc_num; i++) {
		/* set the result ptr in source */
		xdma_desc_set_source(transfer->desc_virt + i, result_bus);
		result_bus += sizeof(struct xdma_result);
	}
}

static void transfer_set_all_control(struct xdma_transfer *transfer,
		u32 control)
{
	int i;

	for (i = 0; i < transfer->desc_num; i++) {
		xdma_desc_control_clear(transfer->desc_virt + i, LS_BYTE_MASK);
		xdma_desc_control_set(transfer->desc_virt + i, control);
	}
}

#if CHAIN_MULTIPLE_TRANSFERS
static void chain_transfers(struct xdma_engine *engine,
	struct xdma_transfer *transfer)
{
	struct xdma_transfer *last;

	BUG_ON(!engine);
	BUG_ON(!transfer);

	/* queue is not empty? try to chain the descriptor lists */
	if (!list_empty(&engine->transfer_list)) {

		dbg_tfr("list not empty\n");
		/* get last transfer queued on the engine */
		last = list_entry(engine->transfer_list.prev,
					struct xdma_transfer, entry);
		/* @only when non-cyclic transfer */
		/* link the last transfer's last descriptor to this transfer */
		xdma_desc_link(last->desc_virt + last->desc_num - 1,
				   transfer->desc_virt, transfer->desc_bus);
		/* do not stop now that there is a linked transfers */
		xdma_desc_control_clear(last->desc_virt + last->desc_num - 1,
					XDMA_DESC_STOPPED);

		dbg_tfr("transfer=0x%p, desc=%d chained at 0x%p on engine %s\n",
			transfer, transfer->desc_num, last,
			engine->running ? "running" : "idle");

		/* queue is empty */
	} else {
		if (engine->running)
			dbg_tfr("queue empty, but engine is running!\n");
		else
			dbg_tfr("queue empty and engine idle\n");
		/* engine should not be running */
		WARN_ON(engine->running);
	}
}
#else
static void chain_transfers(struct xdma_engine *engine,
	struct xdma_transfer *transfer)
{
}
#endif

/* transfer_queue() - Queue a DMA transfer on the engine
 *
 * @engine DMA engine doing the transfer
 * @transfer DMA transfer submitted to the engine
 *
 * Takes and releases the engine spinlock
 */
static int transfer_queue(struct xdma_engine *engine,
		struct xdma_transfer *transfer)
{
	int rc = 0;
	struct xdma_transfer *transfer_started;

	BUG_ON(!engine);
	BUG_ON(!transfer);
	BUG_ON(transfer->desc_num == 0);
	dbg_tfr("transfer_queue(transfer=0x%p).\n", transfer);

	/* lock the engine state */
	spin_lock(&engine->lock);
	engine->prev_cpu = get_cpu();
	put_cpu();

	/* engine is being shutdown; do not accept new transfers */
	if (engine->shutdown & ENGINE_SHUTDOWN_REQUEST) {
		rc = -1;
		goto shutdown;
	}

	/*
	 * either the engine is still busy and we will end up in the
	 * service handler later, or the engine is idle and we have to
	 * start it with this transfer here
	 */
	chain_transfers(engine, transfer);

	/* mark the transfer as submitted */
	transfer->state = TRANSFER_STATE_SUBMITTED;
	/* add transfer to the tail of the engine transfer queue */
	list_add_tail(&transfer->entry, &engine->transfer_list);

	/* Prevent transfer from being kicked off to test bypass capability */
	/* engine is idle? */
	if (!engine->running) {
		/* start engine */
		dbg_tfr("transfer_queue(): starting %s engine.\n",
			engine->name);
		transfer_started = engine_start(engine);
		dbg_tfr("transfer=0x%p started %s engine with transfer 0x%p.\n",
			transfer, engine->name, transfer_started);
	} else {
		dbg_tfr("transfer=0x%p queued, with %s engine running.\n",
			transfer, engine->name);
	}

shutdown:
	/* unlock the engine state */
	dbg_tfr("engine->running = %d\n", engine->running);
	spin_unlock(&engine->lock);
	return rc;
};

#if SD_ACCEL
/* SD_Accel Specific */
void engine_reinit(const struct xdma_engine *engine)
{
	printk(KERN_INFO "%s: Re-init DMA Engine %s\n", DRV_NAME, engine->name);
	write_register(XDMA_CTRL_IE_DESC_STOPPED |
			XDMA_CTRL_IE_DESC_COMPLETED |
			XDMA_CTRL_IE_DESC_ALIGN_MISMATCH |
			XDMA_CTRL_IE_MAGIC_STOPPED |
			// Disable IDLE_STOPPED
			//XDMA_CTRL_IE_IDLE_STOPPED |
			XDMA_CTRL_IE_READ_ERROR |
			XDMA_CTRL_IE_DESC_ERROR,
			&engine->regs->interrupt_enable_mask);
}
#else
void engine_reinit(const struct xdma_engine *engine)
{
}
#endif

static void engine_alignments(struct xdma_engine *engine)
{
	u32 w;
	u32 align_bytes;
	u32 granularity_bytes;
	u32 address_bits;

	w = read_register(&engine->regs->alignments);
	dbg_init("engine %p name %s alignments=0x%08x\n", engine,
		engine->name, (int)w);

	/* RTO  - add some macros to extract these fields */
	align_bytes = (w & 0x00ff0000U) >> 16;
	granularity_bytes = (w & 0x0000ff00U) >> 8;
	address_bits = (w & 0x000000ffU);

	dbg_init("align_bytes = %d\n", align_bytes);
	dbg_init("granularity_bytes = %d\n", granularity_bytes);
	dbg_init("address_bits = %d\n", address_bits);

	if (w) {
		engine->addr_align = align_bytes;
		engine->len_granularity = granularity_bytes;
		engine->addr_bits = address_bits;
	} else {
		/* Some default values if alignments are unspecified */
		engine->addr_align = 1;
		engine->len_granularity = 1;
		engine->addr_bits = 64;
	}
}

static void engine_destroy(struct xdma_dev *lro, struct xdma_engine *engine)
{
	BUG_ON(!lro);
	BUG_ON(!engine);

	dbg_sg("Shutting down engine %s%d", engine->name, engine->channel);

	/* Disable interrupts to stop processing new events during shutdown */
	write_register(0x0, &engine->regs->interrupt_enable_mask);

	engine_msix_teardown(engine);

	/* Release memory use for descriptor writebacks */
	if (poll_mode)
		engine_writeback_teardown(engine);

	/* Release memory for the engine */
	kfree(engine);

	/* Decrement the number of engines available */
	lro->engines_num--;
}

static void engine_msix_teardown(struct xdma_engine *engine)
{
	BUG_ON(!engine);
	if (engine->msix_irq_line) {
		dbg_sg("Release IRQ#%d for engine %p\n", engine->msix_irq_line,
			engine);
		free_irq(engine->msix_irq_line, engine);
	}
}

static int engine_msix_setup(struct xdma_engine *engine)
{
	int rc = 0;
	u32 vector;
	struct xdma_dev *lro;

	BUG_ON(!engine);
	lro = engine->lro;
	BUG_ON(!lro);

	vector = lro->entry[lro->engines_num + MAX_USER_IRQ].vector;

	dbg_init("Requesting IRQ#%d for engine %p\n", vector, engine);
	rc = request_irq(vector, xdma_channel_irq, 0, DRV_NAME, engine);
	if (rc) {
		dbg_init("Unable to request_irq for engine %d\n",
			lro->engines_num);
	} else {
		dbg_init("Requested IRQ#%d for engine %d\n", vector,
			lro->engines_num);
		engine->msix_irq_line = vector;
	}

	return rc;
}

static void engine_writeback_teardown(struct xdma_engine *engine)
{
	struct xdma_dev *lro;

	BUG_ON(!engine);
	lro = engine->lro;
	BUG_ON(!lro);

	if (engine->poll_mode_addr_virt) {
		dbg_sg("Releasing memory for descriptor writeback\n");
		pci_free_consistent(lro->pci_dev, sizeof(struct xdma_poll_wb),
			engine->poll_mode_addr_virt, engine->poll_mode_bus);
		dbg_sg("Released memory for descriptor writeback\n");
	}
}

static int engine_writeback_setup(struct xdma_engine *engine)
{
	u32 w;
	struct xdma_dev *lro;
	struct xdma_poll_wb *writeback;

	BUG_ON(!engine);
	lro = engine->lro;
	BUG_ON(!lro);

	/*
	 * RTO - doing the allocation per engine is wasteful since a full page
	 * is allocated each time - better to allocate one page for the whole
	 * device during probe() and set per-engine offsets here
	 */

	/* Set up address for polled mode writeback */
	dbg_init("Allocating memory for descriptor writeback for %s%d",
		engine->name, engine->channel);
	engine->poll_mode_addr_virt = pci_alloc_consistent(lro->pci_dev,
		sizeof(struct xdma_poll_wb), &engine->poll_mode_bus);
	if (!engine->poll_mode_addr_virt) {
		dbg_init("engine %p (%s) couldn't allocate writeback\n", engine,
			engine->name);
		return -1;
	}
	dbg_init("Allocated memory for descriptor writeback for %s%d",
		engine->name, engine->channel);

	writeback = (struct xdma_poll_wb *)engine->poll_mode_addr_virt;
	writeback->completed_desc_count = 0;

	dbg_init("Setting writeback location to 0x%llx for engine %p",
		engine->poll_mode_bus, engine);
	w = cpu_to_le32(PCI_DMA_L(engine->poll_mode_bus));
	write_register(w, &engine->regs->poll_mode_wb_lo);
	w = cpu_to_le32(PCI_DMA_H(engine->poll_mode_bus));
	write_register(w, &engine->regs->poll_mode_wb_hi);

	return 0;
}

/* engine_create() - Create an SG DMA engine bookkeeping data structure
 *
 * An SG DMA engine consists of the resources for a single-direction transfer
 * queue; the SG DMA hardware, the software queue and interrupt handling.
 *
 * @dev Pointer to pci_dev
 * @offset byte address offset in BAR[lro->config_bar_idx] resource for the
 * SG DMA * controller registers.
 * @dir_to_dev Whether the engine transfers to the device (PCIe Rd).
 * @streaming Whether the engine is attached to AXI ST (rather than MM)
 */
static struct xdma_engine *engine_create(struct xdma_dev *lro, int offset,
		int dir_to_dev, int channel)
{
	u32 reg_value;
	int sgdma_offset = offset + SGDMA_OFFSET_FROM_CHANNEL;
	int rc;
	/* allocate data structure for engine book keeping */
	struct xdma_engine *engine;

	engine = kzalloc(sizeof(struct xdma_engine), GFP_KERNEL);

	/* memory allocation failure? */
	if (!engine)
		return NULL;

	/* set magic */
	engine->magic = MAGIC_ENGINE;

	/* indices */
	engine->channel = channel;
	engine->number_in_channel = !dir_to_dev;

	/* engine interrupt request bit */
	engine->irq_bitmask = (1 << XDMA_ENG_IRQ_NUM) - 1;
	engine->irq_bitmask <<= (lro->engines_num * XDMA_ENG_IRQ_NUM);
	engine->bypass_offset = lro->engines_num * BYPASS_MODE_SPACING;

	/* initialize spinlock */
	spin_lock_init(&engine->lock);
	/* initialize transfer_list */
	INIT_LIST_HEAD(&engine->transfer_list);
	/* parent */
	engine->lro = lro;
	/* register address */
	engine->regs = (lro->bar[lro->config_bar_idx] + offset);
	engine->sgdma_regs = (lro->bar[lro->config_bar_idx] + sgdma_offset);
	/* remember SG DMA direction */
	engine->dir_to_dev = dir_to_dev;
	engine->name = engine->dir_to_dev ? "H2C" : "C2H";
	engine->streaming = get_engine_type(engine->regs);

	dbg_init("engine %p name %s irq_bitmask=0x%08x\n", engine, engine->name,
		(int)engine->irq_bitmask);

	/* initialize the deferred work for transfer completion */
	INIT_WORK(&engine->work, engine_service_work);

	/* Configure per-engine MSI-X vector if MSI-X is enabled */
	if (lro->msix_enabled) {
		rc = engine_msix_setup(engine);
		if (rc) {
			dbg_init("MSI-X config for engine %p failed\n", engine);
			goto fail_msix;
		}
	}

	lro->engines_num++;

	/* initialize wait queue */
	init_waitqueue_head(&engine->shutdown_wq);
	/* initialize wait queue */
	init_waitqueue_head(&engine->xdma_perf_wq);

	write_register(XDMA_CTRL_NON_INCR_ADDR, &engine->regs->control_w1c);

	engine_alignments(engine);

	/* Configure error interrupts by default */
	reg_value = XDMA_CTRL_IE_DESC_ALIGN_MISMATCH;
	reg_value |= XDMA_CTRL_IE_MAGIC_STOPPED;
	reg_value |= XDMA_CTRL_IE_MAGIC_STOPPED;
	reg_value |= XDMA_CTRL_IE_READ_ERROR;
	reg_value |= XDMA_CTRL_IE_DESC_ERROR;

	/* If using polled mode, configure writeback address */
	if (poll_mode) {
		rc = engine_writeback_setup(engine);
		if (rc) {
			dbg_init("Descriptor writeback setup failed for %p\n",
				engine);
			goto fail_wb;
		}
	} else {
		/* Otherwise, enable the relevant completion interrupts */
		reg_value |= XDMA_CTRL_IE_DESC_STOPPED;
		reg_value |= XDMA_CTRL_IE_DESC_COMPLETED;

		/* If using AXI ST, also enable the IDLE_STOPPED interrupt */
		if (engine->streaming && !dir_to_dev)
			reg_value |= XDMA_CTRL_IE_IDLE_STOPPED;
	}

	/* Apply engine configurations */
	write_register(reg_value, &engine->regs->interrupt_enable_mask);

	if (engine->rx_transfer_cyclic)
		transfer_queue(engine, engine->rx_transfer_cyclic);

	/* all engine setup completed successfully */
	goto success;

fail_wb:
	engine_msix_teardown(engine);
fail_msix:
	kfree(engine);
	engine = NULL;

success:
	return engine;
}

/* transfer_destroy() - free transfer */
static void transfer_destroy(struct xdma_dev *lro,
		struct xdma_transfer *transfer)
{
	/* user space buffer was locked in on account of transfer? */
	if (transfer->sgm) {
		/* unmap scatterlist */
		/* the direction is needed to synchronize caches */
		pci_unmap_sg(lro->pci_dev, transfer->sgm->sgl,
				transfer->sgm->mapped_pages,
				transfer->dir_to_dev ?
				DMA_TO_DEVICE : DMA_FROM_DEVICE);
		if (transfer->userspace) {
			/* dirty and unlock the pages */
			sgm_put_user_pages(transfer->sgm,
				transfer->dir_to_dev ? 0 : 1);
		}
		transfer->sgm->mapped_pages = 0;
		sg_destroy_mapper(transfer->sgm);
	}

	/* free descriptors */
	xdma_desc_free(lro->pci_dev, transfer->sgl_nents, transfer->desc_virt,
			transfer->desc_bus);
	/* free transfer */
	kfree(transfer);
}

/* SD_Accel Specific */
#if SD_ACCEL && 0
#define CONFIG_WDMA_256 (1 << 4)
#define CONFIG_WDMA_128 (1 << 3)
#define CONFIG_WDMA_64 (1 << 2)
#define CONFIG_WDMA_32 (1 << 1)
#define CONFIG_WDMA_EN (1 << 0)

#define CONFIG_RDMA_256 (1 << 4)
#define CONFIG_RDMA_128 (1 << 3)
#define CONFIG_RDMA_64 (1 << 2)
#define CONFIG_RDMA_32 (1 << 1)
#define CONFIG_RDMA_EN (1 << 0)

/* xdma_config -- Inspect the XDMA IP Core configuration */
static int xdma_config(struct xdma_dev *lro, unsigned int config_offset)
{
	void *reg = lro->bar[lro->config_bar_idx] + config_offset;
	u32 w, payload, maxread;
	int rc = 0;
	int version = 0;

	/* read identifier of the configuration inspector */
	w = read_register(reg + 0x00);
	/* read version of the configuration inspector */
	version = w & 0x000000ffUL;
	printk(KERN_ERR "Configuration Inspector identifier is 0x%08x.\n", w);
	/* configuration inspector not found? */
	if ((w & 0xffffff00UL) != 0x00b20000UL) {
		printk(KERN_ERR "Configuration Inspector identifier not found (found 0x%08x expected 0x00b20001).\n", w);
		rc = -1;
		goto fail_identifier;
	}

	payload = read_register(reg + 0x08);
	maxread = read_register(reg + 0x0c);

	/* read XDMA System identifier */
	w = read_register(reg + 0x10) & 0x0000ffffUL;
	/* XDMA Target Bridge */
	if (w == 0xFF01UL)
		printk(KERN_DEBUG "XDMA Target Bridge\n");
	/* XDMA XDMA */
	else if (w == 0xFF02UL) {
		/* versions lower than 2 had a fixed configuration */
		if (version < 2) {
			/* read and write engines always present */
			lro->capabilities |= CAP_ENGINE_WRITE | CAP_ENGINE_READ;
			lro->align[/*dir_to_dev=*/0] = lro->align[1] = 8;
		/* engine capabilities in the inspector since version 2 */
		} else {
			/* inspect write engine capabilities */
			w = read_register(reg + 0x1c);
			printk(KERN_DEBUG "WDMA = 0x%08x\n", w);
			if (w & CONFIG_WDMA_EN) {
				lro->capabilities |= CAP_ENGINE_WRITE;
				/* determine address and size alignment for write DMA */
				if (w & CONFIG_WDMA_32) lro->align[/*dir_to_dev=*/0] = 4;
				else if (w & CONFIG_WDMA_128) lro->align[/*dir_to_dev=*/0] = 16;
				else if (w & CONFIG_WDMA_256) lro->align[/*dir_to_dev=*/0] = 32;
				else lro->align[/*dir_to_dev=*/0] = 8;
				printk(KERN_DEBUG "Write Engine requires %d byte alignment.\n", lro->align[/*dir_to_dev=*/0]);
			}
			/* inspect read engine capabilities */
			w = read_register(reg + 0x20);
			printk(KERN_DEBUG "RDMA = 0x%08x\n", w);
			if (w & CONFIG_RDMA_EN) {
				lro->capabilities |= CAP_ENGINE_READ;
				/* determine address and size alignment for read DMA */
				if (w & CONFIG_RDMA_32) lro->align[/*dir_to_dev=*/1] = 4;
				else if (w & CONFIG_RDMA_128) lro->align[/*dir_to_dev=*/1] = 16;
				else if (w & CONFIG_RDMA_256) lro->align[/*dir_to_dev=*/1] = 32;
				else lro->align[/*dir_to_dev=*/1] = 8;
				printk(KERN_DEBUG "Read Engine requires %d byte alignment.\n", lro->align[/*dir_to_dev=*/0]);
			}
		}
		/* any engine present? */
		if (lro->capabilities & (CAP_ENGINE_READ | CAP_ENGINE_WRITE)) {
			printk(KERN_DEBUG "XDMA Scatter-Gather%s%s\n",
				(lro->capabilities & CAP_ENGINE_READ)?" Read":"",
				(lro->capabilities & CAP_ENGINE_WRITE)?" Write":"");
		} else {
			printk(KERN_DEBUG "XDMA Target Bridge (Scatter-Gather Engines disabled in SOPC).\n");
		}
	}
	else
		printk(KERN_DEBUG "XDMA System ID = 0x%04x.\n", w);
	w = read_register(reg + 0x04);
	/* bus, device and function */
	printk(KERN_DEBUG "bus:dev.fn = %02x:%02x.%1x, payload = %d bytes, maxread = %d bytes\n",
		(w >> 8) & 0x0f/*bus*/, (w >> 3) & 0x1f/* device*/, w & 0x07/*function*/,
		(unsigned int)payload, (unsigned int)maxread);
fail_identifier:
	return rc;
}
#endif

#if FORCE_IR_DESC_COMPLETED
static inline void xdma_desc_force_complete(struct xdma_desc *transfer)
{
	xdma_desc_control(transfer, XDMA_DESC_COMPLETED);
}
#else
static inline void xdma_desc_force_complete(struct xdma_desc *transfer)
{
}
#endif

static void transfer_perf(struct xdma_transfer *transfer, int last)
{
	u32 control;

	BUG_ON(!transfer);

	#ifdef XDMA_PERFORMANCE_TEST
	/* create a linked loop */
	xdma_desc_link(transfer->desc_virt + last, transfer->desc_virt,
		transfer->desc_bus);

	/* request IRQ on last descriptor */
	xdma_desc_force_complete(transfer->desc_virt + last);
	#else
		/* terminate last descriptor */
		xdma_desc_link(transfer->desc_virt + last, 0, 0);
		/* stop engine, EOP for AXI ST, req IRQ on last descriptor */
		control = XDMA_DESC_STOPPED;
		control |= XDMA_DESC_EOP;
		control |= XDMA_DESC_COMPLETED;
		xdma_desc_control(transfer->desc_virt + last, control);
	#endif
}

static int transfer_build(struct xdma_transfer *transfer, u64 ep_addr,
		int dir_to_dev, int non_incr_addr, int force_new_desc,
		int userspace)
{
	int i = 0;
	int j = 0;
	int new_desc;
	dma_addr_t cont_addr;
	dma_addr_t addr;
	unsigned int cont_len;
	unsigned int len;
	unsigned int cont_max_len = 0;
	struct scatterlist *sgl;

	BUG_ON(!transfer);

	sgl = transfer->sgm->sgl;

	/* start first contiguous block */
	cont_addr = addr = sg_dma_address(&transfer->sgm->sgl[i]);
	cont_len = 0;

	/* iterate over all remaining entries but the last */
	for (i = 0; i < transfer->sgl_nents - 1; i++) {
		/* bus address of next entry i + 1 */
		dma_addr_t next = sg_dma_address(&sgl[i + 1]);
		/* length of this entry i */
		len = sg_dma_len(&sgl[i]);
		dbg_desc("SGLE %04d: addr=0x%016llx length=0x%08x\n", i,
			(u64)addr, len);

		/* add entry i to current contiguous block length */
		cont_len += len;

		new_desc = 0;
		/* entry i + 1 is non-contiguous with entry i? */
		if (next != addr + len) {
			dbg_desc("NON-CONTIGUOUS WITH DESC %d\n", i + 1);
			new_desc = 1;
		}
		/* entry i reached maximum transfer size? */
		else if (cont_len > (XDMA_DESC_MAX_BYTES - PAGE_SIZE)) {
			dbg_desc("BREAK\n");
			new_desc = 1;
		}

		if ((force_new_desc) && !(userspace))
			new_desc = 1;

		if (new_desc) {
			/* fill in descriptor entry j with transfer details */
			xdma_desc_set(transfer->desc_virt + j, cont_addr,
					ep_addr, cont_len, dir_to_dev);

			xdma_desc_force_complete(transfer->desc_virt + j);

			if (cont_len > cont_max_len)
				cont_max_len = cont_len;

			dbg_desc("DESC %4d:cont_addr=0x%llx\n", j,
				(u64)cont_addr);
			dbg_desc("DESC %4d:cont_len=0x%08x\n", j, cont_len);
			dbg_desc("DESC %4d:ep_addr=0x%llx\n", j, (u64)ep_addr);
			/* proceed EP address for next contiguous block */

			/* for non-inc-add mode don't increment ep_addr */
			if (userspace) {
				if (non_incr_addr == 0)
					ep_addr += cont_len;
			} else {
				ep_addr += cont_len;
			}

			/* start new contiguous block */
			cont_addr = next;
			cont_len = 0;
			j++;
		}
		/* goto entry i + 1 */
		addr = next;
	}
	/* i is the last entry in the scatterlist, add it to the last block */
	len = sg_dma_len(&sgl[i]);
	cont_len += len;
	BUG_ON(j > transfer->sgl_nents);

	/* j is the index of the last descriptor */

	dbg_desc("SGLE %4d: addr=0x%016llx length=0x%08x\n", i, (u64)addr, len);
	dbg_desc("DESC %4d: cont_addr=0x%llx cont_len=0x%08x ep_addr=0x%llx\n",
		j, (u64)cont_addr, cont_len, (unsigned long long)ep_addr);

	/* XXX to test error condition, set cont_len = 0 */

	/* fill in last descriptor entry j with transfer details */
	xdma_desc_set(transfer->desc_virt + j, cont_addr, ep_addr, cont_len,
		dir_to_dev);

	return j;
}

static struct xdma_transfer *transfer_create(struct xdma_dev *lro,
		const char *start, size_t cnt, u64 ep_addr, int dir_to_dev,
		int non_incr_addr, int force_new_desc, int userspace)
{
	int i = 0;
	int last = 0;
	int rc;
	struct scatterlist *sgl;
	struct xdma_transfer *transfer;
	u32 control;

	/* allocate transfer data structure */
	transfer = kzalloc(sizeof(struct xdma_transfer), GFP_KERNEL);

	dbg_sg("transfer_create()\n");

	if (!transfer)
		return NULL;

	/* remember direction of transfer */
	transfer->dir_to_dev = dir_to_dev;

	/* create virtual memory mapper */
	transfer->sgm = sg_create_mapper(cnt);
	BUG_ON(!transfer->sgm);
	transfer->userspace = userspace;

	/* lock user pages in memory and create a scatter gather list */
	if (userspace)
		rc = sgm_get_user_pages(transfer->sgm, start, cnt, !dir_to_dev);
	else
		rc = sgm_kernel_pages(transfer->sgm, start, cnt, !dir_to_dev);

	BUG_ON(rc < 0);

	sgl = transfer->sgm->sgl;

	dbg_sg("mapped_pages=%d.\n", transfer->sgm->mapped_pages);
	dbg_sg("sgl = 0x%p.\n", transfer->sgm->sgl);
	BUG_ON(!lro->pci_dev);
	BUG_ON(!transfer->sgm->sgl);
	BUG_ON(!transfer->sgm->mapped_pages);
	/* map all SG entries into DMA memory */
	transfer->sgl_nents = pci_map_sg(lro->pci_dev, transfer->sgm->sgl,
		transfer->sgm->mapped_pages,
		dir_to_dev ? DMA_TO_DEVICE : DMA_FROM_DEVICE);
	dbg_sg("hwnents=%d.\n", transfer->sgl_nents);

	/* verify if the page start address got into the first sg entry */
	dbg_sg("sg_page(&sgl[0])=0x%p.\n", sg_page(&transfer->sgm->sgl[0]));
	dbg_sg("sg_dma_address(&sgl[0])=0x%016llx.\n",
		(u64)sg_dma_address(&transfer->sgm->sgl[0]));
	dbg_sg("sg_dma_len(&sgl[0])=0x%08x.\n",
		sg_dma_len(&transfer->sgm->sgl[0]));

	/* allocate descriptor list */
	transfer->desc_virt = xdma_desc_alloc(lro->pci_dev, transfer->sgl_nents,
		&transfer->desc_bus, NULL);
	dbg_sg("transfer_create():\n");
	dbg_sg("transfer->desc_bus = 0x%llx.\n", (u64)transfer->desc_bus);

	last = transfer_build(transfer, ep_addr, dir_to_dev, non_incr_addr,
		force_new_desc, userspace);

	if (userspace) {
		transfer_perf(transfer, last);
	} else {
		/* terminate last descriptor */
		xdma_desc_link(transfer->desc_virt + last, 0, 0);
		/* stop engine, EOP for AXI ST, req IRQ on last descriptor */
		control = XDMA_DESC_STOPPED;
		control |= XDMA_DESC_EOP;
		control |= XDMA_DESC_COMPLETED;
		xdma_desc_control(transfer->desc_virt + last, control);
	}

	last++;
	/* last is the number of descriptors */
	transfer->desc_num = transfer->desc_adjacent = last;

	dbg_sg("transfer 0x%p has %d descriptors\n", transfer,
		transfer->desc_num);
	/* fill in adjacent numbers */
	for (i = 0; i < transfer->desc_num; i++) {
		xdma_desc_adjacent(transfer->desc_virt + i,
			transfer->desc_num - i - 1);
	}

	/* initialize wait queue */
	init_waitqueue_head(&transfer->wq);

	return transfer;
}

static int check_transfer_align(struct xdma_engine *engine,
	const char __user *buf, size_t count, loff_t pos, int sync)
{
	BUG_ON(!engine);

	/* AXI ST or AXI MM non-incremental addressing mode? */
	if (engine->streaming || engine->non_incr_addr) {
		int buf_lsb = (int)((uintptr_t)buf) & (engine->addr_align - 1);
		size_t len_lsb = count & ((size_t)engine->len_granularity - 1);
		int pos_lsb = (int)pos & (engine->addr_align - 1);

		dbg_tfr("AXI ST or MM non-incremental\n");
		dbg_tfr("buf_lsb = %d, pos_lsb = %d, len_lsb = %ld\n", buf_lsb,
			pos_lsb, len_lsb);

		if (buf_lsb != 0) {
			dbg_tfr("FAIL: non-aligned buffer address %p\n", buf);
			return -EINVAL;
		}

		if ((!engine->streaming) && (pos_lsb != 0) && (sync)) {
			dbg_tfr("FAIL: non-aligned AXI MM FPGA addr 0x%llx\n",
				(unsigned long long)pos);
			return -EINVAL;
		}

		if (len_lsb != 0) {
			dbg_tfr("FAIL: len %d is not a multiple of %d\n",
				(int)count,
				(int)engine->len_granularity);
			return -EINVAL;
		}
		/* AXI MM incremental addressing mode */
	} else {
		int buf_lsb = (int)((uintptr_t)buf) & (engine->addr_align - 1);
		int pos_lsb = (int)pos & (engine->addr_align - 1);

		if (buf_lsb != pos_lsb) {
			dbg_tfr("FAIL: Misalignment error\n");
			dbg_tfr("host addr %p, FPGA addr 0x%llx\n", buf, pos);
			return -EINVAL;
		}
	}

	return 0;
}

/* sg_aio_read_write() -- Read from or write to the device
 *
 * @buf userspace buffer
 * @count number of bytes in the userspace buffer
 * @pos byte-address in device
 * @dir_to_device If !0, a write to the device is performed
 *
 * Iterate over the userspace buffer, taking at most 255 * PAGE_SIZE bytes for
 * each DMA transfer.
 *
 * For each transfer, get the user pages, build a sglist, map, build a
 * descriptor table. submit the transfer. wait for the interrupt handler
 * to wake us on completion.
 */
static ssize_t sg_aio_read_write(struct kiocb *iocb, const struct iovec *iov,
		unsigned long nr_segs, loff_t pos, int dir_to_dev)
{
	/* fetch file this io request acts on */
	struct file *file = iocb->ki_filp;
	size_t total_done = 0;
	unsigned long seg;
	struct xdma_char *lro_char;
	struct xdma_dev *lro;
	struct xdma_engine *engine;
	int rc = 0;

	/* fetch device specific data stored earlier during open */
	lro_char = (struct xdma_char *)file->private_data;
	BUG_ON(!lro_char);
	BUG_ON(lro_char->magic != MAGIC_CHAR);

	lro = lro_char->lro;
	BUG_ON(!lro);
	BUG_ON(lro->magic != MAGIC_DEVICE);

	engine = lro_char->engine;
	BUG_ON(!engine);
	BUG_ON(engine->magic != MAGIC_ENGINE);

	dbg_tfr("iocb=0x%p, iov=0x%p, nr_segs=%ld, pos=%llu\n", iocb, iov,
		nr_segs, (u64)pos);
	dbg_tfr("dir_to_dev=%d %s request\n",  dir_to_dev,
		dir_to_dev ? "write" : "read");

	/* iterate over all vector segments */
	for (seg = 0; seg < nr_segs; seg++) {
		const char __user *buf = iov[seg].iov_base;
		size_t count = iov[seg].iov_len;
		size_t remaining = count;
		size_t done = 0;
		char *transfer_addr = (char *)buf;

		rc = check_transfer_align(engine, buf, count, pos, 0);
		if (rc) {
			dbg_tfr("Invalid transfer alignment detected\n");
			return rc;
		}

		dbg_tfr("seg %lu: buf=0x%p, count=%lld, pos=%llu\n", seg, buf,
				(s64)count, (u64)pos);
		/* anything left to transfer? */
		while (remaining > 0) {
			struct xdma_transfer *transfer;
			/* DMA transfer size, multiple if necessary */
			size_t transfer_len;

			if (remaining > XDMA_TRANSFER_MAX_BYTES)
				transfer_len = XDMA_TRANSFER_MAX_BYTES;
			else
				transfer_len = remaining;

			/* build device-specific descriptor tables */
			transfer = transfer_create(lro, transfer_addr,
				transfer_len, pos, dir_to_dev, 0, 0, 1);
			dbg_sg("segment:%lu transfer=0x%p.\n", seg, transfer);
			BUG_ON(!transfer);

			if (!transfer) {
				dbg_tfr("Couldn't allocate memory for xfer!");
				return -ENOMEM;
			}

			transfer_dump(transfer);

			/* remember I/O context for later completion */
			transfer->iocb = iocb;
			/* last transfer for the given request? */
			if (transfer_len >= remaining) {
				/* mark as last transfer, using request size */
				transfer->last_in_request = 1;
				transfer->size_of_request = done + transfer_len;
			}
			/* queue the transfer on the hardware */
			transfer_queue(engine, transfer);
			/* calculate the next transfer */
			transfer_addr += transfer_len;
			remaining -= transfer_len;
			done += transfer_len;
			dbg_tfr("remaining = %lld, done = %lld.\n",
					(s64)remaining, (s64)done);
		}
		total_done += done;
	}
	dbg_tfr("queued a total of %lld bytes, returns -EIOCBQUEUED.\n",
		(s64)total_done);
	return -EIOCBQUEUED;
}

/**
* sg_aio_read_write - generic asynchronous read routine
* @iocb:       kernel I/O control block
* @iov:	io vector request
* @nr_segs:    number of segments in the iovec
* @pos:	current file position
*
*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,15,0)
static ssize_t sg_read_iter (struct kiocb *iocb, struct iov_iter *from)
{
	dbg_sg("%s()\n", __func__);
	return sg_aio_read_write(iocb, from->iov, from->nr_segs,
				iocb->ki_pos, 0);
}

static ssize_t sg_write_iter (struct kiocb *iocb, struct iov_iter *iter)
{
	dbg_sg("%s()\n", __func__);
	return sg_aio_read_write(iocb, iter->iov, iter->nr_segs,
				iocb->ki_pos, 1);
}

#else
static ssize_t sg_aio_read(struct kiocb *iocb, const struct iovec *iov,
		unsigned long nr_segs, loff_t pos)
{
	dbg_sg("sg_aio_read()\n");
	return sg_aio_read_write(iocb, iov, nr_segs, pos, 0);
}

static ssize_t sg_aio_write(struct kiocb *iocb, const struct iovec *iov,
		unsigned long nr_segs, loff_t pos)
{
	dbg_sg("sg_aio_write()\n");
	return sg_aio_read_write(iocb, iov, nr_segs, pos, 1);
}
#endif

static loff_t char_sgdma_llseek(struct file *file, loff_t off, int whence)
{
	loff_t newpos = 0;

	switch (whence) {
	case 0:	/* SEEK_SET */
		newpos = off;
		break;
	case 1:	/* SEEK_CUR */
		newpos = file->f_pos + off;
		break;
	case 2:	/* SEEK_END, @TODO should work from end of address space */
		newpos = UINT_MAX + off;
		break;
	default: /* can't happen */
		return -EINVAL;
	}
	if (newpos < 0)
		return -EINVAL;
	file->f_pos = newpos;
	dbg_fops("char_sgdma_llseek: pos=%lld\n", (signed long long)newpos);
	return newpos;
}

static int transfer_monitor(struct xdma_engine *engine,
	struct xdma_transfer *transfer)
{
	u32 desc_count;
	int rc;

	/*
	 * When polling, determine how many descriptors have been queued
	 * on the engine to determine the writeback value expected
	 */
	if (poll_mode) {
		spin_lock(&engine->lock);
		desc_count = transfer->desc_num;
		spin_unlock(&engine->lock);

		dbg_tfr("desc_count=%d\n", desc_count);
		dbg_tfr("starting polling\n");

		rc = engine_service_poll(engine, desc_count);
		dbg_tfr("engine_service_poll()=%d\n", rc);
	} else {
		/* the function servicing the engine will wake us */
		rc = wait_event_interruptible(transfer->wq,
			transfer->state != TRANSFER_STATE_SUBMITTED);
		if (rc)
			dbg_tfr("wait_event_interruptible=%d\n", rc);
	}

	return rc;
}

static ssize_t transfer_data(struct xdma_engine *engine, char *transfer_addr,
		ssize_t remaining, loff_t *pos, int seq)
{
	int rc;
	ssize_t res = 0;
	ssize_t done = 0;
	struct xdma_dev *lro;
	struct xdma_transfer *transfer;
	size_t transfer_len;

	BUG_ON(!engine);
	lro = engine->lro;
	BUG_ON(!lro);

	/* still good and anything left to transfer? */
	while ((res == 0) && (remaining > 0)) {
		/* DMA transfer size, multiple if necessary */
		if (remaining > XDMA_TRANSFER_MAX_BYTES)
			transfer_len = XDMA_TRANSFER_MAX_BYTES;
		else
			transfer_len = remaining;

		/* build device-specific descriptor tables */
		transfer = transfer_create(lro, transfer_addr, transfer_len,
			*pos, engine->dir_to_dev, engine->non_incr_addr, 0, 1);
		dbg_tfr("seq:%d transfer=0x%p.\n", seq, transfer);

		if (!transfer) {
			remaining = 0;
			res = -EIO;
			break;
		}

		transfer_dump(transfer);

		/* last transfer for the given request? */
		if (transfer_len >= remaining) {
			transfer->last_in_request = 1;
			transfer->size_of_request = done + transfer_len;
		}

		/* let the device read from the host */
		transfer_queue(engine, transfer);

		rc = transfer_monitor(engine, transfer);

		/* transfer was taken off the engine? */
		if (transfer->state != TRANSFER_STATE_SUBMITTED) {
			/* transfer failed? */
			if (transfer->state != TRANSFER_STATE_COMPLETED) {
				dbg_tfr("transfer %p failed\n", transfer);
				res = -EIO;
			}
			dbg_tfr("transfer %p completed\n", transfer);
			transfer_destroy(lro, transfer);
			/* interrupted by a signal / polling detected error */
		} else if (rc != 0) {
			/* transfer can still be in-flight */
			engine_status_read(engine, 0);
			read_interrupts(lro);

			res = -ERESTARTSYS;
		}

		/* If an error has occurred, clear counts tracking progress */
		if (res != 0) {
			transfer_len = 0;
			remaining = 0;
		}

		/* calculate the next transfer */
		transfer_addr += transfer_len;
		remaining -= transfer_len;
		done += transfer_len;
		*pos += transfer_len;
		dbg_tfr("remain=%lld, done=%lld\n", (s64)remaining, (s64)done);
	}
	/* return error or else number of bytes */
	res = res ? res : done;

	return res;
}

/* char_sgdma_read_write() -- Read from or write to the device
 *
 * @buf userspace buffer
 * @count number of bytes in the userspace buffer
 * @pos byte-address in device
 * @dir_to_device If !0, a write to the device is performed
 *
 * Iterate over the userspace buffer, taking at most 255 * PAGE_SIZE bytes for
 * each DMA transfer.
 *
 * For each transfer, get the user pages, build a sglist, map, build a
 * descriptor table. submit the transfer. wait for the interrupt handler
 * to wake us on completion.
 */


static ssize_t char_sgdma_read_write(struct file *file, char __user *buf,
		size_t count, loff_t *pos, int dir_to_dev)
{
	int rc;
	ssize_t res = 0;
	static int counter;
	int seq = counter++;
	struct xdma_char *lro_char;
	struct xdma_dev *lro;
	struct xdma_engine *engine;

	/* fetch device specific data stored earlier during open */
	lro_char = (struct xdma_char *)file->private_data;
	BUG_ON(!lro_char);
	BUG_ON(lro_char->magic != MAGIC_CHAR);

	lro = lro_char->lro;
	BUG_ON(!lro);
	BUG_ON(lro->magic != MAGIC_DEVICE);

	engine = lro_char->engine;
	/* XXX detect non-supported directions XXX */
	BUG_ON(!engine);
	BUG_ON(engine->magic != MAGIC_ENGINE);

	dbg_tfr("seq:%d file=0x%p, buf=0x%p, count=%lld, pos=%llu\n", seq,
		file, buf, (s64)count, (u64)*pos);

	dbg_tfr("seq:%d dir_to_dev=%d %s request\n", seq, dir_to_dev,
		dir_to_dev ? "write" : "read");

	dbg_tfr("%s engine channel %d (engine num %d)= 0x%p\n", engine->name,
		engine->channel, engine->number_in_channel, engine);
	dbg_tfr("lro = 0x%p\n", lro);

	/* data direction does not match engine? */
	if (dir_to_dev != engine->dir_to_dev) {
		if (dir_to_dev)
			dbg_tfr("FAILURE: Cannot write to C2H engine.\n");
		else
			dbg_tfr("FAILURE: Cannot read from H2C engine.\n");

		return -EINVAL;
	}

	rc = check_transfer_align(engine, buf, count, *pos, 1);
	if (rc) {
		dbg_tfr("Invalid transfer alignment detected\n");
		return rc;
	}

	dbg_tfr("res = %ld, remaining = %ld\n", res, count);

	res = transfer_data(engine, (char *)buf, count, pos, seq);
	dbg_tfr("seq:%d char_sgdma_read_write() return=%lld.\n", seq, (s64)res);

	interrupt_status(lro);

	return res;
}

static int transfer_monitor_cyclic(struct xdma_engine *engine,
	struct xdma_transfer *transfer)
{
	struct xdma_result *result;
	int rc = 0;

	BUG_ON(!engine);
	BUG_ON(!transfer);

	result = (struct xdma_result *)engine->rx_result_buffer_virt;
	BUG_ON(!result);

	do
	{
		rc++;
		if(rc==100){
			break;
		}
		if (poll_mode) {
			rc = engine_service_poll(engine, 0);
			if (rc) {
				dbg_tfr("engine_service_poll() = %d\n", rc);
				rc = -ERESTARTSYS;
				//break;
			}
		} else {
			if(enable_credit_mp){
				printk("rx_head=%d,rx_tail=%d\n",engine->rx_head,engine->rx_tail);
				rc = wait_event_interruptible(transfer->wq,
						(engine->rx_head!=engine->rx_tail||engine->rx_overrun));
			}else{
				rc = wait_event_interruptible(transfer->wq,
						engine->eop_found);
			}
			if (rc) {
				dbg_tfr("wait_event_interruptible()=%d\n", rc);
				//break;
			}
		}
	} while (result[engine->rx_head].status == 0);

	return rc;
}

static int copy_cyclic_to_user(struct xdma_engine *engine, int pkt_length,
	int head, char __user *buf)
{
	int remaining = pkt_length;
	int copy;
	//int done = 0;
	int rc;
	char *rx_buffer;

	BUG_ON(!engine);
	BUG_ON(!buf);

	rx_buffer = engine->rx_buffer;

	/* EOP found? Transfer anything from head to EOP */
	while (remaining) {
		copy = remaining > RX_BUF_BLOCK ? RX_BUF_BLOCK : remaining;

		dbg_tfr("head = %d, copy %d bytes from %p to %p\n", head,  copy,
			&rx_buffer[head * RX_BUF_BLOCK], &buf[engine->user_buffer_index]);
		rc = copy_to_user(&buf[engine->user_buffer_index], &rx_buffer[head * RX_BUF_BLOCK],
			copy);
		if (rc) {
			dbg_tfr("copy_to_user failed\n");
			return rc;
		}

		remaining -= copy;
		engine->user_buffer_index += copy;
		head = (head + 1) % RX_BUF_PAGES;
	}

	return pkt_length;
}

static int complete_cyclic(struct xdma_engine *engine, char __user *buf)
{
	struct xdma_result *result;
	int pkt_length = 0;
	int fault = 0;
	int eop = 0;
	int head;
	int rc = 0;
	int num_credit = 0;

	BUG_ON(!engine);
	result = (struct xdma_result *)engine->rx_result_buffer_virt;
	BUG_ON(!result);

	spin_lock(&engine->lock);

	/* where the host currently is in the ring buffer */
	head = engine->rx_head;

	/* iterate over newly received results */
	//while (result[engine->rx_head].status) {
	while (engine->rx_head != engine->rx_tail||engine->rx_overrun) {
		WARN_ON(result[engine->rx_head].status==0);
		dbg_tfr("result[engine->rx_head=%3d].status = 0x%08x\n",
			engine->rx_head, (int)result[engine->rx_head].status);

		dbg_tfr("result[engine->rx_head=%3d].length = %d\n",
			engine->rx_head, (int)result[engine->rx_head].length);

		/* overrun is if tail is moved and matches head, not here */
		//if (engine->rx_head == engine->rx_tail) {
			//if(!engine->rx_overrun){
				//dbg_tfr("engine->rx_head==engine->rx_tail\n");
				//break;
			//}
		//} else if ((result[engine->rx_head].status >> 16) != C2H_WB) {
		if ((result[engine->rx_head].status >> 16) != C2H_WB) {
			dbg_tfr("engine->rx_head has no result magic\n");
			fault = 1;
		} else if (result[engine->rx_head].length > RX_BUF_BLOCK) {
			dbg_tfr("engine->rx_head length > %d\n", RX_BUF_BLOCK);
			fault = 1;
		} else if (result[engine->rx_head].length == 0) {
			dbg_tfr("engine->rx_head length is zero\n");
			fault = 1;
			/* valid result */
		} else {
			pkt_length += result[engine->rx_head].length;
			num_credit++; 
			/* seen eop? */
			//if (result[engine->rx_head].status & RX_STATUS_EOP)
			if (result[engine->rx_head].status & RX_STATUS_EOP){
				eop = 1;
				engine->eop_found = 1;
			}

			dbg_tfr("pkt_length=%d (%s)\n", pkt_length,
				eop ? "with EOP" : "no EOP yet");
		}
		/* clear result */
		result[engine->rx_head].status = 0;
		result[engine->rx_head].length = 0;
		/* proceed head pointer so we make progress, even when fault */
		engine->rx_head = (engine->rx_head + 1) % RX_BUF_PAGES;

		/* stop processing if a fault/eop was detected */
		if (fault || eop){
			break;
		}
	}

	spin_unlock(&engine->lock);

	if (fault)
		rc = -EIO;
	//else if (eop)
	else{
		
		rc = copy_cyclic_to_user(engine, pkt_length, head, buf);
		engine->rx_overrun = 0; 
		/* if copy is successful, release credits */
		if(rc>0){
			write_register(num_credit,&engine->sgdma_regs->credits);
		}
	}

	return rc;
}

static ssize_t char_sgdma_read_cyclic(struct file *file, char __user *buf)
{
	int rc = 0;
	int rc_len = 0;
	struct xdma_char *lro_char;
	struct xdma_dev *lro;
	struct xdma_engine *engine;
	struct xdma_transfer *transfer;

	/* fetch device specific data stored earlier during open */
	lro_char = (struct xdma_char *)file->private_data;
	BUG_ON(!lro_char);
	BUG_ON(lro_char->magic != MAGIC_CHAR);

	lro = lro_char->lro;
	BUG_ON(!lro);
	BUG_ON(lro->magic != MAGIC_DEVICE);

	engine = lro_char->engine;
	/* XXX detect non-supported directions XXX */
	BUG_ON(!engine);
	BUG_ON(engine->magic != MAGIC_ENGINE);

	transfer = engine->rx_transfer_cyclic;
	BUG_ON(!transfer);

	dbg_tfr("char_sgdma_read_cyclic()");
        engine->user_buffer_index = 0;

	do {
		rc = transfer_monitor_cyclic(engine, transfer);
		if (rc)
			return rc;
		rc_len += complete_cyclic(engine, buf);
	} while (!engine->eop_found);

	if(enable_credit_mp){
		engine->eop_found = 0;
	}
	dbg_tfr("returning %d\n", rc_len);
	return rc_len;
}

static void get_perf_stats(struct xdma_engine *engine)
{
	u32 hi;
	u32 lo;

	BUG_ON(!engine);
	BUG_ON(!engine->xdma_perf);

	hi = 0;
	lo = read_register(&engine->regs->completed_desc_count);
	engine->xdma_perf->iterations = build_u64(hi, lo);

	hi = read_register(&engine->regs->perf_cyc_hi);
	lo = read_register(&engine->regs->perf_cyc_lo);

	engine->xdma_perf->clock_cycle_count = build_u64(hi, lo);

	hi = read_register(&engine->regs->perf_dat_hi);
	lo = read_register(&engine->regs->perf_dat_lo);
	engine->xdma_perf->data_cycle_count = build_u64(hi, lo);

	hi = read_register(&engine->regs->perf_pnd_hi);
	lo = read_register(&engine->regs->perf_pnd_lo);
	engine->xdma_perf->pending_count = build_u64(hi, lo);
}

static int ioctl_do_perf_start(struct xdma_engine *engine, unsigned long arg)
{
	u32 w;
	int rc;
	struct xdma_dev *lro;

	BUG_ON(!engine);
	lro = engine->lro;
	BUG_ON(!lro);

	/* performance measurement already running on this engine? */
	if (engine->xdma_perf) {
		dbg_perf("IOCTL_XDMA_PERF_START failed!\n");
		dbg_perf("Perf measurement already seems to be running!\n");
		return -EBUSY;
	}
	engine->xdma_perf = kzalloc(sizeof(struct xdma_performance_ioctl),
		GFP_KERNEL);

	if (!engine->xdma_perf)
		return -ENOMEM;

	rc = copy_from_user(engine->xdma_perf,
		(struct xdma_performance_ioctl *)arg,
		sizeof(struct xdma_performance_ioctl));

	if (rc < 0) {
		dbg_perf("Failed to copy from user space 0x%lx\n", arg);
		return -EINVAL;
	}
	if (engine->xdma_perf->version != IOCTL_XDMA_PERF_V1) {
		dbg_perf("Unsupported IOCTL version %d\n",
			engine->xdma_perf->version);
		return -EINVAL;
	}

	w = XDMA_PERF_CLEAR;
	write_register(w, &engine->regs->perf_ctrl);
	read_register(&engine->regs->identifier);
	w = XDMA_PERF_AUTO | XDMA_PERF_RUN;
	write_register(w, &engine->regs->perf_ctrl);
	read_register(&engine->regs->identifier);

	dbg_perf("IOCTL_XDMA_PERF_START\n");
	dbg_perf("transfer_size = %d\n", engine->xdma_perf->transfer_size);
	/* initialize wait queue */
	init_waitqueue_head(&engine->xdma_perf_wq);
	xdma_performance_submit(lro, engine);

	return 0;
}

static int ioctl_do_perf_stop(struct xdma_engine *engine, unsigned long arg)
{
	struct xdma_transfer *transfer = NULL;
	int rc;

	dbg_perf("IOCTL_XDMA_PERF_STOP\n");

	/* no performance measurement running on this engine? */
	if (!engine->xdma_perf) {
		dbg_perf("No measurement in progress\n");
		return -EINVAL;
	}

	/* stop measurement */
	transfer = engine_cyclic_stop(engine);
	dbg_perf("Waiting for measurement to stop\n");

	if (engine->xdma_perf) {
		get_perf_stats(engine);

		rc = copy_to_user((void __user *)arg, engine->xdma_perf,
			sizeof(struct xdma_performance_ioctl));
		if (rc) {
			dbg_perf("Error copying result to user\n");
			return -EINVAL;
		}
	} else {
		dbg_perf("engine->xdma_perf == NULL?\n");
	}

	kfree(engine->xdma_perf);
	engine->xdma_perf = NULL;

	return 0;
}

static int ioctl_do_perf_get(struct xdma_engine *engine, unsigned long arg)
{
	int rc;

	BUG_ON(!engine);

	dbg_perf("IOCTL_XDMA_PERF_GET\n");

	if (engine->xdma_perf) {
		get_perf_stats(engine);

		rc = copy_to_user((void __user *)arg, engine->xdma_perf,
			sizeof(struct xdma_performance_ioctl));
		if (rc) {
			dbg_perf("Error copying result to user\n");
			return -EINVAL;
		}
	} else {
		dbg_perf("engine->xdma_perf == NULL?\n");
		return -EPROTO;
	}

	return 0;
}

static int ioctl_do_addrmode_set(struct xdma_engine *engine, unsigned long arg)
{
	int rc;
	unsigned long dst;
	u32 w = XDMA_CTRL_NON_INCR_ADDR;

	dbg_perf("IOCTL_XDMA_ADDRMODE_SET\n");
	rc = get_user(dst, (int __user *)arg);

	if (rc == 0) {
		engine->non_incr_addr = !!dst;
		if (engine->non_incr_addr)
			write_register(w, &engine->regs->control_w1s);
		else
			write_register(w, &engine->regs->control_w1c);
	}

	engine_alignments(engine);

	return rc;
}

static int ioctl_do_addrmode_get(struct xdma_engine *engine, unsigned long arg)
{
	int rc;
	unsigned long src;

	BUG_ON(!engine);

	src = !!engine->non_incr_addr;

	dbg_perf("IOCTL_XDMA_ADDRMODE_GET\n");
	rc = put_user(src, (int __user *)arg);

	return rc;
}

static int ioctl_do_align_get(struct xdma_engine *engine, unsigned long arg)
{
	int rc;

	BUG_ON(!engine);

	dbg_perf("IOCTL_XDMA_ALIGN_GET\n");
	rc = put_user(engine->addr_align, (int __user *)arg);
	return rc;
}

static long char_sgdma_ioctl(struct file *file, unsigned int cmd,
		unsigned long arg)
{
	struct xdma_char *lro_char;
	struct xdma_dev *lro;
	struct xdma_engine *engine;
	int rc = 0;

	/* fetch device specific data stored earlier during open */
	lro_char = (struct xdma_char *)file->private_data;
	BUG_ON(!lro_char);
	BUG_ON(lro_char->magic != MAGIC_CHAR);

	lro = lro_char->lro;
	BUG_ON(!lro);
	BUG_ON(lro->magic != MAGIC_DEVICE);

	engine = lro_char->engine;

	switch (cmd) {
	case IOCTL_XDMA_PERF_START:
		rc = ioctl_do_perf_start(engine, arg);
		break;

	case IOCTL_XDMA_PERF_STOP:
		rc = ioctl_do_perf_stop(engine, arg);
		break;

	case IOCTL_XDMA_PERF_GET:
		rc = ioctl_do_perf_get(engine, arg);
		break;

	case IOCTL_XDMA_ADDRMODE_SET:
		rc = ioctl_do_addrmode_set(engine, arg);
		break;

	case IOCTL_XDMA_ADDRMODE_GET:
		rc = ioctl_do_addrmode_get(engine, arg);
		break;

	case IOCTL_XDMA_ALIGN_GET:
		rc = ioctl_do_align_get(engine, arg);
		break;

	default:
		dbg_perf("Unsupported operation\n");
		rc = -EINVAL;
		break;
	}

	return rc;
}

/* sg_write() -- Write to the device
 *
 * @buf userspace buffer
 * @count number of bytes in the userspace buffer
 * @pos byte-address in device
 */
static ssize_t char_sgdma_write(struct file *file, const char __user *buf,
		size_t count, loff_t *pos)
{
	return char_sgdma_read_write(file, (char *)buf, count, pos, 1);
}

/* char_sgdma_read() - Read from the device
 *
 * @buf userspace buffer
 * @count number of bytes in the userspace buffer
 *
 * Iterate over the userspace buffer, taking at most 255 * PAGE_SIZE bytes for
 * each DMA transfer.
 *
 * For each transfer, get the user pages, build a sglist, map, build a
 * descriptor table, submit the transfer, wait for the interrupt handler
 * to wake us on completion, free the sglist and descriptors.
 */
static ssize_t char_sgdma_read(struct file *file, char __user *buf,
		size_t count, loff_t *pos)
{
	struct xdma_char *lro_char;
	struct xdma_dev *lro;
	struct xdma_engine *engine;

	/* fetch device specific data stored earlier during open */
	lro_char = (struct xdma_char *)file->private_data;
	BUG_ON(!lro_char);
	BUG_ON(lro_char->magic != MAGIC_CHAR);

	lro = lro_char->lro;
	BUG_ON(!lro);
	BUG_ON(lro->magic != MAGIC_DEVICE);

	engine = lro_char->engine;

	/* fetch device specific data stored earlier during open */
	lro_char = (struct xdma_char *)file->private_data;
	BUG_ON(!lro_char);
	BUG_ON(lro_char->magic != MAGIC_CHAR);

	lro = lro_char->lro;
	BUG_ON(!lro);
	BUG_ON(lro->magic != MAGIC_DEVICE);

	engine = lro_char->engine;
	/* XXX detect non-supported directions XXX */
	BUG_ON(!engine);
	BUG_ON(engine->magic != MAGIC_ENGINE);

	if (!engine->dir_to_dev && engine->rx_buffer &&
		engine->rx_transfer_cyclic) {
		return char_sgdma_read_cyclic(file, buf);
	} else {
		return char_sgdma_read_write(file, buf, count, pos, 0);
	}
}

/*
 * Called when the device goes from unused to used.
 */
static int char_open(struct inode *inode, struct file *file)
{
	struct xdma_char *lro_char;

	/* pointer to containing structure of the character device inode */
	lro_char = container_of(inode->i_cdev, struct xdma_char, cdev);
	BUG_ON(lro_char->magic != MAGIC_CHAR);
	/* create a reference to our char device in the opened file */
	file->private_data = lro_char;

	return 0;
}

/*
 * Called when the device goes from used to unused.
 */
static int char_close(struct inode *inode, struct file *file)
{
	struct xdma_dev *lro;
	struct xdma_char *lro_char = (struct xdma_char *)file->private_data;

	BUG_ON(!lro_char);
	BUG_ON(lro_char->magic != MAGIC_CHAR);

	/* fetch device specific data stored earlier during open */
	lro = lro_char->lro;
	BUG_ON(!lro);
	BUG_ON(lro->magic != MAGIC_DEVICE);

	return 0;
}

static int cyclic_transfer_setup(struct xdma_engine *engine)
{
	int rc;
	struct xdma_dev *lro;
	u32 w = XDMA_DESC_EOP | XDMA_DESC_COMPLETED;

	BUG_ON(!engine);
	lro = engine->lro;
	BUG_ON(!lro);

	spin_lock(&engine->lock);

	engine->rx_tail = 0;
	engine->rx_head = 0;
	engine->rx_overrun = 0;
	engine->eop_found = 0;
	engine->user_buffer_index = 0;
	if (engine->rx_buffer) {
		dbg_tfr("Channel already open, cannot open twice\n");
		return -EBUSY;
	}

	engine->rx_buffer = rvmalloc(RX_BUF_SIZE);
	if (engine->rx_buffer == NULL) {
		dbg_tfr("rvmalloc(%d) failed\n", RX_BUF_SIZE);
		return -ENOMEM;
	}

	dbg_init("engine->rx_buffer = %p\n", engine->rx_buffer);
	engine->rx_transfer_cyclic = transfer_create(lro, engine->rx_buffer,
		RX_BUF_SIZE, 0, engine->dir_to_dev, 0, 1, 0);
	if (engine->rx_buffer == NULL) {
		dbg_tfr("transfer_create(%d) failed\n", RX_BUF_SIZE);
		rc = -EINVAL;
		goto fail_transfer;
	}

	engine->rx_result_buffer_virt = pci_alloc_consistent(lro->pci_dev,
		RX_RESULT_BUF_SIZE, &engine->rx_result_buffer_bus);

	if (engine->rx_result_buffer_virt == NULL) {
		dbg_tfr("pci_alloc_consistent(%d) failed\n", RX_BUF_SIZE);
		rc = -ENOMEM;
		goto fail_result_buffer;
	}

	dbg_init("engine->rx_result_buffer_virt = %p\n",
		engine->rx_result_buffer_virt);
	dbg_init("engine->rx_result_buffer_bus = 0x%016llx\n",
		engine->rx_result_buffer_bus);
	/* replace source addresses with result write-back addresses */
	transfer_set_result_addresses(engine->rx_transfer_cyclic,
		engine->rx_result_buffer_bus);
	/* set control of all descriptors */
	transfer_set_all_control(engine->rx_transfer_cyclic, w);
	/* make this a cyclic transfer */
	xdma_transfer_cyclic(engine->rx_transfer_cyclic);
	transfer_dump(engine->rx_transfer_cyclic);

	spin_unlock(&engine->lock);

	/* write initial credits */
	if(enable_credit_mp){
		//write_register(RX_BUF_PAGES,&engine->sgdma_regs->credits);
		write_register(128,&engine->sgdma_regs->credits);
	}

	/* start cyclic transfer */
	if (engine->rx_transfer_cyclic)
		transfer_queue(engine, engine->rx_transfer_cyclic);

	return 0;

	/* unwind on errors */
fail_result_buffer:
	transfer_destroy(engine->lro, engine->rx_transfer_cyclic);
	engine->rx_transfer_cyclic = NULL;
fail_transfer:
	rvfree(engine->rx_buffer, RX_BUF_SIZE);
	engine->rx_buffer = NULL;
	spin_unlock(&engine->lock);

	return rc;
}

/*
 * Called when the device goes from unused to used.
 */
static int char_sgdma_open(struct inode *inode, struct file *file)
{
	int rc = 0;
	struct xdma_char *lro_char;
	struct xdma_engine *engine;

	/* pointer to containing structure of the character device inode */
	lro_char = container_of(inode->i_cdev, struct xdma_char, cdev);
	BUG_ON(lro_char->magic != MAGIC_CHAR);
	/* create a reference to our char device in the opened file */
	file->private_data = lro_char;

	engine = lro_char->engine;
	BUG_ON(!engine);
	BUG_ON(engine->magic != MAGIC_ENGINE);

	dbg_tfr("char_sgdma_open(0x%p, 0x%p)\n", inode, file);

	/* AXI ST C2H? Set up RX ring buffer on host with a cyclic transfer */
	if (engine->streaming && !engine->dir_to_dev)
		rc = cyclic_transfer_setup(engine);

	return rc;
}

static int cyclic_shutdown_polled(struct xdma_engine *engine)
{
	BUG_ON(!engine);

	spin_lock(&engine->lock);

	dbg_tfr("Polling for shutdown completion\n");
	do {
		engine_status_read(engine, 1);
		schedule();
	} while (engine->status & XDMA_STAT_BUSY);

	if ((engine->running) && !(engine->status & XDMA_STAT_BUSY)) {
		dbg_tfr("Engine has stopped\n");

		if (!list_empty(&engine->transfer_list))
			engine_transfer_dequeue(engine);

		engine_service_shutdown(engine);
	}

	dbg_tfr("Shutdown completion polling done\n");
	spin_unlock(&engine->lock);

	return 0;
}

static int cyclic_shutdown_interrupt(struct xdma_engine *engine)
{
	int rc;

	BUG_ON(!engine);

	rc = wait_event_interruptible(engine->shutdown_wq, !engine->running);

	if (rc) {
		dbg_tfr("wait_event_interruptible=%d\n", rc);
		return rc;
	}

	if (engine->running) {
		dbg_tfr("engine still running?!\n");
		return -EINVAL;
	}

	dbg_tfr("wait_event_interruptible=%d\n", rc);
	return rc;
}

static int cyclic_transfer_teardown(struct xdma_engine *engine)
{
	int rc;
	struct xdma_dev *lro;
	struct xdma_transfer *transfer;

	BUG_ON(!engine);
	lro = engine->lro;
	BUG_ON(!lro);

	transfer = engine_cyclic_stop(engine);

	spin_lock(&engine->lock);
	if (transfer) {
		dbg_tfr("char_sgdma_close() stopped transfer %p\n", transfer);
		if (transfer != engine->rx_transfer_cyclic) {
			dbg_tfr("unexpected transfer %p\n", transfer);
			dbg_tfr("engine->rx_transfer_cyclic = %p\n",
				engine->rx_transfer_cyclic);
		}
	}
	/* allow engine to be serviced after stop request */
	spin_unlock(&engine->lock);
	/* wait for engine to be no longer running */

	if (poll_mode)
		rc = cyclic_shutdown_polled(engine);
	else
		rc = cyclic_shutdown_interrupt(engine);

	/* obtain spin lock to atomically remove resources */
	spin_lock(&engine->lock);

	if (engine->rx_transfer_cyclic) {
		transfer_destroy(engine->lro, engine->rx_transfer_cyclic);
		engine->rx_transfer_cyclic = NULL;
	}
	if (engine->rx_buffer) {
		rvfree(engine->rx_buffer, RX_BUF_SIZE);
		engine->rx_buffer = NULL;
	}
	if (engine->rx_result_buffer_virt) {
		/* free contiguous list */
		pci_free_consistent(lro->pci_dev, RX_RESULT_BUF_SIZE,
			engine->rx_result_buffer_virt,
			engine->rx_result_buffer_bus);
		engine->rx_result_buffer_virt = NULL;
	}

	spin_unlock(&engine->lock);

	return rc;
}

/*
 * Called when the device goes from used to unused.
 */
static int char_sgdma_close(struct inode *inode, struct file *file)
{
	struct xdma_char *lro_char = (struct xdma_char *)file->private_data;
	struct xdma_engine *engine;
	int rc = 0;

	BUG_ON(!lro_char);
	BUG_ON(lro_char->magic != MAGIC_CHAR);

	/* fetch device specific data stored earlier during open */
	engine = lro_char->engine;
	BUG_ON(!engine);
	BUG_ON(engine->magic != MAGIC_ENGINE);

	dbg_tfr("char_sgdma_close(0x%p, 0x%p)\n", inode, file);

	if (engine->streaming && !engine->dir_to_dev)
		rc = cyclic_transfer_teardown(engine);

	return rc;
}

/*
 * RTO - code to detect if MSI/MSI-X capability exists is derived
 * from linux/pci/msi.c - pci_msi_check_device
 */

#ifndef arch_msi_check_device
int arch_msi_check_device(struct pci_dev *dev, int nvec, int type)
{
	return 0;
}
#endif

/* type = PCI_CAP_ID_MSI or PCI_CAP_ID_MSIX */
static int msi_msix_capable(struct pci_dev *dev, int type)
{
	struct pci_bus *bus;
	int ret;

	if (!dev || dev->no_msi)
		return 0;

	for (bus = dev->bus; bus; bus = bus->parent)
		if (bus->bus_flags & PCI_BUS_FLAGS_NO_MSI)
			return 0;

	ret = arch_msi_check_device(dev, 1, type);
	if (ret)
		return 0;

	if (!pci_find_capability(dev, type))
		return 0;

	return 1;
}

static struct xdma_dev *alloc_dev_instance(struct pci_dev *pdev)
{
	int i;
	struct xdma_dev *lro;

	BUG_ON(!pdev);

	/* allocate zeroed device book keeping structure */
	lro = kzalloc(sizeof(struct xdma_dev), GFP_KERNEL);
	if (!lro) {
		dbg_init("Could not kzalloc(xdma_dev).\n");
		return NULL;
	}

	lro->magic = MAGIC_DEVICE;
	lro->config_bar_idx = -1;
	lro->user_bar_idx = -1;
	lro->bypass_bar_idx = -1;
	lro->irq_line = -1;

	/* create a device to driver reference */
	dev_set_drvdata(&pdev->dev, lro);
	/* create a driver to device reference */
	lro->pci_dev = pdev;
	dbg_init("probe() lro = 0x%p\n", lro);

	/* Set up data user IRQ data structures */
	for (i = 0; i < MAX_USER_IRQ; i++) {
		lro->user_irq[i].lro = lro;
		spin_lock_init(&lro->user_irq[i].events_lock);
		init_waitqueue_head(&lro->user_irq[i].events_wq);
	}

	return lro;
}

static int probe_scan_for_msi(struct xdma_dev *lro, struct pci_dev *pdev)
{
	int i;
	int rc = 0;
	int req_nvec = MAX_NUM_ENGINES + MAX_USER_IRQ;

	BUG_ON(!lro);
	BUG_ON(!pdev);

	if (msi_msix_capable(pdev, PCI_CAP_ID_MSIX)) {
		dbg_init("Enabling MSI-X\n");
		for (i = 0; i < req_nvec; i++)
			lro->entry[i].entry = i;

		rc = pci_enable_msix(pdev, lro->entry, req_nvec);
		if (rc < 0)
			dbg_init("Couldn't enable MSI-X mode: rc = %d\n", rc);

		lro->msix_enabled = 1;
	} else if (msi_msix_capable(pdev, PCI_CAP_ID_MSI)) {
		/* enable message signalled interrupts */
		dbg_init("pci_enable_msi()\n");
		rc = pci_enable_msi(pdev);
		if (rc < 0)
			dbg_init("Couldn't enable MSI mode: rc = %d\n", rc);
		lro->msi_enabled = 1;
	} else {
		dbg_init("MSI/MSI-X not detected - using legacy interrupts\n");
	}

	return rc;
}

static int request_regions(struct xdma_dev *lro, struct pci_dev *pdev)
{
	int rc;

	BUG_ON(!lro);
	BUG_ON(!pdev);

	dbg_init("pci_request_regions()\n");
	rc = pci_request_regions(pdev, DRV_NAME);
	/* could not request all regions? */
	if (rc) {
		dbg_init("pci_request_regions() = %d, device in use?\n", rc);
		/* assume device is in use so do not disable it later */
		lro->regions_in_use = 1;
	} else {
		lro->got_regions = 1;
	}

	return rc;
}

static int set_dma_mask(struct pci_dev *pdev)
{
	int rc = 0;

	BUG_ON(!pdev);

	dbg_init("sizeof(dma_addr_t) == %ld\n", sizeof(dma_addr_t));
	/* 64-bit addressing capability for XDMA? */
	if (!pci_set_dma_mask(pdev, DMA_BIT_MASK(64))) {
		/* query for DMA transfer */
		/* @see Documentation/DMA-mapping.txt */
		dbg_init("pci_set_dma_mask()\n");
		/* use 64-bit DMA */
		dbg_init("Using a 64-bit DMA mask.\n");
		/* use 32-bit DMA for descriptors */
		pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(32));
		/* use 64-bit DMA, 32-bit for consistent */
	} else if (!pci_set_dma_mask(pdev, DMA_BIT_MASK(32))) {
		dbg_init("Could not set 64-bit DMA mask.\n");
		pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(32));
		/* use 32-bit DMA */
		dbg_init("Using a 32-bit DMA mask.\n");
	} else {
		dbg_init("No suitable DMA possible.\n");
		rc = -1;
	}

	return rc;
}

static u32 build_vector_reg(u32 a, u32 b, u32 c, u32 d)
{
	u32 reg_val = 0;

	reg_val |= (a & 0x1f) << 0;
	reg_val |= (b & 0x1f) << 8;
	reg_val |= (c & 0x1f) << 16;
	reg_val |= (d & 0x1f) << 24;

	return reg_val;
}

static void write_msix_vectors(struct xdma_dev *lro)
{
	struct interrupt_regs *int_regs;
	u32 reg_val;

	BUG_ON(!lro);
	int_regs = (struct interrupt_regs *)
			(lro->bar[lro->config_bar_idx] + XDMA_OFS_INT_CTRL);

	/* user irq MSI-X vectors */
	reg_val = build_vector_reg(0, 1, 2, 3);
	write_register(reg_val, &int_regs->user_msi_vector[0]);

	reg_val = build_vector_reg(4, 5, 6, 7);
	write_register(reg_val, &int_regs->user_msi_vector[1]);

	reg_val = build_vector_reg(8, 9, 10, 11);
	write_register(reg_val, &int_regs->user_msi_vector[2]);

	reg_val = build_vector_reg(12, 13, 14, 15);
	write_register(reg_val, &int_regs->user_msi_vector[3]);

	/* channel irq MSI-X vectors */
	reg_val = build_vector_reg(16, 17, 18, 19);
	write_register(reg_val, &int_regs->channel_msi_vector[0]);

	reg_val = build_vector_reg(20, 21, 22, 23);
	write_register(reg_val, &int_regs->channel_msi_vector[1]);
}

static int msix_irq_setup(struct xdma_dev *lro)
{
	int i;
	int rc;

	BUG_ON(!lro);
	write_msix_vectors(lro);

	for (i = 0; i < MAX_USER_IRQ; i++) {
		rc = request_irq(lro->entry[i].vector, xdma_user_irq, 0,
			DRV_NAME, &lro->user_irq[i]);

		if (rc) {
			dbg_init("Couldn't use IRQ#%d, rc=%d\n",
				lro->entry[i].vector, rc);
			break;
		}

		dbg_init("Using IRQ#%d with 0x%p\n", lro->entry[i].vector,
			&lro->user_irq[i]);
	}

	/* If any errors occur, free IRQs that were successfully requested */
	if (rc) {
		while (--i >= 0)
			free_irq(lro->entry[i].vector, &lro->user_irq[i]);
		return -1;
	}

	lro->irq_user_count = MAX_USER_IRQ;

	return 0;
}

static void irq_teardown(struct xdma_dev *lro)
{
	int i;

	BUG_ON(!lro);

	if (lro->msix_enabled) {
		for (i = 0; i < lro->irq_user_count; i++) {
			dbg_init("Releasing IRQ#%d\n", lro->entry[i].vector);
			free_irq(lro->entry[i].vector, &lro->user_irq[i]);
		}
	} else if (lro->irq_line != -1) {
		dbg_init("Releasing IRQ#%d\n", lro->irq_line);
		free_irq(lro->irq_line, lro);
	}
}

static int irq_setup(struct xdma_dev *lro, struct pci_dev *pdev)
{
	int rc = 0;
	u32 irq_flag;
	u8 val;
	void *reg;
	u32 w;

	BUG_ON(!lro);

	if (lro->msix_enabled) {
		rc = msix_irq_setup(lro);
	} else {
		if (!lro->msi_enabled){
			pci_read_config_byte(pdev, PCI_INTERRUPT_PIN, &val);
			dbg_init("Legacy Interrupt register value = %d\n", val);
			if (val > 1) {
				val--;
				w = (val<<24) | (val<<16) | (val<<8)| val;
				// Program IRQ Block Channel vactor and IRQ Block User vector with Legacy interrupt value
				reg = lro->bar[lro->config_bar_idx] + 0x2080;   // IRQ user
				write_register(w, reg);      
				write_register(w, reg+0x4);
				write_register(w, reg+0x8);
				write_register(w, reg+0xC);
				reg = lro->bar[lro->config_bar_idx] + 0x20A0;   // IRQ Block
				write_register(w, reg);     
				write_register(w, reg+0x4);
			}
		}
		irq_flag = lro->msi_enabled ? 0 : IRQF_SHARED;
		lro->irq_line = (int)pdev->irq;
		rc = request_irq(pdev->irq, xdma_isr, irq_flag, DRV_NAME, lro);
		if (rc)
			dbg_init("Couldn't use IRQ#%d, rc=%d\n", pdev->irq, rc);
		else
			dbg_init("Using IRQ#%d with 0x%p\n", pdev->irq, lro);
	}

	return rc;
}

static void enable_credit_feature(struct xdma_dev *lro)
{
	u32 w;
	struct sgdma_common_regs *reg = (struct sgdma_common_regs *)
		(lro->bar[lro->config_bar_idx] + (0x6*TARGET_SPACING));
	printk("credit_feature_enable addr = %p",&reg->credit_feature_enable);

	w = 0xF0000U;
	write_register(w,&reg->credit_feature_enable);
}

static u32 get_engine_type(struct engine_regs *regs)
{
	u32 value;

	BUG_ON(!regs);

	value = read_register(&regs->identifier);
	return (value & 0x8000U) ? 1 : 0;
}

static u32 get_engine_channel_id(struct engine_regs *regs)
{
	u32 value;

	BUG_ON(!regs);

	value = read_register(&regs->identifier);

	return (value & 0x00000f00U) >> 8;
}

static u32 get_engine_id(struct engine_regs *regs)
{
	u32 value;

	BUG_ON(!regs);

	value = read_register(&regs->identifier);
	return (value & 0xffff0000U) >> 16;
}

static void remove_engines(struct xdma_dev *lro)
{
	int channel;
	struct xdma_engine *engine;

	BUG_ON(!lro);

	/* iterate over channels */
	for (channel = 0; channel < XDMA_CHANNEL_NUM_MAX; channel++) {
		engine = lro->engine[channel][0];
		if (engine) {
			dbg_sg("Remove %s%d", engine->name, channel);
			engine_destroy(lro, engine);
			dbg_sg("%s%d removed", engine->name, channel);
		}

		engine = lro->engine[channel][1];
		if (engine) {
			dbg_sg("Remove %s%d", engine->name, channel);
			engine_destroy(lro, engine);
			dbg_sg("%s%d removed", engine->name, channel);
		}
	}
}

static int probe_for_engine(struct xdma_dev *lro, int dir_to_dev, int channel)
{
	struct engine_regs *regs;
	int dir_from_dev;
	int offset;
	u32 engine_id;
	u32 engine_id_expected;
	u32 channel_id;
	int rc = 0;

	dir_from_dev = !dir_to_dev;

	/* register offset for the engine */
	/* read channels at 0x0000, write channels at 0x1000,
	 * channels at 0x100 interval */
	offset = (dir_from_dev * H2C_CHANNEL_OFFSET) +
		(channel * CHANNEL_SPACING);

	regs = lro->bar[lro->config_bar_idx] + offset;
	if (dir_to_dev) {
		dbg_init("Probing for H2C %d engine at %p\n", channel, regs);
		engine_id_expected = XDMA_ID_H2C;
	} else {
		dbg_init("Probing for C2C %d engine at %p\n", channel, regs);
		engine_id_expected = XDMA_ID_C2H;
	}

	engine_id = get_engine_id(regs);
	channel_id = get_engine_channel_id(regs);
	dbg_init("engine ID = 0x%x\n", engine_id);
	dbg_init("engine channel ID = 0x%x\n", channel_id);

	if (engine_id != engine_id_expected) {
		dbg_init("Incorrect engine ID - skipping\n");
		return 0;
	}

	if (channel_id != channel) {
		dbg_init("Expected ch ID%d, read %d\n", channel, channel_id);
		return 0;
	}

	if (dir_to_dev)
		dbg_init("Found H2C %d AXI engine at %p\n", channel, regs);
	else
		dbg_init("Found C2H %d AXI engine at %p\n", channel, regs);

	/* allocate and initialize engine */
	lro->engine[channel][dir_from_dev] = engine_create(lro, offset,
		dir_to_dev, channel);

	if (!lro->engine[channel][dir_from_dev]) {
		dbg_init("Error creating channel\n");
		rc = -1;
	}

	return rc;
}

static void destroy_interfaces(struct xdma_dev *lro)
{
	int channel;
	int idx;

	/* iterate over channels */
	for (channel = 0; channel < XDMA_CHANNEL_NUM_MAX; channel++) {
		/* remove SG DMA character device */
		if (lro->sgdma_char_dev[channel][0])
			destroy_sg_char(lro->sgdma_char_dev[channel][0]);

		/* remove SG DMA character device */
		if (lro->sgdma_char_dev[channel][1])
			destroy_sg_char(lro->sgdma_char_dev[channel][1]);

		if (lro->bypass_char_dev[channel][0])
			destroy_sg_char(lro->bypass_char_dev[channel][0]);

		/* remove SG DMA character device */
		if (lro->bypass_char_dev[channel][1])
			destroy_sg_char(lro->bypass_char_dev[channel][1]);
	}

	if (lro->bypass_char_dev_base) 
                destroy_sg_char(lro->bypass_char_dev_base);

	for (idx = 0; idx < MAX_USER_IRQ; idx++) {
		if (lro->events_char_dev[idx])
			destroy_sg_char(lro->events_char_dev[idx]);
	}

	/* remove control character device */
	if (lro->ctrl_char_dev)
		destroy_sg_char(lro->ctrl_char_dev);

	/* remove user character device */
	if (lro->user_char_dev)
		destroy_sg_char(lro->user_char_dev);
}

static int create_engine_interface(struct xdma_engine *engine, int channel,
	int dir_to_dev)
{
	struct xdma_dev *lro;
	enum chardev_type type;
	int rc = 0;
	int dir_from_dev = !dir_to_dev;

	BUG_ON(!engine);
	lro = engine->lro;
	BUG_ON(!lro);

	type = dir_to_dev ? CHAR_XDMA_H2C : CHAR_XDMA_C2H;
	lro->sgdma_char_dev[channel][dir_from_dev] =
		create_sg_char(lro, -1, engine, type);

	if (!lro->sgdma_char_dev[channel][dir_from_dev]) {
		dbg_init("%s%d I/F fail\n", engine->name, channel);
		rc = -1;
	}

	return rc;
}

static int create_bypass_interface(struct xdma_engine *engine, int channel,
	int dir_to_dev)
{
	struct xdma_dev *lro;
	enum chardev_type type;
	int rc = 0;
	int dir_from_dev = !dir_to_dev;

	BUG_ON(!engine);
	lro = engine->lro;
	BUG_ON(!lro);

	type = dir_to_dev ? CHAR_BYPASS_H2C : CHAR_BYPASS_C2H;
	lro->bypass_char_dev[channel][dir_from_dev] =
		create_sg_char(lro, lro->bypass_bar_idx, engine, type);

	if (!lro->bypass_char_dev[channel][dir_from_dev]) {
		dbg_init("%s%d I/F fail\n", engine->name, channel);
		rc = -1;
	}

	return rc;
}

static int create_interfaces(struct xdma_dev *lro)
{
	int channel;
	int i;
	int rc = 0;
	struct xdma_engine *engine;

	BUG_ON(!lro);

	/* initialize user character device */
	if (lro->user_bar_idx < 0) {
		dbg_init("No user logic BAR detected - skip device setup\n");
		lro->user_char_dev = NULL;
	} else {
		lro->user_char_dev = create_sg_char(lro, lro->user_bar_idx,
			NULL, CHAR_USER);
		if (!lro->user_char_dev) {
			dbg_init("create_char(user_char_dev) failed\n");
			goto fail;
		}
	}

	/* initialize control character device */
	lro->ctrl_char_dev = create_sg_char(lro, lro->config_bar_idx, NULL,
		CHAR_CTRL);
	if (!lro->ctrl_char_dev) {
		dbg_init("create_char(ctrl_char_dev) failed\n");
		goto fail;
	}

	/* initialize events character device */
	for (i = 0; i < MAX_USER_IRQ; i++) {
		lro->events_char_dev[i] = create_sg_char(lro, i, NULL,
			CHAR_EVENTS);
		if (!lro->events_char_dev) {
			dbg_init("Creating event %d failed\n", i);
			goto fail;
		}
	}

	/* iterate over channels */
	for (channel = 0; channel < XDMA_CHANNEL_NUM_MAX; channel++) {
		engine = lro->engine[channel][0];

		if (engine)
			rc = create_engine_interface(engine, channel, 1);

		if (rc) {
			dbg_init("Creating H2C%d I/F failed\n", channel);
			goto fail;
		}
	}

	for (channel = 0; channel < XDMA_CHANNEL_NUM_MAX; channel++) {
		engine = lro->engine[channel][1];

		if (engine)
			rc = create_engine_interface(engine, channel, 0);

		if (rc) {
			dbg_init("Creating C2H%d I/F failed\n", channel);
			goto fail;
		}
	}

	/* iterate over channels */
	for (channel = 0; channel < XDMA_CHANNEL_NUM_MAX; channel++) {
		engine = lro->engine[channel][0];

		if (engine && (lro->bypass_bar_idx >= 0))
			rc = create_bypass_interface(engine, channel, 1);

		if (rc) {
			dbg_init("Creating H2C%d bypass I/F failed\n", channel);
			goto fail;
		}
	}

	for (channel = 0; channel < XDMA_CHANNEL_NUM_MAX; channel++) {
		engine = lro->engine[channel][1];

		if (engine && (lro->bypass_bar_idx >= 0))
			rc = create_bypass_interface(engine, channel, 0);

		if (rc) {
			dbg_init("Creating C2H%d bypass I/F failed\n", channel);
			goto fail;
		}
	}

	if (lro->bypass_bar_idx < 0) {
		dbg_init("No bypass BAR detected - skip device setup\n");
		lro->bypass_char_dev_base = NULL;
	}else{
		lro->bypass_char_dev_base = create_sg_char(lro, lro->bypass_bar_idx, NULL, CHAR_BYPASS);
	}

	return 0;

fail:
	destroy_interfaces(lro);
	return -1;
}

static int probe_engines(struct xdma_dev *lro)
{
	int channel;
	int rc = 0;

	BUG_ON(!lro);

	/* iterate over channels */
	for (channel = 0; channel < XDMA_CHANNEL_NUM_MAX; channel++) {
		rc = probe_for_engine(lro, 1, channel);
		if (rc)
			goto fail;
	}

	for (channel = 0; channel < XDMA_CHANNEL_NUM_MAX; channel++) {
		rc = probe_for_engine(lro, 0, channel);
		if (rc)
			break;
	}

	return 0;

fail:
	dbg_init("Engine probing failed - unwinding\n");
	remove_engines(lro);

	return -1;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,5,0)
static void enable_pcie_relaxed_ordering(struct pci_dev *dev)
{
	pcie_capability_set_word(dev, PCI_EXP_DEVCTL, PCI_EXP_DEVCTL_RELAX_EN);
}
#else
static void __devinit enable_pcie_relaxed_ordering(struct pci_dev *dev)
{
	u16 v;
	int pos;

	pos = pci_pcie_cap(dev);
	if (pos > 0) {
		pci_read_config_word(dev, pos + PCI_EXP_DEVCTL, &v);
		v |= PCI_EXP_DEVCTL_RELAX_EN;
		pci_write_config_word(dev, pos + PCI_EXP_DEVCTL, v);
	}
}
#endif

static void pcie_check_extended_tag(struct xdma_dev *lro, struct pci_dev *pdev)
{
	u16 cap;
	u32 v;
	void *__iomem reg;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,5,0)
	pcie_capability_read_word(pdev, PCI_EXP_DEVCTL, &cap);
#else
	int pos;

	pos = pci_pcie_cap(pdev);
	if (pos > 0)
		pci_read_config_word(pdev, pos + PCI_EXP_DEVCTL, &cap);
	else {
		pr_info("pdev 0x%p, unable to access pcie cap.\n", pdev);
		return;
	}
#endif

	if ((cap & PCI_EXP_DEVCTL_EXT_TAG))
		return;

	/* extended tag not enabled */
	pr_info("0x%p EXT_TAG disabled.\n", pdev);

	if (lro->config_bar_idx < 0) {
		pr_info("pdev 0x%p, config bar UNKNOWN.\n", pdev);
		return;
	}

 	reg = lro->bar[lro->config_bar_idx] + XDMA_OFS_CONFIG + 0x4C;
	v =  read_register(reg);
	v = (v & 0xFF) | (((u32)32) << 8);
	write_register(v, reg);
}

static int probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	int rc = 0;
	struct xdma_dev *lro = NULL;
	#if SD_ACCEL
	struct xdma_bitstream_container *clear_contr = NULL;
	#endif
	void *reg = NULL;
	u32 w;
	int stream;

	dbg_init("probe(pdev = 0x%p, pci_id = 0x%p)\n", pdev, id);
	#if SD_ACCEL
	/* SD_Accel Specific */
	clear_contr = dev_get_drvdata(&pdev->dev);
        if (clear_contr)
                printk(KERN_INFO "%s: Found stashed clearing bitstream from previously loaded driver %p\n",
                       DRV_NAME, clear_contr);
	#endif

	/* allocate zeroed device book keeping structure */
	lro = alloc_dev_instance(pdev);
	if (!lro) {
		pr_info("%s: OOM.\n", __func__);
		return -ENOMEM;
	}

	#if SD_ACCEL
	if (lro->pci_dev->device == 0x8138)
		lro->mcap_base = ULTRASCALE_MCAP_CONFIG_BASE;

	if (load_firmware && (lro->pci_dev->device == 0x8138)) {
                if (clear_contr && (clear_contr->magic == 0xBBBBBBBBUL)) {
			/* Copy the stashed clear bitstream from previously loaded driver to this initialization of driver */
			memcpy(&lro->stash, clear_contr, sizeof(struct xdma_bitstream_container));
			kfree(clear_contr);
			clear_contr = NULL;
		}
		else {
			lro->stash.magic = 0xBBBBBBBBUL;
		}
                load_boot_firmware(lro);
        }
	#endif

	rc = pci_enable_device(pdev);
	if (rc) {
		dbg_init("pci_enable_device() failed, rc = %d.\n", rc);
		goto free_alloc;
	}

	/* enable relaxed ordering */
	enable_pcie_relaxed_ordering(pdev);
        
	/* enable bus master capability */
	dbg_init("pci_set_master()\n");
	pci_set_master(pdev);

	rc = probe_scan_for_msi(lro, pdev);
	if (rc < 0)
		goto disable_msi;

	/* known root complex's max read request sizes */
	#ifdef CONFIG_ARCH_TI816X
	dbg_init("TI816X RC detected: limit MaxReadReq size to 128 bytes.\n");
	pcie_set_readrq(pdev, 128);
	#endif

	rc = request_regions(lro, pdev);
	if (rc)
		goto rel_region;

	rc = map_bars(lro, pdev);
	if (rc)
		goto unmap_bar;

	pcie_check_extended_tag(lro, pdev);

	/* SD_Accel */
	#if SD_ACCEL && 0
	printk(KERN_DEBUG "xdma_config()\n");
	/* determine XDMA system configuration */
	rc = xdma_config(lro, 0x000);
	if (rc)
		goto err_system;
	rc = engine_initialize(lro, XDMA_OFS_DMA_WRITE);
	rc = engine_initialize(lro, XDMA_OFS_DMA_READ);
	#endif

	rc = set_dma_mask(pdev);
	if (rc)
		goto unmap_bar;

	rc = irq_setup(lro, pdev);
	if (rc)
		goto disable_irq;
    
	/* enable credit system only in AXI-Stream mode*/
	reg = lro->bar[lro->config_bar_idx];
        w = read_register(reg);
	stream = (w & 0x8000U) ? 1 : 0;
	if (enable_credit_mp & stream ) {
		printk(KERN_DEBUG "Design in Steaming mode enable Credit feature \n");
		enable_credit_feature(lro);
	}

	rc = probe_engines(lro);
	if (rc)
		goto rmv_engine;

	rc = create_interfaces(lro);
	if (rc)
		goto rmv_interface;

	/* enable user interrupts */
	user_interrupts_enable(lro, ~0);

	/* If not running in polled mode, enable engine interrupts */
	if (!poll_mode)
		channel_interrupts_enable(lro, ~0);

	/* Flush writes */
	read_interrupts(lro);

	rc = 0;

	#if SD_ACCEL
	if (clear_contr && (clear_contr->magic == 0xBBBBBBBBUL)) {
		/* Copy the stashed clear bitstream from previously loaded driver to this initialization of driver */
		memcpy(&lro->stash, clear_contr, sizeof(struct xdma_bitstream_container));
		kfree(clear_contr);
		clear_contr = NULL;
	}
	else {
		lro->stash.magic = 0xBBBBBBBBUL;
	}

	lro->feature_id = find_feature_id(lro);
	#endif
	
	printk(KERN_DEBUG "Create device attribute file for major =%d, instance = %d\n", (int)lro->major, (int)lro->instance);
	rc = device_create_file(&pdev->dev, &dev_attr_xdma_dev_instance);
	if(rc){
		printk(KERN_DEBUG "Failed to create device file \n");
		goto rmv_cdev;
	}else{
		printk(KERN_DEBUG "Device file created successfully\n");
	}

	if (rc == 0)
		return 0;

rmv_cdev:
        dev_present[lro->instance] = 0;
	device_remove_file(&pdev->dev, &dev_attr_xdma_dev_instance);
rmv_interface:
	destroy_interfaces(lro);
rmv_engine:
	remove_engines(lro);
disable_irq:
	irq_teardown(lro);
unmap_bar:
	unmap_bars(lro, pdev);
rel_region:
	if (lro->got_regions) {
		pci_release_regions(pdev);
		lro->got_regions = 0;
	}
disable_msi:
	if (lro->msix_enabled) {
		pci_disable_msix(pdev);
		lro->msix_enabled = 0;
	} else if (lro->msi_enabled) {
		pci_disable_msi(pdev);
		lro->msi_enabled = 0;
	}
	if (!lro->regions_in_use)
		pci_disable_device(pdev);
free_alloc:
	kfree(lro);

	dbg_init("probe() returning %d\n", rc);
	return -1;
}

static void remove(struct pci_dev *pdev)
{
	struct xdma_dev *lro;
#if SD_ACCEL
	struct xdma_bitstream_container *clear_contr = NULL;
#endif

	dbg_sg("remove(0x%p)\n", pdev);
	if ((pdev == 0) || (dev_get_drvdata(&pdev->dev) == 0)) {
		dbg_sg("remove(dev = 0x%p) pdev->dev.driver_data = 0x%p\n",
			   pdev, dev_get_drvdata(&pdev->dev));
		return;
	}
	lro = (struct xdma_dev *)dev_get_drvdata(&pdev->dev);
	dbg_sg("remove(dev = 0x%p) where pdev->dev.driver_data = 0x%p\n",
		   pdev, lro);
	if (lro->pci_dev != pdev) {
		dbg_sg("pdev->dev.driver_data->pci_dev(0x%lx) != pdev(0x%lx)\n",
			(unsigned long)lro->pci_dev, (unsigned long)pdev);
	}

	channel_interrupts_disable(lro, ~0);
	user_interrupts_disable(lro, ~0);
	read_interrupts(lro);

	destroy_interfaces(lro);
	remove_engines(lro);
	irq_teardown(lro);
	unmap_bars(lro, pdev);

	if (lro->got_regions)
		pci_release_regions(pdev);

	if (lro->msix_enabled) {
		pci_disable_msix(pdev);
		lro->msix_enabled = 0;
	} else if (lro->msi_enabled) {
		pci_disable_msi(pdev);
		lro->msi_enabled = 0;
	}

	if (!lro->regions_in_use)
		pci_disable_device(pdev);

	/* SD_Accel */
	#if SD_ACCEL && 1
	if (lro->pci_dev->device == 0x8138) {
		/* For Ultrascale stash the clearing bitstream as driver data with device */
		/* It will be retrieved next time driver is loaded and used with the next PR bitstream */
		clear_contr = kmalloc(sizeof(struct xdma_bitstream_container), GFP_KERNEL);
		if (clear_contr)
			memcpy(clear_contr, &lro->stash, sizeof(struct xdma_bitstream_container));
	}
	#endif
	
	dev_present[lro->instance] = 0;
	device_remove_file(&pdev->dev, &dev_attr_xdma_dev_instance);

	kfree(lro);
	
	#if SD_ACCEL && 1
	dev_set_drvdata(&pdev->dev, clear_contr);
	#endif
}

/* maps the PCIe BAR into user space for memory-like access using mmap() */
static int bridge_mmap(struct file *file, struct vm_area_struct *vma)
{
	int rc;
	struct xdma_dev *lro;
	struct xdma_char *lro_char = (struct xdma_char *)file->private_data;
	unsigned long off;
	unsigned long phys;
	unsigned long vsize;
	unsigned long psize;

	BUG_ON(!lro_char);
	BUG_ON(lro_char->magic != MAGIC_CHAR);
	lro = lro_char->lro;
	BUG_ON(!lro);
	BUG_ON(lro->magic != MAGIC_DEVICE);

	off = vma->vm_pgoff << PAGE_SHIFT;
	/* BAR physical address */
	phys = pci_resource_start(lro->pci_dev, lro_char->bar) + off;
	vsize = vma->vm_end - vma->vm_start;
	/* complete resource */
	psize = pci_resource_end(lro->pci_dev, lro_char->bar) -
		pci_resource_start(lro->pci_dev, lro_char->bar) + 1 - off;

	dbg_sg("mmap(): lro_char = 0x%08lx\n",
		(unsigned long)lro_char);
	dbg_sg("mmap(): lro_char->bar = %d\n", lro_char->bar);
	dbg_sg("mmap(): lro = 0x%p\n", lro);
	dbg_sg("mmap(): pci_dev = 0x%08lx\n",
		(unsigned long)lro->pci_dev);

	dbg_sg("off = 0x%lx\n", off);
	dbg_sg("start = 0x%llx\n",
		(unsigned long long)pci_resource_start(lro->pci_dev,
		lro_char->bar));
	dbg_sg("phys = 0x%lx\n", phys);

	if (vsize > psize)
		return -EINVAL;
	/*
	 * pages must not be cached as this would result in cache line sized
	 * accesses to the end point
	 */
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	/*
	 * prevent touching the pages (byte access) for swap-in,
	 * and prevent the pages from being swapped out
	 */
	vma->vm_flags |= VMEM_FLAGS;
	/* make MMIO accessible to user space */
	rc = io_remap_pfn_range(vma, vma->vm_start, phys >> PAGE_SHIFT,
			vsize, vma->vm_page_prot);
	dbg_sg("vma=0x%p, vma->vm_start=0x%lx, phys=0x%lx, size=%lu = %d\n",
		vma, vma->vm_start, phys >> PAGE_SHIFT, vsize, rc);

	if (rc)
		return -EAGAIN;
	return 0;
}

static ssize_t char_ctrl_read(struct file *file, char __user *buf, size_t count,
		loff_t *pos)
{
	u32 w;
	int ret;
	void *reg;
	struct xdma_dev *lro;
	struct xdma_char *lro_char = (struct xdma_char *)file->private_data;

	BUG_ON(!lro_char);
	BUG_ON(lro_char->magic != MAGIC_CHAR);
	lro = lro_char->lro;
	BUG_ON(!lro);
	BUG_ON(lro->magic != MAGIC_DEVICE);

	/* only 32-bit aligned and 32-bit multiples */
	if (*pos & 3)
		return -EPROTO;
	/* first address is BAR base plus file position offset */
	reg = lro->bar[lro_char->bar] + *pos;
	w = read_register(reg);
	dbg_sg("char_ctrl_read(@%p, count=%ld, pos=%d) value = 0x%08x\n", reg,
		(long)count, (int)*pos, w);
	ret = copy_to_user(buf, &w, 4);
	if (ret)
		dbg_sg("Copy to userspace failed but continuing\n");

	*pos += 4;
	return 4;
}

static ssize_t char_ctrl_write(struct file *file, const char __user *buf,
			size_t count, loff_t *pos)
{
	u32 w;
	int ret;
	void *reg;
	struct xdma_dev *lro;
	struct xdma_char *lro_char = (struct xdma_char *)file->private_data;

	BUG_ON(!lro_char);
	BUG_ON(lro_char->magic != MAGIC_CHAR);
	lro = lro_char->lro;
	BUG_ON(!lro);
	BUG_ON(lro->magic != MAGIC_DEVICE);

	/* only 32-bit aligned and 32-bit multiples */
	if (*pos & 3)
		return -EPROTO;

	/* first address is BAR base plus file position offset */
	reg = lro->bar[lro_char->bar] + *pos;
	ret = copy_from_user(&w, buf, 4);
	if (ret)
		dbg_sg("Copy from user failed but continuing\n");

	dbg_sg("char_ctrl_write(0x%08x @%p, count=%ld, pos=%d)\n", w, reg,
		(long)count, (int)*pos);
	write_register(w, reg);
	*pos += 4;
	return 4;
}

/*
 * character device file operations for events
 */
static ssize_t char_events_read(struct file *file, char __user *buf,
		size_t count, loff_t *pos)
{
	int rc;
	int ret;
	struct xdma_irq *user_irq;
	struct xdma_char *lro_char = (struct xdma_char *)file->private_data;
	u32 events_user;
	unsigned long flags;

	BUG_ON(!lro_char);
	BUG_ON(lro_char->magic != MAGIC_CHAR);
	user_irq = lro_char->user_irq;
	BUG_ON(!user_irq);

	if (count != 4)
		return -EPROTO;

	if (*pos & 3)
		return -EPROTO;

	/*
	 * sleep until any interrupt events have occurred,
	 * or a signal arrived
	 */
	rc = wait_event_interruptible(user_irq->events_wq,
			user_irq->events_irq != 0);
	if (rc)
		dbg_sg("wait_event_interruptible=%d\n", rc);

	/* wait_event_interruptible() was interrupted by a signal */
	if (rc == -ERESTARTSYS)
		return -ERESTARTSYS;

	/* atomically decide which events are passed to the user */
	spin_lock_irqsave(&user_irq->events_lock, flags);
	events_user = user_irq->events_irq;
	user_irq->events_irq = 0;
	spin_unlock_irqrestore(&user_irq->events_lock, flags);

	ret = copy_to_user(buf, &events_user, 4);
	if (ret)
		dbg_sg("Copy to user failed but continuing\n");

	return 4;
}

static unsigned int char_events_poll(struct file *file, poll_table *wait)
{
	struct xdma_irq *user_irq;
	struct xdma_char *lro_char = (struct xdma_char *)file->private_data;
	unsigned long flags;
	unsigned int mask = 0;

	BUG_ON(!lro_char);
	BUG_ON(lro_char->magic != MAGIC_CHAR);
	user_irq = lro_char->user_irq;
	BUG_ON(!user_irq);

	poll_wait(file, &user_irq->events_wq,  wait);

	spin_lock_irqsave(&user_irq->events_lock, flags);
	if (user_irq->events_irq)
		mask = POLLIN | POLLRDNORM;	/* readable */

	spin_unlock_irqrestore(&user_irq->events_lock, flags);

	return mask;
}

static int copy_desc_data(struct xdma_transfer *transfer, char __user *buf,
	size_t *buf_offset, size_t buf_size)
{
	int i;
	int copy_err;
	int rc = 0;

	BUG_ON(!buf);
	BUG_ON(!buf_offset);

	/* Fill user buffer with descriptor data */
	for (i = 0; i < transfer->desc_num; i++) {
		if (*buf_offset + sizeof(struct xdma_desc) <= buf_size) {
			copy_err = copy_to_user(&buf[*buf_offset],
				transfer->desc_virt + i,
				sizeof(struct xdma_desc));

			if (copy_err) {
				dbg_sg("Copy to user buffer failed\n");
				*buf_offset = buf_size;
				rc = -EINVAL;
			} else {
				*buf_offset += sizeof(struct xdma_desc);
			}
		} else {
			rc = -ENOMEM;
		}
	}

	return rc;
}

static ssize_t char_bypass_read(struct file *file, char __user *buf,
		size_t count, loff_t *pos)
{
	struct xdma_dev *lro;
	struct xdma_engine *engine;
	struct xdma_char *lro_char = (struct xdma_char *)file->private_data;
	struct xdma_transfer *transfer;
	struct list_head *idx;
	size_t buf_offset = 0;
	int rc = 0;

	BUG_ON(!lro_char);
	BUG_ON(lro_char->magic != MAGIC_CHAR);

	lro = lro_char->lro;
	BUG_ON(!lro);
	BUG_ON(lro->magic != MAGIC_DEVICE);

	engine = lro_char->engine;
	BUG_ON(!engine);
	BUG_ON(engine->magic != MAGIC_ENGINE);

	dbg_sg("In char_bypass_read()\n");

	if (count & 3) {
		dbg_sg("Buffer size must be a multiple of 4 bytes\n");
		return -EINVAL;
	}

	if (!buf) {
		dbg_sg("Caught NULL pointer\n");
		return -EINVAL;
	}

	if (lro->bypass_bar_idx < 0) {
		dbg_sg("Bypass BAR not present - unsupported operation\n");
		return -ENODEV;
	}

	spin_lock(&engine->lock);

	if (!list_empty(&engine->transfer_list)) {
		list_for_each(idx, &engine->transfer_list) {
			transfer = list_entry(idx, struct xdma_transfer, entry);

			rc = copy_desc_data(transfer, buf, &buf_offset, count);
		}
	}

	spin_unlock(&engine->lock);

	if (rc < 0)
		return rc;
	else
		return buf_offset;
}

static ssize_t char_bypass_write(struct file *file, const char __user *buf,
		size_t count, loff_t *pos)
{
	struct xdma_dev *lro;
	struct xdma_engine *engine;
	struct xdma_char *lro_char = (struct xdma_char *)file->private_data;

	u32 desc_data;
	u32 *bypass_addr;
	size_t buf_offset = 0;
	int rc = 0;
	int copy_err;

	BUG_ON(!lro_char);
	BUG_ON(lro_char->magic != MAGIC_CHAR);

	lro = lro_char->lro;
	BUG_ON(!lro);
	BUG_ON(lro->magic != MAGIC_DEVICE);

	engine = lro_char->engine;
	BUG_ON(!engine);
	BUG_ON(engine->magic != MAGIC_ENGINE);

	if (count & 3) {
		dbg_sg("Buffer size must be a multiple of 4 bytes\n");
		return -EINVAL;
	}

	if (!buf) {
		dbg_sg("Caught NULL pointer\n");
		return -EINVAL;
	}

	if (lro->bypass_bar_idx < 0) {
		dbg_sg("Bypass BAR not present - unsupported operation\n");
		return -ENODEV;
	}

	dbg_sg("In char_bypass_write()\n");

	spin_lock(&engine->lock);

	/* Write descriptor data to the bypass BAR */
	bypass_addr = (u32 *)lro->bar[lro->bypass_bar_idx];
	bypass_addr += engine->bypass_offset;
	while (buf_offset < count) {
		copy_err = copy_from_user(&desc_data, &buf[buf_offset],
			sizeof(u32));
		if (!copy_err) {
			write_register(desc_data, bypass_addr);
			buf_offset += sizeof(u32);
			rc = buf_offset;
		} else {
			dbg_sg("Error reading data from userspace buffer\n");
			rc = -EINVAL;
			break;
		}
	}

	spin_unlock(&engine->lock);


	return rc;
}

static int destroy_sg_char(struct xdma_char *lro_char)
{
	BUG_ON(!lro_char);
	BUG_ON(lro_char->magic != MAGIC_CHAR);
	BUG_ON(!lro_char->lro);
	BUG_ON(!g_xdma_class);
	BUG_ON(!lro_char->sys_device);

	if (lro_char->sys_device)
		device_destroy(g_xdma_class, lro_char->cdevno);

	cdev_del(&lro_char->cdev);
	unregister_chrdev_region(lro_char->cdevno, 1);
	kfree(lro_char);

	return 0;
}

static int gen_dev_major(struct xdma_char *lro_char)
{
	int rc;
	int selected;

	/* dynamically pick a number into cdevno */
	if (major == 0) {
		/* allocate a dynamically allocated character device node */
		rc = alloc_chrdev_region(&lro_char->cdevno, XDMA_MINOR_BASE,
			XDMA_MINOR_COUNT, DRV_NAME);

		selected = MAJOR(lro_char->cdevno);
		dbg_init("Dynamic allocated major=%d, rc=%d\n", selected, rc);
	} else {
		selected = major--;
		dbg_init("Reusing allocated major=%d\n", selected);
	}

	return selected;
}

static int gen_dev_minor(struct xdma_engine *engine, enum chardev_type type,
		int event_id)
{
	int minor;
	int tmp;

	if ((type == CHAR_XDMA_H2C) || (type == CHAR_XDMA_C2H)) {
		BUG_ON(!engine);
		BUG_ON(engine->number_in_channel >= 2);
		tmp = engine->number_in_channel * 4;
		minor = 32 + tmp + engine->channel;
	} else if ((type == CHAR_BYPASS_H2C) || (type == CHAR_BYPASS_C2H)) {
		BUG_ON(!engine);
		BUG_ON(engine->number_in_channel >= 2);
		tmp = engine->number_in_channel * 4;
		minor = 64 + tmp + engine->channel;
	} else if (type == CHAR_BYPASS) { 
                minor = 100;
	} else if (type == CHAR_EVENTS) {
		minor = 10 + event_id;
	} else {
		/* minor number is type index for non-SGDMA interfaces */
		minor = type;
	}

	return minor;
}

static const struct file_operations *select_file_ops(enum chardev_type type)
{
	const struct file_operations *fops;

	switch (type) {
	case CHAR_XDMA_H2C:
	case CHAR_XDMA_C2H:
		if (poll_mode)
			fops = &sg_polled_fops;
		else
			fops = &sg_interrupt_fops;
		break;

	case CHAR_USER:
	case CHAR_CTRL:
		fops = &ctrl_fops;
		break;

	case CHAR_EVENTS:
		fops = &events_fops;
		break;

	case CHAR_BYPASS_H2C:
	case CHAR_BYPASS_C2H:
	case CHAR_BYPASS:
		fops = &bypass_fops;
		break;

	default:
		dbg_init("Invalid device type specified\n");
		fops = NULL;
		break;
	}

	return fops;
}

static int config_kobject(struct xdma_char *lro_char, enum chardev_type type)
{
	int rc = 0;
	struct xdma_dev *lro;
	struct xdma_engine *engine;

	BUG_ON(!lro_char);

	lro = lro_char->lro;
	BUG_ON(!lro);

	engine = lro_char->engine;

	switch (type) {
	case CHAR_XDMA_H2C:
	case CHAR_XDMA_C2H:
	case CHAR_BYPASS_H2C:
	case CHAR_BYPASS_C2H:
		BUG_ON(!engine);
		rc = kobject_set_name(&lro_char->cdev.kobj, devnode_names[type],
			lro->instance, engine->channel);
		break;
	case CHAR_BYPASS:
	case CHAR_USER:
	case CHAR_CTRL:
		rc = kobject_set_name(&lro_char->cdev.kobj, devnode_names[type],
			lro->instance);
		break;

	case CHAR_EVENTS:
		rc = kobject_set_name(&lro_char->cdev.kobj, devnode_names[type],
			lro->instance, lro_char->bar);
		break;

	default:
		dbg_init("Invalid device type\n");
		rc = -1;
		break;
	}

	return rc;
}

static int create_dev(struct xdma_char *lro_char, enum chardev_type type)
{
	struct xdma_dev *lro;
	struct xdma_engine *engine;
	int last_param;
	int rc = 0;

	BUG_ON(!lro_char);

	lro = lro_char->lro;
	BUG_ON(!lro);

	engine = lro_char->engine;

	if (type == CHAR_EVENTS)
		last_param = lro_char->bar;
	else
		last_param = engine ? engine->channel : 0;

	lro_char->sys_device = device_create(g_xdma_class, &lro->pci_dev->dev,
		lro_char->cdevno, NULL, devnode_names[type], lro->instance,
		last_param);
	if (!lro_char->sys_device) {
		dbg_init("device_create(%s) failed\n", devnode_names[type]);
		rc = -1;
	}

	return rc;
}

/* create_char() -- create a character device interface to data or control bus
 *
 * If at least one SG DMA engine is specified, the character device interface
 * is coupled to the SG DMA file operations which operate on the data bus. If
 * no engines are specified, the interface is coupled with the control bus.
 */
static struct xdma_char *create_sg_char(struct xdma_dev *lro, int bar,
		struct xdma_engine *engine, enum chardev_type type)
{
	struct xdma_char *lro_char;
	int rc;
	int minor;
	const struct file_operations *fops;

	BUG_ON(!lro);
	BUG_ON(type < 0);
	BUG_ON(type > ARRAY_SIZE(devnode_names));

	dbg_init(DRV_NAME "(lro = 0x%p, engine = 0x%p)\n", lro, engine);
	/* at least one engine must be specified */
	/* allocate book keeping data structure */
	lro_char = kzalloc(sizeof(struct xdma_char), GFP_KERNEL);
	if (!lro_char)
		return NULL;

	lro_char->magic = MAGIC_CHAR;

	/* new instance? */
	if (lro->major == 0) {
		int i = 0;
		while((dev_present[i] == 1) && (i < MAX_XDMA_DEVICES)){
			i++;
		}
		if(i == MAX_XDMA_DEVICES){
			printk(KERN_DEBUG "Device limit reached\n");
			goto fail_alloc;
		}
		dev_present[i] = 1;
		lro->instance = i;

		lro->major = gen_dev_major(lro_char);
	}

	minor = gen_dev_minor(engine, type, bar);

	lro_char->cdevno = MKDEV(lro->major, minor);
	/*
	 * do not register yet, create kobjects and name them,
	 * re-use the name during single-minor registration
	 */

	fops = select_file_ops(type);
	if (!fops) {
		dbg_init("File interface selection failed\n");
		goto fail_fops;
	}

	cdev_init(&lro_char->cdev, fops);

	lro_char->cdev.owner = THIS_MODULE;
	lro_char->lro = lro;
	lro_char->engine = engine;
	lro_char->bar = bar;

	if (type == CHAR_EVENTS)
		lro_char->user_irq = &lro->user_irq[bar];

	rc = config_kobject(lro_char, type);
	if (rc) {
		dbg_init("kobject configuration failed\n");
		goto fail_kobj;
	}

	rc = 0;
	if (major) {
		dbg_init("register_chrdev_region(%s)\n",
			lro_char->cdev.kobj.name);
		rc = register_chrdev_region(lro_char->cdevno, 1,
				lro_char->cdev.kobj.name);
	}

	if (rc < 0) {
		dbg_init("register_chrdev_region()=%d failed\n", rc);
		goto fail_alloc;
	}

	/* bring character device live */
	rc = cdev_add(&lro_char->cdev, lro_char->cdevno, XDMA_MINOR_COUNT);
	if (rc < 0) {
		dbg_init("cdev_add() = %d\n", rc);
		goto fail_add;
	}
	/* create device on our class */
	if (g_xdma_class) {
		rc = create_dev(lro_char, type);

		if (rc < 0)
			goto fail_device;
	}

	goto success;

fail_device:
	cdev_del(&lro_char->cdev);
fail_add:
	unregister_chrdev_region(lro_char->cdevno, XDMA_MINOR_COUNT);
fail_alloc:
fail_fops:
fail_kobj:
	kfree(lro_char);
	lro_char = NULL;
success:

	return lro_char;
}

static int __init xdma_init(void)
{
	int rc = 0;
	int i;

	pr_info(DRV_NAME " v" DRV_MODULE_VERSION "\n");

	dbg_init(DRV_NAME " init()\n");
	/* dbg_init(DRV_NAME " built " __DATE__ " " __TIME__ "\n"); */
	g_xdma_class = class_create(THIS_MODULE, DRV_NAME);
	if (IS_ERR(g_xdma_class)) {
		dbg_init(DRV_NAME ": failed to create class");
		rc = -1;
		goto err_class;
	}
	rc = pci_register_driver(&pci_driver);

	for(i=0;i<MAX_XDMA_DEVICES;i++){
		dev_present[i] = 0;
	}
err_class:
	return rc;
}

static void __exit xdma_exit(void)
{
	dbg_init(DRV_NAME" exit()\n");
	/* unregister this driver from the PCI bus driver */
	pci_unregister_driver(&pci_driver);
	if (g_xdma_class)
		class_destroy(g_xdma_class);
}

module_init(xdma_init);
module_exit(xdma_exit);
