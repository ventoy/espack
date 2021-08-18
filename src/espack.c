/******************************************************************************
 * espack.c  ---- Pack ESP to .img file
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#include "fat_filelib.h"
#include "espack.h"

#define ESPACK_VERSION  "1.0"

int verbose = 0;
static char *g_file_buf = NULL;
static void *g_img_file = NULL;

static int esp_disk_read(uint32 sector, uint8 *buffer, uint32 sector_count)
{
    int64_t offset = sector;

    offset = (offset + 64) * 512LL;
    esp_fseek(g_img_file, offset, SEEK_SET);
    esp_fread(buffer, 1, sector_count * 512, g_img_file);

    return 1;
}

static int esp_disk_write(uint32 sector, uint8 *buffer, uint32 sector_count)
{
    int64_t offset = sector;

    offset = (offset + 64) * 512LL;
    esp_fseek(g_img_file, offset, SEEK_SET);
    esp_fwrite(buffer, 1, sector_count * 512, g_img_file);
    return 1;
}

static void write_zero_sector(int count)
{
    int i;
    mbr_head zero;

    memset(&zero, 0, sizeof(zero));
    for (i = 0; i < count; i++)
    {
        esp_fwrite(&zero, 1, 512, g_img_file);
    }
}

static int esp_enum_calc_back(const char *path, int flag, void *data)
{
    uint64_t len;
    uint64_t *psize = (uint64_t *)data;

    if (flag)
    {
        *psize += SIZE_64KB;
        debug("Find directory %s\n", path);
    }
    else
    {
        len = esp_get_file_size(path);
        if (len <= SIZE_4GB)
        {
            *psize += ((len + (SIZE_64KB - 1)) & ~(SIZE_64KB - 1));
            debug("Find file %s size:%lld\n", path, (long long)len);
        }
        else
        {
            debug("Find file %s size:%lld too big and skip it.\n", path, (long long)len);
        }
    }
    
    return 0;
}

static int esp_enum_copy_back(const char *path, int flag, void *data)
{
    int pos = 0;
    int len = 0;
    int tip = 0;
    uint64_t size = 0;
    uint64_t tot = 0;;
    void *fp;
    void *file;

    pos = (int)(long)data;

    if (flag)
    {
        len = fl_createdirectory(path + pos);
        printf("Create directory %s [%s] \n", path + pos, len ? "OK" : "FAIL");
    }
    else
    {
        size = esp_get_file_size(path);
        if (size > SIZE_4GB)
        {
            printf("[WARNING] <%s> is too big (4GB+) to be ignored.", path);
            return 0;
        }

        if (size > SIZE_1MB * 64)
        {
            tip = 1;
        }
    
        file = fl_fopen(path + pos, "wb");
        if (file)
        {
            fp = esp_fopen(path);
            if (fp)
            {
                if (tip)
                {
                    printf("Copy file %s ... ", path + pos);
                }
                else
                {
                    printf("Copy file %s ... [OK]\n", path + pos);
                }

                while ((len = (int)esp_fread(g_file_buf, 1, FILE_BUF_SIZE, fp)) > 0)
                {
                    fl_fwrite(g_file_buf, 1, len, file);
                    tot += len;
                    
                    if (tip)
                    {
                        printf("\rCopy file %s ... %lld%%  ", path + pos, (long long)(tot * 100 / size));
                    }
                }

                if (tip)
                {
                    printf("\rCopy file %s ... [OK]   \n", path + pos);
                }

                debug("Copy file %s [OK]\n", path);
                esp_fclose(fp);
            }
            else
            {
                debug("Failed to open <%s>\n", path);
            }
            fl_fclose(file);
        }
        else
        {
            debug("Failed to create <%s>\n", path + pos);
        }
    }
    
    return 0;
}

static uint64_t calc_part_size(char *root)
{
    uint64_t size = 0;
    uint64_t mb = 0;
    
    esp_enum_dir(root, esp_enum_calc_back, &size);
    size = ((size + (SIZE_64KB - 1)) & ~(SIZE_64KB - 1));

    /* FAT16 has min volume size limit */
    if (size < SIZE_5MB)
    {
        size = SIZE_5MB;
    }

    mb = size / SIZE_1MB;
    if (mb <= 512)
    {
        size += SIZE_1MB;
    }
    else if (mb <= 1024)
    {
        size += 2 * SIZE_1MB;
    }
    else
    {
        /* size += (mb + 1023) / 1024 * 2 * SIZE_1MB; */
        size += (mb + 1023) * 2048;
    }

    debug("calc_part_size = %lld\n", (long long)size);

    return size;
}

static int fill_backup_gpt_head(gpt_info *pInfo, gpt_head *pHead)
{
    uint64_t LBA;
    uint64_t BackupLBA;

    memcpy(pHead, &pInfo->Head, sizeof(gpt_head));

    LBA = pHead->EfiStartLBA;
    BackupLBA = pHead->EfiBackupLBA;
    
    pHead->EfiStartLBA = BackupLBA;
    pHead->EfiBackupLBA = LBA;
    pHead->PartTblStartLBA = BackupLBA + 1 - 33;

    pHead->Crc = 0;
    pHead->Crc = esp_crc32(pHead, pHead->Length);

    return 0;
}

static int esp_fill_gpt(uint64_t disksize, uint64_t partsize, gpt_info *gptm, gpt_info *gptb, char *label)
{
    int i;
    uint64_t disksector;
    mbr_head *pMBR;
    gpt_head *pHead;
    esp_guid disksig;
    esp_guid espguid = { 0xc12a7328, 0xf81f, 0x11d2, { 0xba, 0x4b, 0x00, 0xa0, 0xc9, 0x3e, 0xc9, 0x3b } };

    debug("fill gpt partition table ...\n");

    disksector = disksize / 512;
    
    pMBR = &gptm->MBR;
    pMBR->PartTbl[0].Active = 0x00;
    pMBR->PartTbl[0].FsFlag = 0xEE;
    pMBR->PartTbl[0].StartHead = 0;
    pMBR->PartTbl[0].StartSector = 1;
    pMBR->PartTbl[0].StartCylinder = 0;
    pMBR->PartTbl[0].EndHead = 254;
    pMBR->PartTbl[0].EndSector = 63;
    pMBR->PartTbl[0].EndCylinder = 1023;
    pMBR->PartTbl[0].StartSectorId = 1;
    if ((disksector - 1) > 0xFFFFFFFF)
    {
        pMBR->PartTbl[0].SectorCount = 0xFFFFFFFF;
    }
    else
    {
        pMBR->PartTbl[0].SectorCount = (uint32_t)(disksector - 1);        
    }
    pMBR->Byte55 = 0x55;
    pMBR->ByteAA = 0xAA;

    esp_uuid(&disksig);
    memcpy(pMBR->BootCode + 0x1B8, &disksig, 4);

    pHead = &gptm->Head;
    memcpy(pHead->Signature, "EFI PART", 8);

    pHead->Version[2] = 0x01;
    pHead->Length = 92;
    pHead->Crc = 0;
    pHead->EfiStartLBA = 1;
    pHead->EfiBackupLBA = disksector - 1;
    pHead->PartAreaStartLBA = 34;
    pHead->PartAreaEndLBA = disksector - 34;
    esp_uuid(&pHead->DiskGuid);
    pHead->PartTblStartLBA = 2;
    pHead->PartTblTotNum = 128;
    pHead->PartTblEntryLen = 128;

    memcpy(&(gptm->PartTbl[0].PartType), &espguid, sizeof(esp_guid));
    esp_uuid(&(gptm->PartTbl[0].PartGuid));
    
    gptm->PartTbl[0].StartLBA = 64;
    gptm->PartTbl[0].LastLBA = partsize / 512 + gptm->PartTbl[0].StartLBA - 1;
    gptm->PartTbl[0].Attr = 0;

    for (i = 0; i < 36 && label[i]; i++)
    {
        gptm->PartTbl[0].Name[i] = (uint16_t)label[i];
    }
    
    /* Update CRC */
    pHead->PartTblCrc = esp_crc32(gptm->PartTbl, sizeof(gptm->PartTbl));
    pHead->Crc = esp_crc32(pHead, pHead->Length);

    fill_backup_gpt_head(gptm, &gptb->Head);
    memcpy(gptb->PartTbl, gptm->PartTbl, sizeof(gptb->PartTbl));

    return 0;
}

static int fill_mbr_loc(uint64_t DiskSizeInBytes, uint32_t StartSectorId, uint32_t SectorCount, mbr_part_table *Table)
{
    uint8_t Head;
    uint8_t Sector;
    uint8_t nSector = 63;
    uint8_t nHead = 8;    
    uint32_t Cylinder;
    uint32_t EndSectorId;

    while (nHead != 0 && (DiskSizeInBytes / 512 / nSector / nHead) > 1024)
    {
        nHead = (uint8_t)nHead * 2;
    }

    if (nHead == 0)
    {
        nHead = 255;
    }

    Cylinder = StartSectorId / nSector / nHead;
    Head = StartSectorId / nSector % nHead;
    Sector = StartSectorId % nSector + 1;

    Table->StartHead = Head;
    Table->StartSector = Sector;
    Table->StartCylinder = Cylinder;

    EndSectorId = StartSectorId + SectorCount - 1;
    Cylinder = EndSectorId / nSector / nHead;
    Head = EndSectorId / nSector % nHead;
    Sector = EndSectorId % nSector + 1;

    Table->EndHead = Head;
    Table->EndSector = Sector;
    Table->EndCylinder = Cylinder;

    Table->StartSectorId = StartSectorId;
    Table->SectorCount = SectorCount;

    return 0;
}

static int esp_fill_mbr(uint64_t disksize, uint64_t partsize, mbr_head *mbr)
{
    esp_guid disksig;

    mbr->PartTbl[0].Active = 0x80;
    mbr->PartTbl[0].FsFlag = 0xEF;

    esp_uuid(&disksig);
    memcpy(mbr->BootCode + 0x1B8, &disksig, 4);
    
    mbr->Byte55 = 0x55;
    mbr->ByteAA = 0xAA;
    fill_mbr_loc(disksize, 64, (uint32_t)(partsize / 512), mbr->PartTbl);

    return 0;
}

static int espack(int ptype, int extmb, char *root, char *label)
{
    int ret;
    int sector;
    int rc = 1;
    uint64_t partsize;
    uint64_t disksize;
    mbr_head mbr;
    gpt_info *gptm = NULL;
    gpt_info *gptb = NULL;

    partsize = calc_part_size(root);
    partsize += extmb * SIZE_1MB;

    debug("espack fat part size: %lld (%lldMB)\n", (long long)partsize, (long long)(partsize / SIZE_1MB));

    if (ptype)
    {
        disksize = partsize + (64 + 40) * 512;
        gptm = malloc(sizeof(gpt_info) * 2);
        if (!gptm)
        {
            printf("Failed to alloc gpt info %d\n", errno);
            return 1;
        }

        memset(gptm, 0, sizeof(gpt_info) * 2);
        gptb = gptm + 1;

        esp_fill_gpt(disksize, partsize, gptm, gptb, label);

        esp_fseek(g_img_file, 0, SEEK_SET);
        esp_fwrite(gptm, 1, sizeof(gpt_info), g_img_file);
        sector = 34;
    }
    else
    {
        disksize = partsize + (64 + MBR_APPEND_SECTOR) * 512;
        memset(&mbr, 0, sizeof(mbr));

        esp_fill_mbr(disksize, partsize, &mbr);
        
        esp_fseek(g_img_file, 0, SEEK_SET);
        esp_fwrite(&mbr, 1, 512, g_img_file);
        sector = 1;
    }

    if (disksize > SIZE_2TB)
    {
        printf("Image file size overflow (2TB+).\n");
        goto out;
    }

    /* write align to 64 sectors */
    debug("align to 64 sectors ...\n");
    write_zero_sector(64 - sector);

    debug("format the esp part ...\n");
    fl_init();

    (void)fl_attach_media(esp_disk_read, esp_disk_write);
    ret = fl_format((uint32_t)(partsize / 512), label);
    debug("format fat part %lld sectors, [%s]\n", (long long)(partsize / 512), ret ? "OK" : "FAIL");

    esp_enum_dir(root, esp_enum_copy_back, (void *)strlen(root));

    fl_shutdown();

    /* write gpt backup part table */
    if (ptype)
    {
        debug("write backup gpt part table ...\n");

        esp_fseek(g_img_file, partsize + 512 * 64, SEEK_SET);
        write_zero_sector(7);
        esp_fwrite(gptb->PartTbl, 1, 512 * 32, g_img_file);
        esp_fwrite(&gptb->Head, 1, 512, g_img_file);
    }
    else
    {
        /* seek to the end of partition 1 and append a 4KB zero data */
        esp_fseek(g_img_file, partsize + 512 * 64, SEEK_SET);
        write_zero_sector(MBR_APPEND_SECTOR);
    }

    rc = 0;
    debug("finished with file size %lld ...\n", (long long)esp_ftell(g_img_file));

out:
    if (gptm)
    {
        free(gptm);        
    }
    return rc;
}

static void print_usage(void)
{
    printf("\n================== espack %s ==================\n\n"
           "Usage: espack [ OPTION ]\n"
           "  OPTION (Optional)\n"
           "   -r ROOT      Source root path\n"
           "   -m           MBR partition style (default is GPT)\n"
           "   -l LABEL     ESP partition label (default is ESP)\n"
           "   -e SIZE_MB   Extra space for the ESP in MB unit (default is 2)\n"
           "   -o FILE      Output file (default is ./esp.img)\n"
           "   -h           Print this help information\n"
           "   -v           Verbose\n"
           "\n",
           ESPACK_VERSION
           );
}

int main(int argc, char **argv)
{
    int i;
    int ch;
    int ret = 0;
    int len = 0;
    int ptype = 1;
    int exsize = 2;
    char root[ESP_MAX_PATH];
    char outfile[ESP_MAX_PATH];
    char label[64];
    void *fp = NULL;

    root[0] = outfile[0] = label[0] = 0;
    sprintf_s(outfile, sizeof(outfile), "./esp.img");
    sprintf_s(label, sizeof(label), "ESP");

    if (argc == 1)
    {
        print_usage();
        return 0;
    }

    setbuf(stdout, NULL);

    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] != '-')
        {
            print_usage();
            return 1;
        }

        ch = argv[i][1];
        if (ch == 'r')
        {
            strcpy_s(root, sizeof(root) - 1, argv[++i]);
        }
        else if (ch == 'l')
        {
            strcpy_s(label, sizeof(label) - 1, argv[++i]);
        }
        else if (ch == 'o')
        {
            strcpy_s(outfile, sizeof(outfile) - 1, argv[++i]);
        }
        else if (ch == 'e')
        {
            exsize = (int)strtoul(argv[++i], NULL, 10);
        }
        else if (ch == 'm')
        {
            ptype = 0;
        }
        else if (ch == 'h')
        {
            print_usage();
            return 0;
        }
        else if (ch == 'v')
        {
            verbose = 1;
        }
        else
        {
            printf("222\n");
            print_usage();
            return 1;
        }
    }

    for (len = (int)strlen(root); len > 0 && root[len - 1] == '/'; len--)
    {
        root[len - 1] = 0;
    }

    if (root[0] == 0)
    {
        printf("Please use -r to specify the source root path.\n");
        print_usage();
        return 1;
    }

    g_file_buf = malloc(FILE_BUF_SIZE);
    if (!g_file_buf)
    {
        printf("Failed to alloc buffer.\n");
        return 1;
    }

    printf("\n================== espack %s ==================\n", ESPACK_VERSION);
    
    debug("Part  Stlye: <%s>\n", (ptype ? "GPT" : "MBR"));
    debug("Part  Label: <%s>\n", label);
    debug("Extral Size: <%d MB>\n", exsize);
    debug("Source Root: <%s>\n", root);
    debug("Output File: <%s>\n", outfile);

    fp = esp_fcreate(outfile);
    if (!fp)
    {
        printf("Failed to create file %s error:%d\n", outfile, errno);
        free(g_file_buf);
        return 1;
    }

    debug("create empty file %s sucess...\n", outfile);

    g_img_file = fp;
    ret = espack(ptype, exsize, root, label);
    esp_fflush(fp);
    esp_fclose(fp);

    if (0 == ret)
    {
        printf("\nESP image file <%s> create success.\n\n", outfile);
    }
    else
    {
        esp_remove(outfile);
        printf("\n[ERROR] ESP image file <%s> create failed.\n\n", outfile);
    }

    free(g_file_buf);
    return ret;
}

