        /*  BMP-related data types and structures
            Only included is what we need for pxshop  */

#include <stdio.h>
#include <stdint.h>

// Data types
typedef uint8_t  BYTE;  // 1 byte
typedef uint32_t DWORD; // 4 bytes
typedef int32_t  LONG;  // 4 bytes
typedef uint16_t WORD;  // 2 bytes
typedef int16_t  SHORT; // 2 bytes

// Header structures
typedef struct BITMAPFILEHEADER{
    WORD   Type;      // 2 bytes
    DWORD  Size;      // 4 bytes
    WORD   Reserved1; // 2 bytes
    WORD   Reserved2; // 2 bytes
    DWORD  OffBits;   // 4 bytes
} __attribute__((__packed__))
BITMAPFILEHEADER;     // 14 bytes

typedef struct BITMAPV3HEADER{
    DWORD  Size;          // 4 bytes
    LONG   Width;         // 4 bytes
    LONG   Height;        // 4 bytes
    WORD   Planes;        // 2 bytes
    WORD   BitCount;      // 2 bytes
    DWORD  Compression;   // 4 bytes
    DWORD  SizeImage;     // 4 bytes
    LONG   XPelsPerMeter; // 4 bytes
    LONG   YPelsPerMeter; // 4 bytes
    DWORD  ClrUsed;       // 4 bytes
    DWORD  ClrImportant;  // 4 bytes
} __attribute__((__packed__))
BITMAPV3HEADER;         // 40 bytes

typedef struct RGBQUAD{ 
    BYTE   rgbBlue;     
    BYTE   rgbGreen;    
    BYTE   rgbRed;      
    BYTE   rgbReserved; 
} __attribute__((__packed__))
RGBQUAD;                // 4 bytes

// Custom struct which only contains the info needed for data extraction, regardless of version
typedef struct BMPINFO{
    WORD   Version; 
    DWORD  Offset;
    LONG   Width;
    LONG   Height;
    WORD   Bitrate;
    DWORD  Compression;
    WORD   NumColors; 
}
BMPINFO; 

// Container to package a file stream and our info struct
typedef struct BITMAP{
    FILE *bmpFile;
    BMPINFO info;
}
BITMAP;