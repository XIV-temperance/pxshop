        /*  Lirary for BMP classification and decoding/encoding for pxshop
            Supports Windows BMP versions 3.0 - 5.0
            Supports 1, 2, 4 and 8 bit color indicies
            No RLE support (yet?)  */

#include "bmptools.h"


int bmp_validate(FILE *bmpFile){
    // Validates that file is a Windows BMP
    WORD type;
    rewind(bmpFile);
    fread(&type, sizeof(WORD), 1, bmpFile);
    if (type == 19778) // 'BM' in little-endian/decimal
        return 1;
    else
        return 0;
}

int bmp_get_version(int headerSize){
    // Returns the version (2 - 5) of the BMP file, 0 if OS/2 or proprietary
    switch (headerSize){
        case 12:
        return 2;
        case 40:
        return 3;
        case 108:
        return 4;
        case 124:
        return 5;
    }
    return 0;
}

float bmp_get_padding(int width, int bitrate){
    // Returns the number of bytes of padding per scanline (if any)
    // If a pixel is less than a byte, there could be bit-level padding as well, hence the float return
    float bytesPP = (float) bitrate / 8;
    float bytePadding;
    if (bytesPP < 1){
        bytePadding = ceil(width * bytesPP) - (width * bytesPP);
        bytePadding += (4 - (int) ceil(width * bytesPP) % 4) % 4;
    }else{
        bytePadding = (4 - (width * (int) bytesPP) % 4) % 4;
    }
    return bytePadding;
}

BMPINFO bmp_get_info(FILE *bmpFile){
    // Collects the info we care about from the BMP for decompression
    BMPINFO info = {0};
    if (!bmp_validate(bmpFile))
        return info;
    rewind(bmpFile);
    BITMAPFILEHEADER fh;
    fread(&fh, sizeof(BITMAPFILEHEADER), 1, bmpFile);
    info.Offset = fh.OffBits;
    DWORD ihSize;
    fread(&ihSize, sizeof(DWORD), 1, bmpFile);
    fseek(bmpFile, -sizeof(DWORD), SEEK_CUR);
    info.Version = bmp_get_version(ihSize);
    BITMAPV3HEADER ih;
    fread(&ih, sizeof(BITMAPV3HEADER), 1, bmpFile);
    info.Bitrate = ih.BitCount;
    info.Width = ih.Width;
    info.Height = ih.Height;
    info.NumColors = (info.Offset - sizeof(BITMAPFILEHEADER) - ihSize) / 4;
    info.Compression = ih.Compression;
    return info;
}

void bmp_to_array(BITMAP bmp, unsigned char array[]){
    // Takes an empty char array and fills it with the uncompressed pixel data of a BMP
    // Input BMP is processed according to its header info and pixels are placed into array indicies
    int bytesPerLine = (float) bmp.info.Width / (8 / bmp.info.Bitrate) + bmp_get_padding(bmp.info.Width, bmp.info.Bitrate);
    fseek(bmp.bmpFile, bmp.info.Offset, SEEK_SET);
    for (int line = 0; line < bmp.info.Height; line++){
        unsigned char linePixels[bytesPerLine * 8 / bmp.info.Bitrate], lineBuffer[bytesPerLine];
        fread(&lineBuffer, bytesPerLine, 1, bmp.bmpFile);
        for (int j = 0; j < bytesPerLine; j++){
            for (int k = 0; k < 8 / bmp.info.Bitrate; k++){
                linePixels[(j + 1) * (8 / bmp.info.Bitrate) - k - 1] = lineBuffer[j] & (0xFF >> ((8 / bmp.info.Bitrate - 1) * bmp.info.Bitrate));
                lineBuffer[j] >>= bmp.info.Bitrate;
            }
        }
        for (int j = 0; j < bmp.info.Width; j++)
            array[j + line * bmp.info.Width] = linePixels[j];
    }
    return;
}

void bmp_from_array(BITMAP bmp, unsigned char array[], RGBQUAD palette[]){
    // Takes a populated unsigned char array and loads it into an indexed color bitmap 3.0
    // First part loads the file headers and writes them
    float bytePadding = bmp_get_padding(bmp.info.Width, bmp.info.Bitrate);
    BITMAPFILEHEADER fh = {0};
    fh.Type = 19778;
    fh.Size = bmp.info.Width * bmp.info.Height * bmp.info.Bitrate / 8 + 
    bytePadding * bmp.info.Height +
    sizeof(BITMAPFILEHEADER) + sizeof(BITMAPV3HEADER) + sizeof(RGBQUAD) * bmp.info.NumColors;
    fh.OffBits = bmp.info.Offset;
    BITMAPV3HEADER ih = {0};
    ih.Size = 40;
    ih.Width = bmp.info.Width;
    ih.Height = bmp.info.Height;
    ih.BitCount = bmp.info.Bitrate;
    ih.SizeImage = bmp.info.Width * bmp.info.Height * bmp.info.Bitrate / 8 + 
    bytePadding * bmp.info.Height;
    ih.Planes = 1;
    ih.ClrUsed = bmp.info.NumColors;
    rewind(bmp.bmpFile);
    fwrite(&fh, sizeof(BITMAPFILEHEADER), 1, bmp.bmpFile);
    fwrite(&ih, sizeof(BITMAPV3HEADER), 1, bmp.bmpFile);
    // Writing the palette
    for (int i = 0; i < bmp.info.NumColors; i++)
        fwrite(&palette[i], sizeof(RGBQUAD), 1, bmp.bmpFile);
    // Writing the compressed pixel data (fun math makes this work for all supported bitrates)
    int bitPadding = bmp.info.Width % (8 / bmp.info.Bitrate);
    unsigned char empty = 0;
    for (int line = 0; line < bmp.info.Height; line++){
        int pixel = 0;
        for (pixel = 0; pixel < bmp.info.Width - bitPadding; pixel += (8 / bmp.info.Bitrate)){
            unsigned char newByte = array[pixel + line * bmp.info.Width];
            for (int comp = 1; comp < (8 / bmp.info.Bitrate); comp++){
                newByte <<= bmp.info.Bitrate;
                newByte |= array[pixel + comp + line * bmp.info.Width];
            }
            fwrite(&newByte, 1, 1, bmp.bmpFile);
        }
        unsigned char extra = 0;
        for (int bit = 0; bit < bitPadding; bit++)
            extra |= array[pixel + bit + line * bmp.info.Width] << (((8 / bmp.info.Bitrate) - bit) * bmp.info.Bitrate - bmp.info.Bitrate);
        if (bitPadding)
            fwrite(&extra, 1, 1, bmp.bmpFile);
        for (int pad = 0; pad < (int) bytePadding; pad++)
            fwrite(&empty, 1, 1, bmp.bmpFile);
    }
    return;
}