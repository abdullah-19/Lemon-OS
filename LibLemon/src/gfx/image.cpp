#include <gfx/graphics.h>

#include <math.h>
#include <stdio.h>
#include <string.h>

namespace Lemon::Graphics{
    bool IsPNG(const void* data){
        return !png_sig_cmp((png_const_bytep)data, 0, 8);
    }

    int LoadImage(FILE* f, surface_t* surface){
        char sig[8];
        fseek(f, 0, SEEK_SET);

        if(!fread(sig, 8, 1, f)) {
            return -2; // Could not read first 8 bytes of image
        }

        int type = IdentifyImage(sig);

        if(type == Image_BMP){
            return LoadBitmapImage(f, surface);
        } else if(type == Image_PNG){
            return LoadPNGImage(f, surface);
        } else return -1;
    }

    int LoadImage(const char* path, surface_t* surface){
        FILE* imageFile = fopen(path, "rb");

        if(!imageFile){
            return -1; // Error opening image file
        }

        char sig[8];

        fseek(imageFile, 0, SEEK_SET);

        if(!fread(sig, 8, 1, imageFile)) {
            fclose(imageFile);
            return -2; // Could not read first 8 bytes of image
        }

        int type = IdentifyImage(sig); // Identify Image Type

        if(type == Image_Unknown){ // Unknown Image Type
            fclose(imageFile);
            return type;
        }

        surface_t surf;
        int r = LoadImage(imageFile, &surf);
        fclose(imageFile);

        *surface = surf;
        return r;
    }

    int LoadImage(const char* path, int x, int y, int w, int h, surface_t* surface, bool preserveAspectRatio){
        FILE* imageFile = fopen(path, "rb");

        if(!imageFile){
            return -1; // Error opening image file
        }

        char sig[8];

        fseek(imageFile, 0, SEEK_SET);

        if(!fread(sig, 8, 1, imageFile)) {
            fclose(imageFile);
            return -2; // Could not read first 8 bytes of image
        }

        int type = IdentifyImage(sig); // Identify Image Type

        if(type == Image_Unknown){ // Unknown Image Type
            fclose(imageFile);
            return type;
        }

        surface_t surf;
        int r = LoadImage(imageFile, &surf);

        double xScale = ((double)w) / surf.width;
        double yScale = (((double)h) / surf.height);
        double xOffset = 0;

        if(preserveAspectRatio){
            xScale = ((double)h * (((double)surf.width) / surf.height)) / surf.width;
        // if((xScale * originalWidth) < originalWidth) xOffset = (originalWidth - (xScale * originalWidth)) / 2;
        }

        uint8_t* srcBuffer = surf.buffer;
        uint32_t* destBuffer = (uint32_t*)surface->buffer;

        for (int i = 0; i < h && i + y < surface->height; i++) {
            double _yval = ((double)i) / yScale;
            if(ceil(_yval) >= surf.height) break;
            for (int j = 0; j < w && j + x < surface->width; j++) {
                double _xval = xOffset + ((double) + j) / xScale;
                if(ceil(_xval) >= surf.width) break;
                uint32_t offset = (y + i) * surface->width + x + j;
                
                long b = Interpolate(srcBuffer[((int)floor(_yval) * surf.width + (int)floor(_xval)) * 4], srcBuffer[((int)floor(_yval) * surf.width + (int)ceil(_xval)) * 4], srcBuffer[((int)ceil(_yval) * surf.width + (int)floor(_xval)) * 4], srcBuffer[((int)ceil(_yval) * surf.width + (int)ceil(_xval)) * 4], _xval, _yval);
                long g = Interpolate(srcBuffer[((int)floor(_yval) * surf.width + (int)floor(_xval)) * 4 + 1], srcBuffer[((int)floor(_yval) * surf.width + (int)ceil(_xval)) * 4 + 1], srcBuffer[((int)floor(_yval) * surf.width + (int)ceil(_xval)) * 4 + 1], srcBuffer[((int)ceil(_yval) * surf.width + (int)ceil(_xval)) * 4 + 1], _xval, _yval);
                long r = Interpolate(srcBuffer[((int)floor(_yval) * surf.width + (int)floor(_xval)) * 4 + 2], srcBuffer[((int)floor(_yval) * surf.width + (int)ceil(_xval)) * 4 + 2], srcBuffer[((int)floor(_yval) * surf.width + (int)ceil(_xval)) * 4 + 2], srcBuffer[((int)ceil(_yval) * surf.width + (int)ceil(_xval)) * 4 + 2], _xval, _yval);
                long a = Interpolate(srcBuffer[((int)floor(_yval) * surf.width + (int)floor(_xval)) * 4 + 3], srcBuffer[((int)floor(_yval) * surf.width + (int)ceil(_xval)) * 4 + 3], srcBuffer[((int)floor(_yval) * surf.width + (int)ceil(_xval)) * 4 + 3], srcBuffer[((int)ceil(_yval) * surf.width + (int)ceil(_xval)) * 4 + 3], _xval, _yval);

                destBuffer[offset] = (a << 24) | (r << 16) | (g << 8) | b;
            }
        }

        free(surf.buffer);
        fclose(imageFile);

        return r;
    }

    int LoadPNGImage(FILE* f, surface_t* surface) {
        static png_structp png = nullptr;
        static png_infop info = nullptr;
        static png_infop endInfo = nullptr;
        
        if(!png) {
            png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
            if(!png) return -10;

            info = png_create_info_struct(png);
            if(!info) return -11;
        }

        int e = setjmp(png_jmpbuf(png));
        if(e){
            return e;
        }

        png_init_io(png, f);
        png_set_sig_bytes(png, 8);//ftell(f));

        png_read_info(png, info);

        png_uint_32 width = png_get_image_width(png, info);
        png_uint_32 height = png_get_image_height(png, info);

        if (png_get_color_type(png, info) == PNG_COLOR_TYPE_PALETTE)
            png_set_palette_to_rgb(png);

        if (png_get_color_type(png, info) == PNG_COLOR_TYPE_RGB)
            png_set_filler(png, 0xff, PNG_FILLER_BEFORE);

        png_set_bgr(png);

        surface_t _surface = {.width = width, .height = height, .buffer = (uint8_t*)malloc(width * height * 4)};
        *surface = _surface;

        png_bytepp rowPointers = new png_bytep[height];

        for(int i = 0; i < height; i++){
            rowPointers[i] = surface->buffer + i * surface->width * 4;
        }

        png_read_image(png, rowPointers);

        return 0;
    }

    int LoadBitmapImage(FILE* f, surface_t* surface) {
        fseek(f, 0, SEEK_SET);

        bitmap_file_header_t fileHeader;
        fread(&fileHeader, sizeof(bitmap_file_header_t), 1, f);

        bitmap_info_header_t infoHeader;
        fread(&infoHeader, sizeof(bitmap_info_header_t), 1, f);

        fseek(f, fileHeader.offset, SEEK_SET);

        uint8_t bpp = infoHeader.bpp;
        int width = infoHeader.width;
        int height = infoHeader.height;
        *surface = (surface_t){.width = width, .height = height, .buffer = (uint8_t*)malloc(width * height * 4)};

        uint32_t rowSize = floor((bpp*width + 31) / 32) * 4;
        uint8_t* row = new uint8_t[rowSize];

        uint32_t* buffer = (uint32_t*)surface->buffer;
        
        for (int i = height; i > 0; i--) {
            if(!fread(row, rowSize, 1, f)) break; // End of file
            for (int j = 0; j < width; j++) {
                int c1 = row[j * (bpp / 8)];
                int c2 = row[j * (bpp / 8) + 1];
                int c3 = row[j * (bpp / 8) + 2];
                buffer[(i - 1) * width + j] = (c3 << 16) | (c2 << 8) | c1;
            }
        }

        return 0;
    }

    int DrawBitmapImage(int x, int y, int w, int h, uint8_t *data, surface_t* surface, bool preserveAspectRatio) {
        bitmap_file_header_t bmpHeader = *(bitmap_file_header_t*)data;
        bitmap_info_header_t bmpInfoHeader = *(bitmap_info_header_t*)(data + sizeof(bitmap_file_header_t));
        data += bmpHeader.offset;

        int originalWidth = bmpInfoHeader.width;
        int originalHeight = bmpInfoHeader.height;

        double xScale = ((double)w) / originalWidth;
        double yScale = (((double)h) / originalHeight);
        double xOffset = 0;

        if(preserveAspectRatio){
            xScale = ((double)h * (((double)originalWidth) / originalHeight)) / originalWidth;
        // if((xScale * originalWidth) < originalWidth) xOffset = (originalWidth - (xScale * originalWidth)) / 2;
        }


        uint8_t bmpBpp = 24;
        uint32_t rowSize = (int)floor((bmpBpp*bmpInfoHeader.width + 31) / 32) * 4;
        uint32_t bmp_offset = rowSize * (originalHeight - 1);

        uint32_t pixelSize = 4;
        for (int i = 0; i < h && i + y < surface->height; i++) {
            double _yval = ((double)i) / yScale;
            if(ceil(_yval) >= originalHeight) break;
            for (int j = 0; j < w && j + x < surface->width; j++) {
                double _xval = xOffset + ((double) + j) / xScale;
                if(ceil(_xval) >= originalWidth) break;

                uint8_t r = Interpolate(data[bmp_offset - ((int)floor(_yval) * rowSize) + (int)floor(_xval) * (bmpBpp / 8) + 0], data[bmp_offset - ((int)floor(_yval) * rowSize) + (int)ceil(_xval) * (bmpBpp / 8) + 0], data[bmp_offset - ((int)ceil(_yval) * rowSize) + (int)floor(_xval) * (bmpBpp / 8) + 0], data[bmp_offset - ((int)ceil(_yval) * rowSize) + (int)ceil(_xval) * (bmpBpp / 8) + 0], _xval, _yval);
                uint8_t g = Interpolate(data[bmp_offset - ((int)floor(_yval) * rowSize) + (int)floor(_xval) * (bmpBpp / 8) + 1], data[bmp_offset - ((int)floor(_yval) * rowSize) + (int)ceil(_xval) * (bmpBpp / 8) + 1], data[bmp_offset - ((int)ceil(_yval) * rowSize) + (int)floor(_xval) * (bmpBpp / 8) + 1], data[bmp_offset - ((int)ceil(_yval) * rowSize) + (int)ceil(_xval) * (bmpBpp / 8) + 1], _xval, _yval);
                uint8_t b = Interpolate(data[bmp_offset - ((int)floor(_yval) * rowSize) + (int)floor(_xval) * (bmpBpp / 8) + 2], data[bmp_offset - ((int)floor(_yval) * rowSize) + (int)ceil(_xval) * (bmpBpp / 8) + 2], data[bmp_offset - ((int)ceil(_yval) * rowSize) + (int)floor(_xval) * (bmpBpp / 8) + 2], data[bmp_offset - ((int)ceil(_yval) * rowSize) + (int)ceil(_xval) * (bmpBpp / 8) + 2], _xval, _yval);
                
                uint32_t offset = (y + i)*(surface->width*pixelSize) + (x + j) * pixelSize;
                surface->buffer[offset] = r;
                surface->buffer[offset + 1] = g;
                surface->buffer[offset + 2] = b;
            }
        }

        return 0;
    }
}