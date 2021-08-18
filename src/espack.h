/******************************************************************************
 * espack.h  ---- Pack ESP to .img file
 *
 * Copyright (c) 2021, longpanda <admin@ventoy.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __ESPACK_H__
#define __ESPACK_H__

extern int verbose;

#if defined(_MSC_VER) || defined(WIN32)

#include <Windows.h>

#define debug(fmt, ...) if (verbose) printf("[V] "fmt, ##__VA_ARGS__)
#define ESP_MAX_PATH MAX_PATH

#define uint64_t UINT64
#define uint32_t UINT32
#define uint16_t UINT16
#define uint8_t  UINT8
#define int64_t INT64
#define int32_t INT32
#define int16_t INT16
#define int8_t  INT8
#define esp_remove DeleteFileA

#else
#include <unistd.h>
#include <stdint.h>
#include <linux/limits.h>
#define debug(fmt, args...) if (verbose) printf("[V] "fmt, ##args)
#define ESP_MAX_PATH PATH_MAX
#define strcpy_s(a, b, c)  strncpy(a, c, b)
#define sprintf_s snprintf
#define esp_remove remove

#endif   


#pragma pack(1)

typedef struct esp_guid
{
    uint32_t   data1;
    uint16_t   data2;
    uint16_t   data3;
    uint8_t    data4[8];
}esp_guid;

typedef struct mbr_part_table
{
    uint8_t  Active; // 0x00  0x80

    uint8_t  StartHead;
    uint16_t StartSector : 6;
    uint16_t StartCylinder : 10;

    uint8_t  FsFlag;

    uint8_t  EndHead;
    uint16_t EndSector : 6;
    uint16_t EndCylinder : 10;

    uint32_t StartSectorId;
    uint32_t SectorCount;
}mbr_part_table;

typedef struct mbr_head
{
    uint8_t BootCode[446];
    mbr_part_table PartTbl[4];
    uint8_t Byte55;
    uint8_t ByteAA;
}mbr_head;

typedef struct gpt_head
{
    char   Signature[8]; /* EFI PART */
    uint8_t  Version[4];
    uint32_t Length;
    uint32_t Crc;
    uint8_t  Reserved1[4];
    uint64_t EfiStartLBA;
    uint64_t EfiBackupLBA;
    uint64_t PartAreaStartLBA;
    uint64_t PartAreaEndLBA;
    uint8_t  DiskGuid[16];
    uint64_t PartTblStartLBA;
    uint32_t PartTblTotNum;
    uint32_t PartTblEntryLen;
    uint32_t PartTblCrc;
    uint8_t  Reserved2[420];
}gpt_head;

typedef struct gpt_part_tbl
{
    uint8_t  PartType[16];
    uint8_t  PartGuid[16];
    uint64_t StartLBA;
    uint64_t LastLBA;
    uint64_t Attr;
    uint16_t Name[36];
}gpt_part_tbl;

typedef struct gpt_info
{
    mbr_head MBR;
    gpt_head Head;
    gpt_part_tbl PartTbl[128];
}gpt_info;
#pragma pack()

#define SIZE_1KB    1024
#define SIZE_4KB    4096
#define SIZE_8KB    8192
#define SIZE_64KB   65536
#define SIZE_1MB    (1024 * 1024)
#define SIZE_4MB    (4096 * 1024)
#define SIZE_5MB    (5 * 1024 * 1024)
#define SIZE_4GB    0x100000000LL
#define SIZE_2TB    2199023255552ULL
#define FILE_BUF_SIZE   SIZE_1MB
#define MBR_APPEND_SECTOR   8

typedef int (*esp_enum_callback)(const char *path, int flag, void *data);
int esp_enum_dir(char *root, esp_enum_callback callback, void *data);
uint64_t esp_get_file_size(const char *path);
uint32_t esp_crc32(void *Buffer, uint32_t Length);
void esp_uuid(void *uuid);
void *esp_fcreate(const char *path);
void *esp_fopen(const char *path);
void esp_fflush(void *f);
void esp_fclose(void *f);
int64_t esp_fseek(void *f, int64_t offset, int whence);
size_t esp_fread(void *ptr, size_t size, size_t nmemb, void *f);
size_t esp_fwrite(const void *ptr, size_t size, size_t nmemb, void *f);
int64_t esp_ftell(void *f);

#endif /* __ESPACK_H__ */

