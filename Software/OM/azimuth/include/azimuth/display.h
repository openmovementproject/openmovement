// Monochrome Display Utilities
// Dan Jackson, 2013
// 

#ifndef DISPLAY_H
#define DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#define DISPLAY_WIDTH  128
#define DISPLAY_HEIGHT 64
#define DISPLAY_BUFFER_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT / 8)

// Packed Monochrome Display structure
typedef struct display_struct_t
{
    unsigned char buffer[DISPLAY_BUFFER_SIZE];  // Allocated buffer
} display_t;

// Initializes a mew display
int DisplayInit(display_t *display);

// Create mew display
void DisplaySetPixel(display_t *display, int x, int y, char c);



// RGB888 Image structure
typedef struct image_struct_t
{
    int width, height;      // Dimensions
    unsigned char *buffer;  // Allocated buffer
    unsigned char *tempBuffer;  // Temp buffer for processing
} image_t;

// Image create empty
int ImageInit(image_t *image, int width, int height);

// Image create from file (only bottom-up BGR888 .BMP files are supported for now)
int ImageNewFromFile(image_t *image, const char *filename);

// Image delete
void ImageDelete(image_t *image);

// Image edge enhance
void ImageEdgeEnhance(image_t *image, int level);

// Image ordered dither
void ImageDither(image_t *image, int type);

// Options for image to display
#define IMAGE_TO_DISPLAY_INVERT 0x0001

// Copy image to monochrome display
void ImageToDisplay(image_t *image, display_t *display, int options);


// Write display to a device
struct azimuth_struct_t;
void DisplayToDeviceDump(struct azimuth_struct_t *azimuth, display_t *display);
void DisplayToDeviceSlip(struct azimuth_struct_t *azimuth, display_t *display);
void DisplayToDevice(struct azimuth_struct_t *azimuth, display_t *display);

#ifdef __cplusplus
}
#endif


#endif
