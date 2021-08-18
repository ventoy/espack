/******************************************************************************
 * espack_win.c  ---- Pack ESP to .img file
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

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "espack.h"

static char g_enum_path[ESP_MAX_PATH + 1];

static int enum_dir(int pos, esp_enum_callback callback, void *data)
{
    int len;
    HANDLE hFind;
    WIN32_FIND_DATAA findData;

    g_enum_path[pos] = 0;
    debug("enum <%s> ...\n", g_enum_path);

    g_enum_path[pos] = '/';
    g_enum_path[pos + 1] = '*';
    g_enum_path[pos + 2] = 0;

    hFind = FindFirstFileA(g_enum_path, &findData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        debug("Failed to open dir <%s>\n", g_enum_path);
        return 1;
    }

    do
    {
        if (findData.cFileName[0] == '.')
        {
            if (findData.cFileName[1] == 0 || (findData.cFileName[1] == '.' && findData.cFileName[2] == 0))
            {
                continue;
            }
        }

        len = (int)sprintf_s(g_enum_path + pos, sizeof(g_enum_path) - pos, "/%s", findData.cFileName);
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            callback(g_enum_path, 1, data);
            enum_dir(pos + len, callback, data);
        }
        else
        {
            callback(g_enum_path, 0, data);
        }
    } while (FindNextFileA(hFind, &findData));

    g_enum_path[pos] = 0;
    FindClose(hFind);
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
    HANDLE hFile;
    DWORD dSizeLow  = 0;
    DWORD dSizeHigh = 0;

    hFile = CreateFileA(path, FILE_READ_EA, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        return 0;
    }

    dSizeLow = GetFileSize(hFile, &dSizeHigh);
    CloseHandle(hFile);

    return (uint64_t)((uint64_t)dSizeHigh << 32) | dSizeLow; 
}

void esp_uuid(void *uuid)
{
    CoCreateGuid(uuid);
}

void *esp_fopen(const char *path)
{
    FILE *fp = NULL;

    fopen_s(&fp, path, "rb");
    return fp;    
}

void *esp_fcreate(const char *path)
{
    FILE *fp = NULL;

    fopen_s(&fp, path, "wb+");
    return fp; 
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
    return _fseeki64((FILE *)f, offset, whence);
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
    return _ftelli64((FILE *)f);
}

