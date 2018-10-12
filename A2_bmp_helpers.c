
/*
Created by: GREG WOO
Program: Work with BMP files
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

int bmp_open( char* bmp_filename,        unsigned int *width,
  unsigned int *height,      unsigned int *bits_per_pixel,
  unsigned int *padding,     unsigned int *data_size,
  unsigned int *data_offset, unsigned char** img_data ){

    //////////////////////////////////////////////////////////////
    // BMP Info

    //We begin by opening the BMP file to read from it
    FILE *bmpfile = fopen(bmp_filename, "rb");

    //We check that the file begins with characters 'B' & 'M'
    //This way we want to make sure it is of the correct format
    char b, m;
    fread(&b, 1, 1, bmpfile);
    fread(&m, 1, 1, bmpfile);
    if (b != 'B' || m != 'M'){
      printf("The file provided is not a BMP image.");
      return -1;
    }

    // The strategy is to read the information of certain bytes
    // and attribute this data to the correct variable

    //Read the size of the image
    unsigned int size_image;
    fread (&size_image,1,sizeof(unsigned int),bmpfile);
    *data_size = size_image;

    //Here we skip bytes in order to reach the 10th byte
    char temp;
    for (int i = 0; i < 4; i++) {
      fread (&temp, 1, sizeof (char), bmpfile);
    }

    //Getting the data_offset
    unsigned int offset_image;
    fread (&offset_image,1,sizeof (unsigned int),bmpfile);
    *data_offset= offset_image;

    //Again we skip bytes in order to reach the 18th byte
    int temp2 = 0;
    fread (&temp2,1,sizeof (int),bmpfile);

    //Getting the width of the image
    unsigned int width_image;
    fread (&width_image, 1, sizeof(unsigned int), bmpfile);
    *width = width_image;

    //Getting the height of the image
    unsigned int height_image;
    fread (&height_image, 1, sizeof(unsigned int), bmpfile);
    *height = height_image;

    //Again we skip bytes in order to reach the 28th byte
    for (int i = 0; i < 2; i++) {
      fread (&temp, 1, sizeof(char), bmpfile);
    }

    //Getting the number of bits per pixel
    unsigned int bpp;
    fread (&bpp, 1, sizeof (unsigned int), bmpfile);
    *bits_per_pixel = bpp;

    //Getting the padding
    unsigned int padding_image = 0;
    int temp3 = ((*width) * (*bits_per_pixel / 8)) % 4;
    if ( temp3 != 0) {
      padding_image = 4 - temp3;
    }
    *padding = padding_image;

    // Now we close the file and then re-open it to be at the beginning
    fclose(bmpfile);
    bmpfile = fopen (bmp_filename, "rb");

    // Here we allocate memory for the image's data into the heap
    *img_data = (unsigned char*)malloc(*data_size);
    fread (*img_data,1,*data_size,bmpfile);

    // Finally we close the file
    fclose (bmpfile);

    return 0;
  }

  // We've implemented bmp_close for you. No need to modify this function
  void bmp_close( unsigned char **img_data ){

    if( *img_data != NULL ){
      free( *img_data );
      *img_data = NULL;
    }
  }

  int bmp_mask( char* input_bmp_filename, char* output_bmp_filename,
    unsigned int x_min, unsigned int y_min, unsigned int x_max, unsigned int y_max,
    unsigned char red, unsigned char green, unsigned char blue )
    {
      unsigned int img_width;
      unsigned int img_height;
      unsigned int bits_per_pixel;
      unsigned int data_size;
      unsigned int padding;
      unsigned int data_offset;
      unsigned char* img_data   = NULL;

      int open_return_code = bmp_open( input_bmp_filename, &img_width, &img_height, &bits_per_pixel, &padding, &data_size, &data_offset, &img_data );

      if( open_return_code ){ printf( "bmp_open failed. Returning from bmp_mask without attempting changes.\n" ); return -1; }


      //////////////////////////////////////////////////////////////
      //BMP Mask

      //First we want to assign heap space for the new image
      unsigned char* newImage = (unsigned char*)(malloc(data_size));

      //Then we copy the previous image into the memory
      memcpy (newImage, img_data, data_size);

      int x_pixel = 0;
      int y_pixel = 0;

      // Using the two variables above, we can go through each pixel of the image
      for (int i = 0 ; (x_pixel <= img_width) && (y_pixel < img_height) ; i += 3){

        //When we reach the end of a line we update the variables
        if ((x_pixel/(img_width) == 1) && (x_pixel != 0)){

          if (padding != 0){
            for (int j = 0; j < padding; j++){
              newImage[data_offset + j] = 0;
            }
            i += padding;
          }

          y_pixel++;
          x_pixel = 0;
        }

        //Check if the pixels correspond to what the user inputted
        if ((x_pixel <= x_max) && (x_pixel >= x_min)){
          if ((y_pixel <= y_max) && (y_pixel >= y_min)){

            //Here we change the pixel to the color set by the user
            newImage[data_offset + i + 2] = red;
            newImage[data_offset + i + 1] = green;
            newImage[data_offset + i] = blue;
          }
        }
        x_pixel++;
      }

      //Then we write the file with the updated image
      FILE *fp = fopen (output_bmp_filename, "w");
      fwrite(newImage, data_size, 1, fp);

      //Finally we close the file
      fclose(fp);

      bmp_close( &newImage );
      bmp_close( &img_data );

      return 0;
    }

    //////////////////////////////////////////////////////////////
    //BMP Collage

    // To begin, we want to write 2 helper methods we are going to use in our collage

    // Method that finds the biggest int between two integers inputted
    int findBiggest (int a, int b) {

      if (a > b) {
        return a;
      } else {
        return b;
      }
    }

    // Method that finds the smallest int between two integers inputted
    int findSmallest (int a, int b) {

      if (a > b) {
        return b;
      } else {
        return a;
      }
    }

    int bmp_collage( char* bmp_input1, char* bmp_input2, char* bmp_result, int x_offset, int y_offset ){

      unsigned int img_width1;
      unsigned int img_height1;
      unsigned int bits_per_pixel1;
      unsigned int data_size1;
      unsigned int padding1;
      unsigned int data_offset1;
      unsigned char* img_data1    = NULL;

      int open_return_code = bmp_open( bmp_input1, &img_width1, &img_height1, &bits_per_pixel1, &padding1, &data_size1, &data_offset1, &img_data1 );

      if( open_return_code ){ printf( "bmp_open failed for %s. Returning from bmp_collage without attempting changes.\n", bmp_input1 ); return -1; }

      unsigned int img_width2;
      unsigned int img_height2;
      unsigned int bits_per_pixel2;
      unsigned int data_size2;
      unsigned int padding2;
      unsigned int data_offset2;
      unsigned char* img_data2    = NULL;

      open_return_code = bmp_open( bmp_input2, &img_width2, &img_height2, &bits_per_pixel2, &padding2, &data_size2, &data_offset2, &img_data2 );

      if( open_return_code ){ printf( "bmp_open failed for %s. Returning from bmp_collage without attempting changes.\n", bmp_input2 ); return -1; }

      /////////////////////////////////////////////////////////////////////////////////
      //We start by declaring variables that are going to be in the header of our collaged image
      unsigned int width_collage, height_collage, data_size_collage;

      //Here we find the top left and bottom right corners
      int left_min = findSmallest (0, x_offset);
      int down_min = findSmallest (0, y_offset);
      int right_max = findBiggest (img_width1, x_offset + img_width2);
      int up_max = findBiggest (img_height1, y_offset + img_height2);

      //Then we update the height and width of the image
      width_collage = right_max - left_min;
      height_collage = up_max - down_min;

      //We find the padding needed with the updated width
      int padding_collage = 0;

      for (int i = 0 ; i < 4 ; i++){
        unsigned int j = width_collage * bits_per_pixel1 /8;

        if ((j + i) % 4 == 0) {
        padding_collage = i;
        }

      }

      //Here we want to know when we should start printing pixels from each of the images
      int img1_start_col, img1_start_row;
      int img2_start_col, img2_start_row;

      //First y axis
      if (y_offset > 0) {
        img1_start_row = 0;
        img2_start_row = y_offset;

      } else {
        img1_start_row = -y_offset;
        img2_start_row = 0;

      }

      //Then x axis
      if (x_offset > 0) {
        img1_start_col = 0;
        img2_start_col = x_offset;

      } else {
        img1_start_col = -x_offset;
        img2_start_col = 0;

      }

      //Now we want to know when we should stop printing pixels from each of the images
      int img1_end_col, img1_end_row;
      int img2_end_col, img2_end_row;

      img1_end_col = img_width1 + img1_start_col;
      img1_end_row = img_height1 + img1_start_row;
      img2_end_col = img_width2 + img2_start_col;
      img2_end_row = img_height2 + img2_start_row;

      //Here we find the number of bytes per row and update the data size variable
      int bytes_per_row = width_collage * (bits_per_pixel1 / 8) + padding_collage;
      data_size_collage = bytes_per_row * height_collage + data_offset1;

      //Now we create space in the heap to then copy the header of the 1st image to the collaged file
      unsigned char* image_data_collage = (unsigned char*) malloc (data_size_collage);
      memcpy (image_data_collage, img_data1, data_offset1);

      // We update the data size value in the header of our collage file
      image_data_collage [2] = (unsigned char) (data_size_collage);
      image_data_collage [3] = (unsigned char) (data_size_collage >> 8);
      image_data_collage [4]= (unsigned char) (data_size_collage >> 16);
      image_data_collage [5] = (unsigned char) (data_size_collage >> 24);

      // We update the width value in the header of our collage file
      image_data_collage [18]= (unsigned char) (width_collage);
      image_data_collage [19]= (unsigned char) (width_collage >> 8);
      image_data_collage [20]= (unsigned char) (width_collage >> 16);
      image_data_collage [21]= (unsigned char) (width_collage >> 24);

      // We update the height value in the header of our collage file
      image_data_collage [22]= (unsigned char) (height_collage);
      image_data_collage [23]= (unsigned char) (height_collage >> 8);
      image_data_collage [24]= (unsigned char) (height_collage >> 16);
      image_data_collage [25]= (unsigned char) (height_collage >> 24);

      int p1 = 0;
      int p2 = 0;
      int x1 = 0;
      int x2 = 0;
      int y1 = 0;
      int y2 = 0;

      // We want to go through each pixel contained in our collage
      for (int i = 0; i < height_collage; i++) {
        for (int j = 0; j < width_collage; j++) {

          //This variable is to know if we added a pixel
          int pixelAdded = 0;

          //Calculating the position of the selected pixel
          int position = data_offset1 + i * bytes_per_row + j * (bits_per_pixel1 / 8);

          // If statement to know if we should print the first image at this pixel
          if (j >= img1_start_col && j < img1_end_col) {
            if(i >= img1_start_row && i < img1_end_row) {

              //Here we find the first byte of the pixel we are adding to our collaged file
              p1 = data_offset1 + y1 * (img_width1 * (bits_per_pixel1 / 8) + padding1) + x1 * (bits_per_pixel1 / 8);

              //Update the values to the ones of the pixel of the 1st image
              image_data_collage [position] = img_data1 [p1];
              image_data_collage [position+1] = img_data1 [p1+1];
              image_data_collage [position+2] = img_data1 [p1+2];

              //Update the position of the next pixel
              pixelAdded = 1;
              x1++;

              if (x1 == img_width1) {
                x1 = 0;
                y1++;
              }
            }
          }

          //If statement to know if we should print the 2nd image at this pixel
          if (j >= img2_start_col && j < img2_end_col) {
            if(i >= img2_start_row && i < img2_end_row) {

              //Calculating the position of the selected pixel
              p2 = data_offset2 + y2 * (img_width2 * (bits_per_pixel2 / 8) + padding2) + x2 * (bits_per_pixel2 / 8);

              //Update the values to the ones of the pixel of the 2nd image
              image_data_collage [position] = img_data2[p2];
              image_data_collage [position + 1] = img_data2 [p2 + 1];
              image_data_collage [position + 2] = img_data2 [p2 + 2];

              //Update the position of the next pixel
              pixelAdded = 1;
              x2++;

              if (x2 == img_width2) {
                x2 = 0;
                y2++;
              }
            }
          }

          //If nothing was printed on the current pixel, then we turn it black
          if (pixelAdded == 0) {
            image_data_collage [position] = 0;
            image_data_collage [position + 1] = 0;
            image_data_collage [position + 2] = 0;
          }
        }

        //Updating the padding
        int pos = data_offset1 + (i + 1) * (bytes_per_row - padding_collage);
        for (int k = 0; k < padding_collage; k++) {
          image_data_collage [pos + 4 + k] = 0;
        }
      }

      //Finally, we turn the collaged image into a file
      FILE * fp = fopen (bmp_result, "w");
      fwrite (image_data_collage, 1, data_size_collage, fp);

      // We can now close the files
      fclose (fp);
      bmp_close (&image_data_collage);
      bmp_close( &img_data1 );
      bmp_close( &img_data2 );

      return 0;
    }
