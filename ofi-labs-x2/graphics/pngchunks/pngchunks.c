/*
    This file is part of the Ofi Labs X2 project.

    Copyright (C) 2010 Ariya Hidayat <ariya.hidayat@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>

int main(int argc, char **argv)
{
    const char *fname;
    FILE *f;
    unsigned char buf[9];
    unsigned char magic[] = { 137, 80, 78, 71, 13, 10, 26, 10 };
    int total, i;

    if (argc != 2) {
        printf("Usage: pngchunks image.png\n\n");
        return -1;
    }

    fname = argv[1];
    f = fopen(fname, "rb");
    if (!f) {
        printf("Error: can't open file %s!\n", fname);
        return -1;
    }

    fread(buf, 1, 8, f);
    for (i = 0; i < 8; ++i) {
        if (buf[i] != magic[i]) {
            printf("Not a PNG file!\n");
            fclose(f);
            return -1;
        }
    }

    fseek(f, 0, SEEK_END);
    total = ftell(f);
    printf("File size: %d byte(s)\n", total);
    rewind(f);
    fseek(f, 8, SEEK_SET);

    printf("\n");
    printf("Offset    Chunk    Size\n");

    while (1) {
        int ofs, length;
        ofs = ftell(f);
        if (ofs >= total)
            break;
        fread(buf, 1, 8, f);
        buf[8] = '\0';
        length = buf[3] + (buf[2] << 8) + (buf[1] << 16) + (buf[0] << 24);
        printf("%7d    %s   %5d\n", ofs, buf + 4, length);
        if (length < 0)
            printf("%d %d %d %d\n", buf[0], buf[1], buf[2], buf[3]);
        fseek(f, length, SEEK_CUR);
        fread(buf, 1, 4, f); /* CRC */
    }

    fclose(f);

    return 0;
}
