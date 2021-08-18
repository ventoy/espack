/******************************************************************************
 * espack_linux.c  ---- Pack ESP to .img file
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
#include <unistd.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <fcntl.h>

#include "espack.h"

static char g_enum_path[ESP_MAX_PATH + 1];

static int enum_dir(int pos, esp_enum_callback callback, void *data)
{
    int len;
    DIR *dir;
    struct dirent *ptr;

    g_enum_path[pos] = 0;
    
    debug("enum <%s> ...\n", g_enum_path);
    if ((dir = opendir(g_enum_path)) == NULL)
    {
        debug("Failed to open dir <%s>\n", g_enum_path);
        return 1;
    }

    while ((ptr = readdir(dir)) != NULL)
    {
        if (ptr->d_name[0] == '.')
        {
            if (ptr->d_name[1] == 0 || (ptr->d_name[1] == '.' && ptr->d_name[2] == 0))
            {
                continue;
            }
        }

        len = (int)sprintf_s(g_enum_path + pos, sizeof(g_enum_path) - pos, "/%s", ptr->d_name);
        if (ptr->d_type & DT_DIR)
        {
            callback(g_enum_path, 1, data);
            enum_dir(pos + len, callback, data);
        }
        else if (ptr->d_type & DT_REG)
        {
            callback(g_enum_path, 0, data);
        }        
    }

    g_enum_path[pos] = 0;
    closedir(dir);
    return 0;
}

int esp_enum_dir(char *root, esp_enum_callback callback, void *data)
{
    int len;
    
    len = (int)sprintf_s(g_enum_path, sizeof(g_enum_path), "%s", root);

    enum_dir(len, callback, data);

    return 0;
}

uint64_t esp_get_file_size(const char *path)
{
    struct stat Stat;

    memset(&Stat, 0, sizeof(Stat));
    if (stat(path, &Stat) < 0)
    {
        return 0;
    }
    else
    {  
        return (uint64_t)Stat.st_size;
    }  
}

void esp_uuid(void *uuid)
{
    int i;
    int fd;

    fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0)
    {
        srand(time(NULL));
        for (i = 0; i < 8; i++)
        {
            *((uint16_t *)uuid + i) = (uint16_t)(rand() & 0xFFFF);
        }
    }
    else
    {
        read(fd, uuid, 16);
        close(fd);
    }
}

void *esp_fopen(const char *path)
{
    return fopen(path, "rb");
}

void *esp_fcreate(const char *path)
{
    return fopen(path, "wb+");
}

void esp_fflush(void *f)
{
    fflush((FILE *)f);
}

void esp_fclose(void *f)
{
    fclose((FILE *)f);
}

int64_t esp_fseek(void *f, int64_t offset, int whence)
{
    return fseeko((FILE *)f, offset, whence);
}

size_t esp_fread(void *ptr, size_t size, size_t nmemb, void *f)
{
    return fread(ptr, size, nmemb, (FILE *)f);
}

size_t esp_fwrite(const void *ptr, size_t size, size_t nmemb, void *f)
{
    return fwrite(ptr, size, nmemb, (FILE *)f);
}

int64_t esp_ftell(void *f)
{
    return ftello((FILE *)f);
}

