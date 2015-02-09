// Monochrome Display Utilities
// Dan Jackson, 2013

#ifdef _WIN32
#define _USE_MATH_DEFINES       // For M_PI
#endif

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "azimuth/azimuth.h"
#include "azimuth/display.h"


int DisplayInit(display_t *display)
{
    ;
    return 1;
}

void DisplaySetPixel(display_t *display, int x, int y, char c)
{
    if (c)
    {
        display->buffer[DISPLAY_WIDTH * (y >> 3) + x] |= (1 << (y & 0x7));
    }
    else
    {
        display->buffer[DISPLAY_HEIGHT * (y >> 3) + x] &= ~(1 << (y & 0x7));
    }
}



// Image create empty
int ImageInit(image_t *image, int width, int height)
{
    int size;
    if (image == NULL || width < 0 || height < 0) { return 0; }
    image->width = width;
    image->height = height;
    size = width * height * 3;
    image->buffer = (unsigned char *)realloc(image->buffer, size);
    image->tempBuffer = (unsigned char *)realloc(image->tempBuffer, size);
    if (image->buffer == NULL) { return 0; }
    memset(image->buffer, 0, size);
    return 1;
}


// Image create from file (only bottom-up BGR888 .BMP files are supported for now)
int ImageInitFromFile(image_t *image, const char *filename)
{
    unsigned char bmpHeader[54];          // BMP_HEADERSIZE = 54
    unsigned long bfOffBits, biWidth, biHeight, biCompression;
    unsigned short biPlanes, biBitCount;
    unsigned int sourceWidth;
    unsigned char *line;
    unsigned char *src, *dest;
    int x, y;
    FILE *fp;

    if (image == NULL) { return 0; }

    // Open file, read and check header
    if (filename == NULL || filename[0] == '\0')    { printf("ERROR: loadImage() - file not specified\n"); return 0; }
    if ((fp = fopen(filename, "rb")) == NULL)       { printf("ERROR: loadImage() - file couldn't be opened (file not found?): %s\n", filename); return 0; }
    if (fread(bmpHeader, 1, sizeof(bmpHeader), fp) != sizeof(bmpHeader)) { printf("ERROR: loadImage() - problem reading header from file: %s\n", filename); fclose(fp); return 0; }
    if (bmpHeader[0] != 'B' || bmpHeader[1] != 'M') { printf("ERROR: loadImage() - invalid header in file: %s\n", filename); fclose(fp); return 0; }

    // Extract important information from header (long winded endian-neutral)
    bfOffBits     = ((unsigned long)bmpHeader[10] | (((unsigned long)bmpHeader[11]) << 8) | (((unsigned long)bmpHeader[12]) << 16) | (((unsigned long)bmpHeader[13]) << 24));
    biWidth       = ((unsigned long)bmpHeader[18] | (((unsigned long)bmpHeader[19]) << 8) | (((unsigned long)bmpHeader[20]) << 16) | (((unsigned long)bmpHeader[21]) << 24));
    biHeight      = ((unsigned long)bmpHeader[22] | (((unsigned long)bmpHeader[23]) << 8) | (((unsigned long)bmpHeader[24]) << 16) | (((unsigned long)bmpHeader[25]) << 24));
    biPlanes      = ((unsigned short)bmpHeader[26] | (((unsigned short)bmpHeader[27]) << 8));
    biBitCount    = ((unsigned short)bmpHeader[28] | (((unsigned short)bmpHeader[29]) << 8));
    biCompression = ((unsigned short)bmpHeader[30] | (((unsigned short)bmpHeader[31]) << 8));
    if (biPlanes != 1 || biCompression != 0 || biBitCount != 24 || (int)biHeight <= 0 || biWidth == 0) { printf("ERROR: loadImage() - unsupported format, (only support bottom-up 24-bit uncompressed), in file: %s\n", filename); fclose(fp); return 0; }

    // Create buffer for RGB data
    if (!ImageInit(image, biWidth, biHeight))
    {
        printf("ERROR: loadImage() - problem creating image: %s\n", filename); 
        fclose(fp); 
        return 0;
    }

    // Read file, turn bottom-up BGR888 to top-down RGB888, go bottom up to not overwrite source data in same buffer
    fseek(fp, bfOffBits, SEEK_SET);
    sourceWidth = ((((biWidth * biBitCount) + 31) & ~31) >> 3);  // Lines stored to word-aligned boundaries
    line = (unsigned char *)malloc(sourceWidth);
    for (y = (int)biHeight - 1; y >= 0; y--)
    {
        fread(line, 1, sourceWidth, fp);
        src = line;
        dest = image->buffer + biWidth * y * 3;
//dest = buffer + biWidth * ((int)biHeight - 1 - y) * 3;    // Invert
        for (x = 0; x < (int)biWidth; x++)
        {
            dest[0] = src[2]; dest[1] = src[1]; dest[2] = src[0]; 
            dest += 3; 
            src += 3;
        }
    }

    // Clean-up and return
    free(line);
    fclose(fp);

    return 1;
}


// Image delete
void ImageDelete(image_t *image)
{
    if (image == NULL) { return; }
    if (image->buffer != NULL) { free(image->buffer); image->buffer = NULL; }
    if (image->tempBuffer != NULL) { free(image->tempBuffer); image->tempBuffer = NULL; }
    image->width = 0; image->height = 0; 
    return;
}


// Image edge-enhance
void ImageEdgeEnhance(image_t *image, int level)
{
    int x, y;

    memcpy(image->tempBuffer, image->buffer, image->width * image->height * 3);
    for (y = 0; y < image->height; y++)
    {
        int span = image->width * 3;
        unsigned char *videoBuffer = image->buffer;
        const unsigned char *s;
        unsigned char *d;
        s = (const unsigned char *)image->tempBuffer + (y * span);
        d = (unsigned char *)image->buffer + (y * span);
        for (x = 0; x < image->width; x++)
        {
            // Weights -- laplacian filter approximations: 4,-1,0 | 8,-1,-1 | 4,-2,1
            #define EDGE_WEIGHT_CENTRE    (8+1)
            #define EDGE_WEIGHT_PRIMARY   -1
            #define EDGE_WEIGHT_SECONDARY -1

            // Current pixel value
            int r = 0, g = 0, b = 0;
            int n = EDGE_WEIGHT_CENTRE;

            // 4-connected Left / Right / Above / Below
            if (x > 0)                  { r += (int)EDGE_WEIGHT_PRIMARY * s[-3];        g += (int)EDGE_WEIGHT_PRIMARY * s[-2];        b += (int)EDGE_WEIGHT_PRIMARY * s[-1];        n += abs(EDGE_WEIGHT_PRIMARY); }
            if (x < image->width - 1)   { r += (int)EDGE_WEIGHT_PRIMARY * s[3];         g += (int)EDGE_WEIGHT_PRIMARY * s[4];         b += (int)EDGE_WEIGHT_PRIMARY * s[5];         n += abs(EDGE_WEIGHT_PRIMARY); }
            if (y > 0)                  { r += (int)EDGE_WEIGHT_PRIMARY * s[-span + 0]; g += (int)EDGE_WEIGHT_PRIMARY * s[-span + 1]; b += (int)EDGE_WEIGHT_PRIMARY * s[-span + 2]; n += abs(EDGE_WEIGHT_PRIMARY); }
            if (y < image->height - 1)  { r += (int)EDGE_WEIGHT_PRIMARY * s[ span + 0]; g += (int)EDGE_WEIGHT_PRIMARY * s[+span + 1]; b += (int)EDGE_WEIGHT_PRIMARY * s[+span + 2]; n += abs(EDGE_WEIGHT_PRIMARY); }

            // 8-connected corners
            #if EDGE_WEIGHT_SECONDARY != 0
            if (x > 0 && y > 0)                                { r += (int)EDGE_WEIGHT_SECONDARY * s[-span - 3]; g += (int)EDGE_WEIGHT_SECONDARY * s[-span - 2]; b += (int)EDGE_WEIGHT_SECONDARY * s[-span - 1]; n += abs(EDGE_WEIGHT_SECONDARY); }
            if (x < image->width - 1 && y > 0)                 { r += (int)EDGE_WEIGHT_SECONDARY * s[-span + 3]; g += (int)EDGE_WEIGHT_SECONDARY * s[-span + 4]; b += (int)EDGE_WEIGHT_SECONDARY * s[-span + 5]; n += abs(EDGE_WEIGHT_SECONDARY); }
            if (x > 0 && y < image->height - 1)                { r += (int)EDGE_WEIGHT_SECONDARY * s[+span - 3]; g += (int)EDGE_WEIGHT_SECONDARY * s[+span - 2]; b += (int)EDGE_WEIGHT_SECONDARY * s[+span - 1]; n += abs(EDGE_WEIGHT_SECONDARY); }
            if (x < image->width - 1 && y < image->height - 1) { r += (int)EDGE_WEIGHT_SECONDARY * s[+span + 3]; g += (int)EDGE_WEIGHT_SECONDARY * s[+span + 4]; b += (int)EDGE_WEIGHT_SECONDARY * s[+span + 5]; n += abs(EDGE_WEIGHT_SECONDARY); }
            #endif

            r += (int)EDGE_WEIGHT_CENTRE * s[0];
            g += (int)EDGE_WEIGHT_CENTRE * s[1];
            b += (int)EDGE_WEIGHT_CENTRE * s[2];

            // Normalize against weights
            //r /= n; g /= n; b /= n;

            // Clamp
            if (r < 0x00) { r = 0x00; } if (r > 0xff) { r = 0xff; }
            if (g < 0x00) { g = 0x00; } if (g > 0xff) { g = 0xff; }
            if (b < 0x00) { b = 0x00; } if (b > 0xff) { b = 0xff; }

            // Write result
            d[0] = (unsigned char)r; d[1] = (unsigned char)g; d[2] = (unsigned char)b;

            // Next pixel
            s += 3; d += 3;
        }
    }
}


// Binary with no dither
const unsigned char dither0[1 * 1] =
{
    0x80, 
};

// Special 3-value 0%, 50%, 100% dither pattern
const unsigned char dither1[2 * 2] =
{
    0xAA, 0x55, 
    0x55, 0xAA, 
};

// 2x2 dither
const unsigned char dither2[2 * 2] =
{
    0x33, 0x99, 
    0xCC, 0x66, 
};

// 3x3 dither
const unsigned char dither3[3 * 3] =
{
    0x4C, 0xCC, 0x66, 
    0x99, 0x19, 0xE5, 
    0x33, 0xB2, 0x7F, 
};

// 4x4 dither
const unsigned char dither4[4 * 4] =
{
    0x0F, 0x87, 0x2D, 0xA5, 
    0xC3, 0x4B, 0xE1, 0x69, 
    0x3C, 0xB4, 0x1E, 0x96, 
    0xF0, 0x78, 0xD2, 0x5A, 
};

// 8x8 dither
const unsigned char dither8[8 * 8] =
{
	0x03, 0xC0, 0x33, 0xF0, 0x0F, 0xCC, 0x3F, 0xFC, 
	0x81, 0x42, 0xB1, 0x72, 0x8D, 0x4E, 0xBD, 0x7E, 
	0x23, 0xE0, 0x13, 0xD0, 0x2F, 0xEC, 0x1F, 0xDC, 
	0xA1, 0x62, 0x91, 0x52, 0xAD, 0x6E, 0x9D, 0x5E, 
	0x0B, 0xC8, 0x3B, 0xF8, 0x07, 0xC4, 0x37, 0xF4, 
	0x89, 0x4A, 0xB9, 0x7A, 0x85, 0x46, 0xB5, 0x76, 
	0x2B, 0xE8, 0x1B, 0xD8, 0x27, 0xE4, 0x17, 0xD4, 
	0xA9, 0x6A, 0x99, 0x5A, 0xA5, 0x66, 0x95, 0x56, 
};


void ImageDither(image_t *image, int type)
{
    // Dither down
    int x, y;        
    const unsigned char *dither = dither0; 
    int ditherSize = 1; 

    if (type == 1) { dither = dither1; ditherSize = 2; }
    if (type == 2) { dither = dither2; ditherSize = 2; }
    if (type == 3) { dither = dither3; ditherSize = 3; }
    if (type == 4) { dither = dither4; ditherSize = 4; }
    if (type == 8) { dither = dither8; ditherSize = 8; }

    for (y = 0; y < image->height; y++)
    {
        int ty = y % ditherSize;
        const unsigned char *s = (const unsigned char *)image->buffer + (y * image->width * 3);
        unsigned char *d = (unsigned char *)s;

        for (x = 0; x < image->width; x++)
        {
            int tx = x % ditherSize;
            int limit = dither[ty * ditherSize + tx];

            unsigned char r = *s++;
            unsigned char g = *s++;
            unsigned char b = *s++;

//unsigned char v = ((int)r + g + b) / 3;
            unsigned char v = (unsigned char)(0.30f * r + 0.59f * g + 0.11f * b);

            // Binary
            if (v < limit) { v = 0x00; } else { v = 0xff; }

            *d++ = v; *d++ = v; *d++ = v;
        }
    }

}


void ImageToDisplay(image_t *image, display_t *display, int options)
{
    // Construct buffer
    int line;

    for (line = 0; line < DISPLAY_HEIGHT/8; line++)
    {
        int y;
        for (y = 0; y < 8; y++)
        {
            int x;
            const unsigned char *s;
            unsigned char *d = (unsigned char *)display->buffer + (line * DISPLAY_WIDTH);
            unsigned char mask = (1 << y);
            unsigned char result = 0;
            int yc = (line * 8) + y;

            if (options & IMAGE_TO_DISPLAY_INVERT) { yc = image->height - 1 - yc; }

            s = (const unsigned char *)image->buffer + (yc * image->width * 3);

            for (x = 0; x < DISPLAY_WIDTH; x++)
            {
                unsigned char r = *s++;
                unsigned char g = *s++;
                unsigned char b = *s++;
                unsigned char v = (r+g+b)/3; //(unsigned char)(0.30f * r + 0.59f * g + 0.11f * b);

                if (v >= 0x80) { *d |= mask; }
                else { *d &= ~mask; }
                d++;
            }

        }
    }
}



// Utility function: Encode a binary input as an ASCII Base64-encoded (RFC 3548) stream with NULL ending -- output buffer must have capacity for (((length + 2) / 3) * 4) + 1 bytes
static int MyEncodeBase64(char *output, const unsigned char *input, int length)
{
    unsigned short value = 0;
    unsigned char bitcount = 0;
    int count = 0;
    int i;

    if (!output || !input) { return 0; }
    for (i = 0; i < length; i++)
    {
        // Add next byte into accumulator
        value = (value << 8) | input[i];
        bitcount += 8;
        
        // End of stream padding to next 6-bit boundary
        if (i + 1 >= length)
        {
            char boundary = ((bitcount + 5) / 6) * 6;
            value <<= (boundary - bitcount);
            bitcount = boundary;
        }

        // While we have 6-bit values to write
        while (bitcount >= 6)
        {
            // Get highest 6-bits and remove from accumulator
            unsigned char c = (unsigned char)((value >> (bitcount - 6)) & 0x3f);
            bitcount -= 6;
            
            // Convert highest 6-bits to an ASCII character
            if (c <= 25) { c = c + 'A'; }                           // 0-25 'A'-'Z'
            else if (c <= 51) { c = c - 26 + 'a'; }                 // 26-51 'a'-'z'
            else if (c <= 61) { c = c - 52 + '0'; }                 // 52-61 '0'-'9'
            else if (c == 62) { c = '+'; }                          // 62 '+'
            else if (c == 63) { c = '/'; }                          // 63 '/'
            
            // Output
            output[count++] = c;
        }
    }

    // Padding for correct Base64 encoding
    while ((count & 3) != 0) { output[count++] = '='; }

    // NULL ending (without incrementing count)
    output[count] = '\0';
    return count;
}



void DisplayToDeviceDump(azimuth_t *azimuth, display_t *display)
{
    #define DISPLAY_STEP 64
    int line, col;
    unsigned char scratch[DISPLAY_STEP + 3];
    char buffer[128];
    for (line = 0; line < DISPLAY_HEIGHT/8; line++)
    {
        for (col = 0; col < DISPLAY_WIDTH; )
        {
            int step = DISPLAY_STEP;
            char *p;

            if (col + step > DISPLAY_WIDTH) { step = DISPLAY_WIDTH - col; }
            if (step <= 0) { break; }

            // Add header
            scratch[0] = col;
            scratch[1] = line;
            scratch[2] = step;
            memcpy(scratch + 3, display->buffer + (line * DISPLAY_WIDTH) + col, step);

            p = buffer;
            p += sprintf(p, "\r\nGDUMP ");
            p += MyEncodeBase64(p, scratch, step + 3);
            p += sprintf(p, "\r\n");

            // Send command
            AzimuthSend(azimuth, (unsigned char *)buffer, (size_t)(p - buffer));

            col += step;
        }
    }
}


void DisplayToDeviceSlip(azimuth_t *azimuth, display_t *display)
{
    #define SLIP_END     0xC0                   // End of packet indicator
    #define SLIP_ESC     0xDB                   // Escape character, next character will be a substitution
    #define SLIP_ESC_END 0xDC                   // Escaped sustitution for the END data byte
    #define SLIP_ESC_ESC 0xDD                   // Escaped sustitution for the ESC data byte

    static unsigned char buffer[2 * DISPLAY_BUFFER_SIZE + 2];
    int i, o = 0;

    buffer[o++] = SLIP_END;
    for (i = 0; i < DISPLAY_BUFFER_SIZE; i++)
    {
        unsigned char c = display->buffer[i];

        if (c == SLIP_END)
		{
            buffer[o++] = SLIP_ESC;
            buffer[o++] = SLIP_ESC_END;
		}
        else if (c == SLIP_ESC)
		{
            buffer[o++] = SLIP_ESC;
            buffer[o++] = SLIP_ESC_ESC;
		}
        else
		{
            buffer[o++] = c;
		}
    }
    buffer[o++] = SLIP_END;

    // Send command
    AzimuthSend(azimuth, buffer, o);
}


void DisplayToDevice(azimuth_t *azimuth, display_t *display)
{
    if (azimuth->deviceMode == 1)
    {
        DisplayToDeviceSlip(azimuth, display);
    }
    else
    {
        DisplayToDeviceDump(azimuth, display);
    }
}

