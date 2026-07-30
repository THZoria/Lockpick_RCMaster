/*
 * Copyright (c) 2022 shchmue
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cal0_read.h"

#include <gfx_utils.h>
#include <sec/se.h>
#include <sec/se_t210.h>
#include "../storage/emummc.h"
#include <storage/emmc.h>
#include <utils/util.h>

static const u16 crc16_table16[16] = {
	0x0000, 0xCC01, 0xD801, 0x1400,
	0xF001, 0x3C00, 0x2800, 0xE401,
	0xA001, 0x6C00, 0x7800, 0xB401,
	0x5000, 0x9C01, 0x8801, 0x4400
};

/* ----- CRC16 (CAL0) implementation (ported from switchbrew/wiki/Calibration) ----- */
/* Core: continue CRC starting from `crc` */
u16 crc16_calc_continue(u16 crc, const u8 *buf, u32 len)
{
	const u8 *p = buf;
	const u8 *q = buf + len;

	for (; p < q; p++) {
		u8 oct = *p;
		crc = (crc >> 4) ^ crc16_table16[crc & 0xF] ^ crc16_table16[(oct >> 0) & 0xF];
		crc = (crc >> 4) ^ crc16_table16[crc & 0xF] ^ crc16_table16[(oct >> 4) & 0xF];
	}
	return crc;
}

u16 crc16_calc(const u8 *buf, u32 len)
{
	return crc16_calc_continue(0x55AA, buf, len);
}

static int se_aes_crypt_xts_nx(u32 tweak_ks, u32 crypt_ks, int enc, u64 sec, void *dst, void *src, u32 secsize, u32 num_secs) {
	u8 tweak[SE_AES_BLOCK_SIZE] __attribute__((aligned(4)));

	u8 *pdst = (u8 *)dst;
	u8 *psrc = (u8 *)src;

	for (u32 i = 0; i < num_secs; i++) {
		if (se_aes_crypt_xts_sec_nx(tweak_ks, crypt_ks, enc, sec + i, tweak, true, 0, pdst + secsize * i, psrc + secsize * i, secsize)) {
			return 1;
		}
	}

	return 0;
}

bool cal0_read(u32 tweak_ks, u32 crypt_ks, void *read_buffer) {
    nx_emmc_cal0_t *cal0 = (nx_emmc_cal0_t *)read_buffer;

    // Check if CAL0 was already read into this buffer
    if (cal0->magic == MAGIC_CAL0) {
        return true;
    }

    if (emummc_storage_read(NX_EMMC_CALIBRATION_OFFSET / EMMC_BLOCKSIZE, NX_EMMC_CALIBRATION_SIZE / EMMC_BLOCKSIZE, read_buffer)) {
        EPRINTF("Unable to read PRODINFO.");
        return false;
    }

    se_aes_crypt_xts_nx(tweak_ks, crypt_ks, DECRYPT, 0, read_buffer, read_buffer, XTS_CLUSTER_SIZE, NX_EMMC_CALIBRATION_SIZE / XTS_CLUSTER_SIZE);

    if (cal0->magic != MAGIC_CAL0) {
        EPRINTF("Invalid CAL0 magic. Check BIS key 0.");
        return false;
    }

    return true;
}

bool cal0_get_ssl_rsa_key(const nx_emmc_cal0_t *cal0, const void **out_key, u32 *out_key_size, const void **out_iv, u32 *out_generation) {
    const u32 ext_key_size = sizeof(cal0->ext_ssl_key_iv) + sizeof(cal0->ext_ssl_key);
    const u32 ext_key_crc_size = ext_key_size + sizeof(cal0->ext_ssl_key_ver) + sizeof(cal0->crc16_pad39);
    const u32 key_size = sizeof(cal0->ssl_key_iv) + sizeof(cal0->ssl_key);
    const u32 key_crc_size = key_size + sizeof(cal0->crc16_pad18);

    if (cal0->ext_ssl_key_crc == crc16_calc(cal0->ext_ssl_key_iv, ext_key_crc_size)) {
        *out_key = cal0->ext_ssl_key;
        *out_key_size = ext_key_size;
        *out_iv = cal0->ext_ssl_key_iv;
        // Settings sysmodule manually zeroes this out below cal version 9
        *out_generation = cal0->version <= 8 ? 0 : cal0->ext_ssl_key_ver;
    } else if (cal0->ssl_key_crc == crc16_calc(cal0->ssl_key_iv, key_crc_size)) {
        *out_key = cal0->ssl_key;
        *out_key_size = key_size;
        *out_iv = cal0->ssl_key_iv;
        *out_generation = 0;
    } else {
        EPRINTF("Crc16 error reading device key.");
        return false;
    }
    return true;
}


bool cal0_get_eticket_rsa_key(const nx_emmc_cal0_t *cal0, const void **out_key, u32 *out_key_size, const void **out_iv, u32 *out_generation) {
    const u32 ext_key_size = sizeof(cal0->ext_ecc_rsa2048_eticket_key_iv) + sizeof(cal0->ext_ecc_rsa2048_eticket_key);
    const u32 ext_key_crc_size = ext_key_size + sizeof(cal0->ext_ecc_rsa2048_eticket_key_ver) + sizeof(cal0->crc16_pad38);
    const u32 key_size = sizeof(cal0->rsa2048_eticket_key_iv) + sizeof(cal0->rsa2048_eticket_key);
    const u32 key_crc_size = key_size + sizeof(cal0->crc16_pad21);

    if (cal0->ext_ecc_rsa2048_eticket_key_crc == crc16_calc(cal0->ext_ecc_rsa2048_eticket_key_iv, ext_key_crc_size)) {
        *out_key = cal0->ext_ecc_rsa2048_eticket_key;
        *out_key_size = ext_key_size;
        *out_iv = cal0->ext_ecc_rsa2048_eticket_key_iv;
        // Settings sysmodule manually zeroes this out below cal version 9
        *out_generation = cal0->version <= 8 ? 0 : cal0->ext_ecc_rsa2048_eticket_key_ver;
    } else if (cal0->rsa2048_eticket_key_crc == crc16_calc(cal0->rsa2048_eticket_key_iv, key_crc_size)) {
        *out_key = cal0->rsa2048_eticket_key;
        *out_key_size = key_size;
        *out_iv = cal0->rsa2048_eticket_key_iv;
        *out_generation = 0;
    } else {
        EPRINTF("Crc16 error reading device key.");
        return false;
    }
    return true;
}
