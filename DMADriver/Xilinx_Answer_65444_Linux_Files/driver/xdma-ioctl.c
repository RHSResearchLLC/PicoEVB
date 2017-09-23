/**
 *  Copyright (C) 2015 Xilinx, Inc. All rights reserved.
 *  Author: Sonal Santan
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/firmware.h>
#include <linux/pci.h>
#include <linux/vmalloc.h>


#include "xdma-core.h"
#include "xdma-ioctl.h"
#include "xbar_sys_parameters.h"

#if SD_ACCEL
/*
 * Precomputed table with divide and divide_fractional values together
 * with target frequency. The steps are 25 MHz apart, the fractional
 * portion have been pre-multiplied with 1000.
 */
const static struct xdma_ocl_clockwiz frequency_table[] = {
		{25, 0x28, 0x0},
		{50, 0x14, 0x0},
		{75, 0xd, 0x14d},
		{100, 0xa, 0x0},
		{125, 0x8, 0x0},
		{150, 0x6, 0x29a},
		{175, 0x5, 0x2ca},
		{200, 0x5, 0x0},
		{225, 0x4, 0x1bc},
		{250, 0x4, 0x0},
		{275, 0x3, 0x27c},
		{300, 0x3, 0x14d},
		{325, 0x3, 0x4c},
		{350, 0x2, 0x359},
		{375, 0x2, 0x29a},
		{400, 0x2, 0x1f4}
};

static int reinit(struct xdma_dev *lro)
{
	int rc = 0;
	int dir_from_dev;
	int channel;
	struct xdma_engine *engine = NULL;

	/* Renable the interrupts */
	rc = interrupts_enable(lro, XDMA_OFS_INT_CTRL, 0x00ffffffUL);
	for (dir_from_dev = 0; dir_from_dev < 2; dir_from_dev++) {
		/* Re-init all the channels */
		for (channel = 0; channel < XDMA_CHANNEL_NUM_MAX; channel++) {
			engine = lro->engine[channel][dir_from_dev];
			if (engine)
				engine_reinit(engine);
		}
	}
	return rc;
}


static unsigned compute_unit_busy(struct xdma_dev *lro)
{
	int i = 0;
	unsigned result = 0;
	u32 r = ioread32(lro->bar[lro->user_bar_idx] + AXI_GATE_OFFSET_READ);

	/* r != 0x3 implies that OCL region is isolated and we cannot read CUs' status */
	if (r != 0x3)
	    return 0;

	for (i = 0; i < 16; i++) {
		r = ioread32(lro->bar[lro->user_bar_idx] + OCL_CTLR_OFFSET + i * OCL_CU_CTRL_RANGE);
		if (r == 0x1)
		    result |= (r << i);
	}
	return result;
}


void freezeAXIGate(struct xdma_dev *lro)
{
	u8 w = 0x0;
	u32 t;

	BUG_ON(lro->axi_gate_frozen);
//	printk(KERN_DEBUG "IOCTL %s:%d\n", __FILE__, __LINE__);
	t = ioread32(lro->bar[lro->user_bar_idx] + AXI_GATE_OFFSET_READ);
//	printk("Register %x\n", t);
	iowrite8(w, lro->bar[lro->user_bar_idx] + AXI_GATE_OFFSET);
	t = ioread32(lro->bar[lro->user_bar_idx] + AXI_GATE_OFFSET_READ);
//	printk("Register %x\n", t);
	lro->axi_gate_frozen = 1;
	printk(KERN_DEBUG "%s: Froze AXI gate\n", DRV_NAME);
}

void freeAXIGate(struct xdma_dev *lro)
{
	/*
	 * First pulse the OCL RESET. This is important for PR with multiple
	 * clocks as it resets the edge triggered clock converter FIFO
	 */
	u8 w = 0x2;
	u32 t;

	BUG_ON(!lro->axi_gate_frozen);
//	printk(KERN_DEBUG "IOCTL %s:%d\n", __FILE__, __LINE__);
	t = ioread32(lro->bar[lro->user_bar_idx] + AXI_GATE_OFFSET_READ);
//	printk("Register %x\n", t);
	iowrite8(w, lro->bar[lro->user_bar_idx] + AXI_GATE_OFFSET);
	ndelay(500);

	w = 0x0;
	t = ioread32(lro->bar[lro->user_bar_idx] + AXI_GATE_OFFSET_READ);
//	printk("Register %x\n", t);
	iowrite8(w, lro->bar[lro->user_bar_idx] + AXI_GATE_OFFSET);
	ndelay(500);

	w = 0x2;
	t = ioread32(lro->bar[lro->user_bar_idx] + AXI_GATE_OFFSET_READ);
//	printk("Register %x\n", t);
	iowrite8(w, lro->bar[lro->user_bar_idx] + AXI_GATE_OFFSET);
	ndelay(500);

	w = 0x3;
	t = ioread32(lro->bar[lro->user_bar_idx] + AXI_GATE_OFFSET_READ);
//	printk("Register %x\n", t);
	iowrite8(w, lro->bar[lro->user_bar_idx] + AXI_GATE_OFFSET);
	ndelay(500);
	t = ioread32(lro->bar[lro->user_bar_idx] + AXI_GATE_OFFSET_READ);
//	printk("Register %x\n", t);
	lro->axi_gate_frozen = 0;
	printk(KERN_DEBUG "%s: Un-froze AXI gate\n", DRV_NAME);
}

static u64 get_ocl_frequency(const struct xdma_dev *lro)
{
	u32 val;
	const u64 input = (lro->pci_dev->device == 0x8138) ? XDMA_KU3_INPUT_FREQ : XDMA_7V3_INPUT_FREQ;
	u32 mul0, div0;
	u32 mul_frac0 = 0;
	u32 div1;
	u32 div_frac1 = 0;
	u64 freq;

	val = ioread32(lro->bar[lro->user_bar_idx] + OCL_CLKWIZ_STATUS);
	printk(KERN_INFO "%s: ClockWiz SR %x\n", DRV_NAME, val);
	if ((val & 1) == 0)
		return 0;

	val = ioread32(lro->bar[lro->user_bar_idx] + OCL_CLKWIZ_CONFIG(0));
	printk(KERN_INFO "%s: ClockWiz CONFIG(0) %x\n", DRV_NAME, val);

	div0 = val & 0xff;
	mul0 = (val & 0xff00) >> 8;
	if (val & BIT(26)) {
		mul_frac0 = val >> 16;
		mul_frac0 &= 0x3ff;
	}

	/*
	 * Multiply both numerator (mul0) and the denominator (div0) with 1000 to
	 * account for fractional portion of multiplier
	 */
	mul0 *= 1000;
	mul0 += mul_frac0;
	div0 *= 1000;

	val = ioread32(lro->bar[lro->user_bar_idx] + OCL_CLKWIZ_CONFIG(2));
	printk(KERN_INFO "%s: ClockWiz CONFIG(2) %x\n", DRV_NAME, val);
	div1 = val &0xff;
	if (val & BIT(18)) {
		div_frac1 = val >> 8;
		div_frac1 &= 0x3ff;
	}

	/*
	 * Multiply both numerator (mul0) and the denominator (div1) with 1000 to
	 * account for fractional portion of divider
	 */

	div1 *= 1000;
	div1 += div_frac1;
	div0 *= div1;
	mul0 *= 1000;
	if (div0 == 0) {
		printk(KERN_ERR "%s: ClockWiz Invalid divider 0\n", DRV_NAME);
		return 0;
	}
	freq = (input * mul0)/div0;
	printk(KERN_INFO "%s: ClockWiz OCL Frequency %lld\n", DRV_NAME, freq);
	return freq;
}

static long link_info(const struct xdma_dev *lro, struct xdma_ioc_info *obj)
{
#ifdef RTO
    u16 stat;
    long result;

    obj->pcie_link_width = 0;
    obj->pcie_link_speed = 0;
    result = pcie_capability_read_word(lro->pci_dev, PCI_EXP_LNKSTA, &stat);
    if (result)
        return result;
    obj->pcie_link_width = (stat & PCI_EXP_LNKSTA_NLW) >> PCI_EXP_LNKSTA_NLW_SHIFT;
    obj->pcie_link_speed = stat & PCI_EXP_LNKSTA_CLS;
#endif
    return 0;
}

static long version_ioctl(struct xdma_char *lro_char, void __user *arg)
{
	struct xdma_ioc_info obj;
	struct xdma_dev *lro = lro_char->lro;

	if (copy_from_user((void *)&obj, arg, sizeof(struct xdma_ioc_info)))
		return -EFAULT;
	memset(&obj, 0, sizeof(obj));
	obj.vendor = lro->pci_dev->vendor;
	obj.device = lro->pci_dev->device;
	obj.subsystem_vendor = lro->pci_dev->subsystem_vendor;
	obj.subsystem_device = lro->pci_dev->subsystem_device;
	obj.feature_id = lro->feature_id;
	obj.driver_version = 0x10000;
    obj.ocl_frequency = get_ocl_frequency(lro);
    link_info(lro, &obj);
	if (copy_to_user(arg, &obj, sizeof(struct xdma_ioc_info)))
		return -EFAULT;
	return 0;
}

static long reset_ocl_ioctl(struct xdma_char *lro_char)
{
	struct xdma_dev *lro = lro_char->lro;

	/* If compute units are not busy then nothing to do */
	if (!compute_unit_busy(lro))
		return 0;

	freezeAXIGate(lro);
	freeAXIGate(lro);
	return compute_unit_busy(lro) ? -EBUSY : 0;
}


/**
 * Simple implementation of device reset using PCIe hot reset
 * Toggle a special bit in the PCI_MIN_GNT config byte of connected
 * root port to reset the card except for the EP which stays up.
 * There is no support yet for quashing any pending DMA transactions,
 * and returning EIO for pending DMA read/writes, etc. More features
 * will be added incrementally. Note this does not reset the PCIe link.
 */

static long reset_hot_ioctl(struct xdma_char *lro_char)
{
	u32 *pci_cfg = NULL;
	u32 pci_cfg2[0x40];
	u8 hot;
	int i;
	long err = 0;
	const char *ep_name;
	const char *rp_name;
	struct xdma_dev *lro = lro_char->lro;
	struct pci_dev *pdev = lro->pci_dev;

	BUG_ON(!pdev->bus);
	BUG_ON(!pdev->bus->self);

	if (!pdev->bus || !pdev->bus->self) {
		printk(KERN_ERR "%s: Unable to identify device root port for card %d\n", DRV_NAME,
		       lro->instance);
		err = -EIO;
		goto done;
	}

	ep_name = pdev->bus->name;
#if defined(__PPC64__)
	printk(KERN_INFO "%s: Ignoring reset operation for card %d in slot %s:%02x:%1x\n", DRV_NAME, lro->instance, ep_name,
	       PCI_SLOT(pdev->devfn), PCI_FUNC(pdev->devfn));
#else
	printk(KERN_INFO "%s: Trying to reset card %d in slot %s:%02x:%1x\n", DRV_NAME, lro->instance, ep_name,
	       PCI_SLOT(pdev->devfn), PCI_FUNC(pdev->devfn));

	/* Allocate buffer for 256B PCIe config space */
	pci_cfg = kmalloc(0x100, GFP_KERNEL);
	if (!pci_cfg)
		return -ENOMEM;

	rp_name = pdev->bus->parent ? pdev->bus->parent->name : "null";
	/* Save the card's PCIe config space */

	for (i = 0; i < 0x100; i += 4) {
		pci_read_config_dword(pdev, i, &pci_cfg[i/4]);
	}

	pci_read_config_byte(pdev->bus->self, PCI_MIN_GNT, &hot);

	/* Toggle the PCIe hot reset bit in the root port */
	pci_write_config_byte(pdev->bus->self, PCI_MIN_GNT, hot | 0x40);

	ssleep(1);

	pci_write_config_byte(pdev->bus->self, PCI_MIN_GNT, hot);

	ssleep(1);

	/* Restore the card's PCIe config space */
	for (i = 0; i < 0x100; i += 4)
		pci_write_config_dword(pdev, i, pci_cfg[i/4]);

	ssleep(1);

	/* Verify we were able to restore card's PCIe config space */
	for (i = 0; i < 0x100; i += 4) {
		pci_read_config_dword(pdev, i, &pci_cfg2[i/4]);
		if (pci_cfg2[i/4] != pci_cfg[i/4])
			printk(KERN_WARNING "%s: Unable to restore config dword at %x (%x->%x) for card %d in slot %s:%02x:%1x\n",
			       DRV_NAME, i, pci_cfg[i/4], pci_cfg2[i/4], lro->instance, ep_name, PCI_SLOT(pdev->devfn),
			       PCI_FUNC(pdev->devfn));
	}

	err = reinit(lro);
	ssleep(1);
#endif
done:
	kfree(pci_cfg);
	return err;
}

/*
 * Based on Clocking Wizard v5.1, section Dynamic Reconfiguration through AXI4-Lite
 */
static long ocl_freqscaling_ioctl(struct xdma_char *lro_char, void __user *arg)
{
	struct xdma_ioc_freqscaling obj;
	struct xdma_dev *lro = lro_char->lro;
	long err = 0;
	int idx = 0;
	u32 val = 0;
        int i = 0;
	/* Divide by 1; multiply by 5 for Utrascale and 10 for V7 */
	u32 config = (lro->pci_dev->device == 0x8138) ? 0x04000501 : 0x04000a01;

	if (copy_from_user((void *)&obj, arg, sizeof(struct xdma_ioc_freqscaling)))
		return -EFAULT;

	val = ioread32(lro->bar[lro->user_bar_idx] + OCL_CLKWIZ_STATUS);
	if ((val & 0x1) == 0)
		return -EBUSY;

	if (compute_unit_busy(lro))
		return -EBUSY;

	if ((obj.ocl_target_freq > 250) || (obj.ocl_target_freq < frequency_table[0].ocl))
		return -EINVAL;

        get_ocl_frequency(lro);

	idx = obj.ocl_target_freq / frequency_table[0].ocl;
        idx--;
	/*
         * TODO:
	 * Need to lock the device so that another thread is not fiddling with the device at
	 * the same time, like downloading bitstream or starting kernel, etc.
	 */
	freezeAXIGate(lro);
	val = ioread32(lro->bar[lro->user_bar_idx] + OCL_CLKWIZ_CONFIG(0));
	printk(KERN_INFO "%s: ClockWiz CONFIG(0) %x\n", DRV_NAME, val);

        val = ioread32(lro->bar[lro->user_bar_idx] + OCL_CLKWIZ_CONFIG(2));
	printk(KERN_INFO "%s: ClockWiz CONFIG(2) %x\n", DRV_NAME, val);

        val = ioread32(lro->bar[lro->user_bar_idx] + OCL_CLKWIZ_STATUS);
        if (val != 1) {
                err = -EBUSY;
                goto done;
        }

        iowrite32(config, lro->bar[lro->user_bar_idx] + OCL_CLKWIZ_CONFIG(0));
        config = frequency_table[idx].divide_frac;
        config <<= 8;
        if (config) /* Enable divide fraction bit */
                config &= (0x1 << 18);
        config |= frequency_table[idx].divide;
        iowrite32(config, lro->bar[lro->user_bar_idx] + OCL_CLKWIZ_CONFIG(2));
        msleep(10);
        iowrite32(0x00000007, lro->bar[lro->user_bar_idx] + OCL_CLKWIZ_CONFIG(23));
        msleep(10);
        iowrite32(0x00000002, lro->bar[lro->user_bar_idx] + OCL_CLKWIZ_CONFIG(23));
        printk(KERN_INFO "%s: ClockWiz Waiting for locked signal\n", DRV_NAME);
        msleep(100);
        for (i = 0; i < 1000; i++) {
                val = ioread32(lro->bar[lro->user_bar_idx] + OCL_CLKWIZ_STATUS);
                if (val != 1) {
                        msleep(100);
                        continue;
                }
        }
        if (val != 1) {
                printk(KERN_ERR "%s: ClockWiz MMCM/PLL did not lock after 100 * 1000 ms, restoring the original configuration\n", DRV_NAME);
                /* restore the original clock configuration */
                iowrite32(0x00000004, lro->bar[lro->user_bar_idx] + OCL_CLKWIZ_CONFIG(23));
                msleep(10);
                iowrite32(0x00000000, lro->bar[lro->user_bar_idx] + OCL_CLKWIZ_CONFIG(23));
                err = -EINVAL;
        }

done:
        get_ocl_frequency(lro);
	freeAXIGate(lro);
	return err;
}


long char_ctrl_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct xdma_dev *lro;
	struct xdma_char *lro_char = (struct xdma_char *)filp->private_data;
	struct xdma_ioc_base ioctl_obj;
	long result = 0;
	BUG_ON(!lro_char);
	BUG_ON(lro_char->magic != MAGIC_CHAR);
	lro = lro_char->lro;
	BUG_ON(!lro);
	BUG_ON(lro->magic != MAGIC_DEVICE);

	printk(KERN_DEBUG "IOCTL %s:%d Command: %x\n", __FILE__, __LINE__, cmd);

	if (lro_char != lro->user_char_dev)
		return -ENOTTY;

	if (_IOC_TYPE(cmd) != XDMA_IOC_MAGIC)
		return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ)
		result = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		result =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));

	if (result)
		return -EFAULT;

	printk(KERN_DEBUG "IOCTL %s:%d\n", __FILE__, __LINE__);
	if (copy_from_user((void *)&ioctl_obj, (void *) arg, sizeof(struct xdma_ioc_base)))
		return -EFAULT;
	if (ioctl_obj.magic != XDMA_XCL_MAGIC)
		return -ENOTTY;
	printk(KERN_DEBUG "IOCTL %s:%d\n", __FILE__, __LINE__);
	switch (cmd) {
	case XDMA_IOCINFO:
		return version_ioctl(lro_char, (void __user *)arg);
	case XDMA_IOCICAPDOWNLOAD:
	case XDMA_IOCMCAPDOWNLOAD:
		return bitstream_ioctl(lro_char, cmd, (void __user *)arg);
	case XDMA_IOCOCLRESET:
		return reset_ocl_ioctl(lro_char);
	case XDMA_IOCHOTRESET:
		return reset_hot_ioctl(lro_char);
	case XDMA_IOCFREQSCALING:
		return ocl_freqscaling_ioctl(lro_char, (void __user *)arg);
	default:
		return -ENOTTY;
	}
	return 0;
}


long reset_device_if_running(struct xdma_dev *lro)
{
	/* If compute units are not busy then nothing to do */
	if (!compute_unit_busy(lro))
		return 0;

	/* If one or more compute units are busy then try to reset the card */
	printk(KERN_INFO "%s: One or more compute units busy\n", DRV_NAME);

	if (reset_ocl_ioctl(lro->user_char_dev) == 0)
		return 0;
	return reset_hot_ioctl(lro->user_char_dev);
}

#endif /* SD_ACCEL */

