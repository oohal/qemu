/*
 * QEMU PowerPC PowerNV Emulation of a few OCC related registers
 *
 * Copyright (c) 2015-2017, IBM Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "qemu/osdep.h"
#include "hw/hw.h"
#include "sysemu/sysemu.h"
#include "target/ppc/cpu.h"
#include "qapi/error.h"
#include "qemu/log.h"

#include "hw/ppc/fdt.h"
#include "hw/ppc/pnv.h"
#include "hw/ppc/pnv_xscom.h"
#include "hw/ppc/pnv_occ.h"

#include <libfdt.h>

#if 0
static void pnv_occ_set_misc(PnvOCC *occ, uint64_t val)
{
    bool irq_state;
    PnvOCCClass *poc = PNV_OCC_GET_CLASS(occ);

    val &= 0xffff000000000000ull;

    occ->occmisc = val;
    irq_state = !!(val >> 63);
    pnv_psi_irq_set(occ->psi, poc->psi_irq, irq_state);
}
#endif

#define VIRTIO_BASE_ADDR 0x0003000000000000

// translate the virtio IRQ to a PSI irq
static void pnv_virtio_bounce(void *dev, int irq, int level)
{
    PnvVirtio *virtio = PNV_VIRTIO(dev);
    PnvVirtioClass *poc = PNV_VIRTIO_GET_CLASS(virtio);

    // XXX: Qemu will handle the interrupt whenever set_irq is called.
    // why do we have to call pnv_psi_irq_set() instead of just setting
    // the irq directly and have the psi set the irq handler itself?
    // I'm so confuse ;_;
    pnv_psi_irq_set(virtio->psi, poc->psi_irq_idx, level);
}

static void pnv_virtio_realize(DeviceState *dev, Error **errp)
{
    PnvVirtio *virtio = PNV_VIRTIO(dev);
    Error *local_err = NULL;
    Object *obj;

    obj = object_property_get_link(OBJECT(dev), "psi", &local_err);
    if (!obj) {
        error_propagate(errp, local_err);
        error_prepend(errp, "required link 'psi' not found: ");
        return;
    }

//    psi->qirqs = qemu_allocate_irqs(ics_simple_set_irq, ics, ics->nr_irqs);

    virtio->psi = PNV_PSI(obj);
    virtio->irq = qemu_allocate_irqs(pnv_virtio_bounce, virtio, 1);

    sysbus_create_simple("virtio-mmio", VIRTIO_BASE_ADDR, *virtio->irq);
}

void pnv_dt_virtio(void *fdt, PnvChip *chip)
{
    uint64_t reg[2];
    char *name;
    int off;

    reg[0] = cpu_to_be64(VIRTIO_BASE_ADDR);
    reg[1] = cpu_to_be64(0x2000);

    name = g_strdup_printf("virtio-mmio@%"HWADDR_PRIx, VIRTIO_BASE_ADDR);
    off = fdt_add_subnode(fdt, 0, name);
    g_free(name);

    _FDT((fdt_setprop_string(fdt, off, "compatible", "virtio,mmio")));
    _FDT((fdt_setprop(fdt, off, "reg", reg, sizeof(reg))));

    // mainly to prevent unused variable chip warnings
    _FDT((fdt_setprop_cell(fdt, off, "ibm,chip-id", cpu_to_be32(chip->chip_id))));
}

static void pnv_virtio_class_init(ObjectClass *klass, void *data)
{
    PnvVirtioClass *poc = PNV_VIRTIO_CLASS(klass);
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = pnv_virtio_realize;
    dc->desc = "PowerNV Virtio hack";

    poc->psi_irq_idx = PSIHB_IRQ_FSI;
}

static const TypeInfo pnv_virtio_type_info = {
    .name          = TYPE_PNV_VIRTIO,
    .parent        = TYPE_DEVICE,
    .instance_size = sizeof(PnvVirtio),
    .class_init    = pnv_virtio_class_init,
    .class_size    = sizeof(PnvVirtioClass),
};

static void pnv_virtio_register_types(void)
{
    type_register_static(&pnv_virtio_type_info);
}
type_init(pnv_virtio_register_types);
