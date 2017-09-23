/**
 *  Xilinx SDAccel xclbin container definition
 *  Copyright (C) 2015, Xilinx Inc - All rights reserved
 */

#ifndef _XCLBIN_H_
#define _XCLBIN_H_

#if defined(__KERNEL__)
#include <linux/types.h>
#elif defined(__cplusplus)
#include <cstdlib>
#include <cstdint>
#else
#include <stdlib.h>
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * Container format for Xilinx bitstreams, metadata and other
     * binary blobs.
     * Every segment must be aligned at 8 byte boundary with null byte padding
     * between adjacent segments if required.
     * For segements which are not present both offset and length must be 0 in
     * the header.
     * Currently only xclbin0\0 is recognized as file magic. In future if/when file
     * format is updated the magic string will be changed to xclbin1\0 and so on.
     */
    enum XCLBIN_MODE {
        XCLBIN_FLAT,
        XCLBIN_PR,
        XCLBIN_TANDEM_STAGE2,
        XCLBIN_TANDEM_STAGE2_WITH_PR,
        XCLBIN_MODE_MAX
    };

    struct xclBin {
        char m_magic[8];                    /* should be xclbin0\0  */
        uint64_t m_length;                  /* total size of the xclbin file */
        uint64_t m_timeStamp;               /* number of seconds since epoch when xclbin was created */
        uint64_t m_version;                 /* tool version used to create xclbin */
        unsigned m_mode;                    /* XCLBIN_MODE */
        char m_nextXclBin[24];              /* Name of next xclbin file in the daisy chain */
        uint64_t m_metadataOffset;          /* file offset of embedded metadata */
        uint64_t m_metadataLength;          /* size of the embedded metdata */
        uint64_t m_primaryFirmwareOffset;   /* file offset of bitstream or emulation archive */
        uint64_t m_primaryFirmwareLength;   /* size of the bistream or emulation archive */
        uint64_t m_secondaryFirmwareOffset; /* file offset of clear bitstream if any */
        uint64_t m_secondaryFirmwareLength; /* size of the clear bitstream */
        uint64_t m_driverOffset;            /* file offset of embedded device driver if any (currently unused) */
        uint64_t m_driverLength;            /* size of the embedded device driver (currently unused) */
    };


#ifdef __cplusplus
}
#endif

#endif

