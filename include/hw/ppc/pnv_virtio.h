/*
 * QEMU PowerPC PowerNV Emulation of a few VIRTIO related registers
 *
 * Copyright (c) 2015-2017, IBM Corporation.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _PPC_PNV_VIRTIO_H
#define _PPC_PNV_VIRTIO_H

#include "hw/ppc/pnv_psi.h"

#define TYPE_PNV_VIRTIO "pnv-virtio"
#define PNV_VIRTIO(obj) OBJECT_CHECK(PnvVirtio, (obj), TYPE_PNV_VIRTIO)

typedef struct PnvVirtio {
    DeviceState xd;

    PnvPsi *psi;
    qemu_irq *irq;
} PnvVirtio;

#define PNV_VIRTIO_CLASS(klass) \
     OBJECT_CLASS_CHECK(PnvVirtioClass, (klass), TYPE_PNV_VIRTIO)
#define PNV_VIRTIO_GET_CLASS(obj) \
     OBJECT_GET_CLASS(PnvVirtioClass, (obj), TYPE_PNV_VIRTIO)

typedef struct PnvVirtioClass {
    DeviceClass parent_class;

    int psi_irq_idx; // move out of the class?
} PnvVirtioClass;

#endif /* _PPC_PNV_VIRTIO_H */
