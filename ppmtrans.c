/**************************************************************
 *                      ppmtrans.c
 *
 *    Assignment: locality
 *    Authors: Milo Shields (mshiel04) &
               Katherine Epifanio (kepifa01)
 *    Date: 10.13.20
 *
 *    Summary:
 *
 *          This file contains the main driver code for
 *          performing transformations on images provided
 *          from a file or from standard input. To handle
 *          different queries from the command line, ppmtrans.c
 *          uses the methods suite to direct the image to
 *          either a UArray2 or a blocked UArray2.
 *
 **************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "assert.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "pnm.h"
#include "stdbool.h"
#include "cputiming.h"
typedef A2Methods_UArray2 A2;
typedef void transform_fun(int i, int j, A2 array2, void *ptr, void *cl);
typedef struct temp_rgb{
        unsigned red;
        unsigned green;
        unsigned blue;
} temp_rgb;

#define SET_METHODS(METHODS, MAP, WHAT) do {                    \
        methods = (METHODS);                                    \
        assert(methods != NULL);                                \
        map = methods->MAP;                                     \
        if (map == NULL) {                                      \
                fprintf(stderr, "%s does not support "          \
                                WHAT "mapping\n",               \
                                argv[0]);                       \
                exit(1);                                        \
        }                                                       \
} while (0)


static void
usage(const char *progname)
{
        fprintf(stderr, "Usage: %s [-rotate <angle>] "
                        "[-{row,col,block}-major] [filename]\n",
                        progname);
        exit(1);
}

void set_rotation(int rotation,transform_fun **transform);
void rotate_0(int i, int j, A2 array2, A2Methods_Object *ptr, void *cl);
void rotate_90(int i, int j, A2 array2, A2Methods_Object *ptr, void *cl);
void rotate_180(int i, int j, A2 array2, A2Methods_Object *ptr, void *cl);
void dim_check(int rotation, Pnm_ppm *image);
void make_temp_array(A2 *temp, Pnm_ppm *image, bool blocked, int rotation);
void reportTime(char *time_file_name, int rotation,
      char* mappingType, Pnm_ppm newImage, double timeRecorded);
/* NEEDSWORK: our main function should be more modular and call
 *     other helper functions.
 */
int main(int argc, char *argv[])
{
    char *time_file_name = NULL;
    char *mappingType = "Default";
    int rotation = 0;
    int i;
    bool blocked = false;

    FILE *fp;
    fp = NULL;
    (void) time_file_name;
    /* default to UArray2 methods */
    A2Methods_T methods = uarray2_methods_plain;
    assert(methods);

    /* default to best map */
    A2Methods_mapfun *map = methods->map_default;
    assert(map);

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-row-major") == 0) {
            mappingType = "Row Major";
            SET_METHODS(uarray2_methods_plain, map_row_major,
                "row-major");
        } else if (strcmp(argv[i], "-col-major") == 0) {
            mappingType = "Column Major";
            SET_METHODS(uarray2_methods_plain, map_col_major,
                "column-major");
        } else if (strcmp(argv[i], "-block-major") == 0) {
            mappingType = "Block Major";
            SET_METHODS(uarray2_methods_blocked, map_block_major,
                "block-major");
            blocked = true;
        } else if (strcmp(argv[i], "-rotate") == 0) {
            if (!(i + 1 < argc)) {      /* no rotate value */
                usage(argv[0]);
            }
            char *endptr;
            rotation = strtol(argv[++i], &endptr, 10);
            if (!(rotation == 0 || rotation == 90 ||
                rotation == 180 || rotation == 270)) {
                fprintf(stderr,
                    "Rotation must be 0, 90 180 or 270\n");
                usage(argv[0]);
            }
            if (!(*endptr == '\0')) {    /* Not a number */
                usage(argv[0]);
            }
        } else if (strcmp(argv[i], "-flip") == 0) {
            fprintf(stderr, "The -flip option is unimplemented\n");
            exit(EXIT_FAILURE);
        } else if(strcmp(argv[i], "-transpose") == 0) {
            fprintf(stderr, "The -transpose option is unimplemented\n");
            exit(EXIT_FAILURE);
        }else if (strcmp(argv[i], "-time") == 0) {
            time_file_name = argv[++i];
        } else if (*argv[i] == '-') {
            fprintf(stderr, "%s: unknown option '%s'\n", argv[0],
            argv[i]);
        } else if (argc - i > 1) {
            fprintf(stderr, "Too many arguments\n");
            usage(argv[0]);
        } else if (fp == NULL) {
            fp = fopen(argv[i],"rb");
            if (fp == NULL) {
                fprintf(stderr,"%s %s %s\n","Could not open file",
                    argv[i], "for reading");
                exit(EXIT_FAILURE);
            }
        } else {
            break;
        }
    }

    if (fp == NULL) {
        fp = stdin;
    }


    /* make original image from file */
    Pnm_ppm image = Pnm_ppmread(fp, methods);
    assert(image != NULL);
    /* select transformation function */
    transform_fun *fun_p;
    set_rotation(rotation,&fun_p);
    assert(fun_p != NULL);

    /* make temporary array of correct dimensions */
    A2 temp;
    make_temp_array(&temp,&image, blocked,rotation);
    assert(temp != NULL);

    /* write the transformed image to the temp array */
    CPUTime_T cpuTimer;
    cpuTimer = CPUTime_New();
    CPUTime_Start(cpuTimer);
    map(temp,fun_p,&image);
    double timeRecorded = CPUTime_Stop(cpuTimer);
    reportTime(time_file_name, rotation, mappingType,
        image, timeRecorded);

    /* switch temp array with original array */
    dim_check(rotation,&image);
    image->methods->free(&(image->pixels));
    image->pixels = temp;

    /* write to stdout, close files, free structures */
    Pnm_ppmwrite(stdout,image);
    fclose(fp);
    Pnm_ppmfree(&image);
    CPUTime_Free(&cpuTimer);

    exit(EXIT_SUCCESS);
}

/* This function directs information about the image being
 *     transformed and the CPUTime_T to the file time_file_name.
 */
void reportTime(char *time_file_name, int rotation,
      char* mappingType, Pnm_ppm newImage, double timeRecorded)
{
    if (time_file_name == NULL) {
        return;
    }

    int pixels = (newImage->width)*(newImage->height);
    double timePerPixel = timeRecorded / pixels;
    FILE *time_report = fopen(time_file_name, "a");
    assert(time_report != NULL);

    fprintf(time_report, "********* CPU TIME REPORT *********\n\n");
    fprintf(time_report, "All reported tests performed");
    fprintf(time_report, " on same machine via halligan server\n");
    fprintf(time_report, "Rotation Performed:      %d\n", rotation);
    fprintf(time_report, "Mapping Type:      %s\n", mappingType);
    fprintf(time_report, "Number of Pixels:      %d\n", pixels);
    fprintf(time_report, "Total Time:      %f\n", timeRecorded);
    fprintf(time_report, "Time Per Pixel:      %f\n", timePerPixel);

    fclose(time_report);
}

/*
 * This function is called in situations where a 90-degree
 *     transformation is to be performed on an image. In those
 *     situations, the dimensinos of our Pnm_ppm for the
 *     new transformed image must be changed such that width
 *     and height from the original image are swapped in the
 *     new image. It is a checked runtime error for the Pnm_ppm
 *     pointer passed to the function to be NULL.
 */
void dim_check(int rotation, Pnm_ppm *image)
{
    assert(image != NULL && *image != NULL);
    if (rotation == 90) {
        int tempw = (*image)->height;
        (*image)->height = (*image)->width;
        (*image)->width = tempw;
    }
}

/*
 * This function creates a temporary A2 in which to store pixel
 *     values in the new index at which they will be stored in
 *     the new transformed imaged. It is a checked runtime error
 *     for the A2 or Pnm_ppm pointers to be NULL.
 */
void make_temp_array(A2 *temp, Pnm_ppm *image,
    bool blocked, int rotation){

    assert(temp != NULL);
    assert(image != NULL && *image != NULL);
    if (blocked && (rotation != 90)) {
        *temp = (*image)->methods->new_with_blocksize((*image)->width,
                (*image)->height, sizeof(struct Pnm_rgb),
                (int)sqrt(((*image)->width) * ((*image)->height)));
    } else if (blocked && (rotation == 90)) {
        *temp = (*image)->methods->new_with_blocksize((*image)->height,
                (*image)->width, sizeof(struct Pnm_rgb),
                (int)sqrt(((*image)->width) * ((*image)->height)));
    } else if (rotation != 90) {
        *temp = (*image)->methods->new((*image)->width,
                (*image)->height, sizeof(struct Pnm_rgb));
    } else {
        *temp = (*image)->methods->new((*image)->height,
                (*image)->width, sizeof(struct Pnm_rgb));
    }
}


void set_rotation(int rotation,transform_fun **transform){
        if (rotation == 0) {
                *transform =  &(rotate_0);
        }
        if (rotation == 90) {
                *transform = &(rotate_90);
        }
        if (rotation == 180) {
                *transform =  &(rotate_180);
        }
}

/* This function is an apply function which is passed to the
 *     mapping function in situations in which a transformation
 *     of 0 degrees is to be performed on an image.
 */
void rotate_0(int i, int j, A2 array2, A2Methods_Object *ptr, void *cl) {

    (void) array2;
    Pnm_ppm image = *(Pnm_ppm*)cl;
    /* get pointer to the struct Pnm_pgm in the image structure */
    Pnm_rgb source_rgb =
            ((Pnm_rgb)image->methods->at(image->pixels,i,j));
    /* make a pointer to a struct Pnm_pgm on the heap */
    Pnm_rgb dest_rgb = malloc(sizeof(struct Pnm_rgb));
    assert(dest_rgb != NULL);

    /* set VALUE of allocated struct equal to VALUE of struct's image */
    *dest_rgb = *source_rgb;
    /* set VALUE of space in A2 equal to VALUE of allocated struct */
    *(Pnm_rgb) ptr = *dest_rgb;

    free(dest_rgb);
}

/* This function is an apply function which is passed to the
 *     mapping function in situations in which a transformation
 *     of 180 degrees is to be performed on an image.
 */
void rotate_180(int i, int j, A2 array2, A2Methods_Object *ptr,
    void *cl) {

    (void) array2;
    Pnm_ppm image = *(Pnm_ppm*)cl;
    /* get pointer to the struct Pnm_pgm in the image structure */
    Pnm_rgb source_rgb = ((Pnm_rgb)image->methods->at(image->pixels,
            (image->width)-i-1, (image->height)-j-1));
    /* make a pointer to a struct Pnm_pgm on the heap */
    Pnm_rgb dest_rgb = malloc(sizeof(struct Pnm_rgb));
    assert(dest_rgb != NULL);

    /* set VALUE of allocated struct equal to VALUE of struct from image */
    *dest_rgb = *source_rgb;
    /* set VALUE of space in A2 equal to VALUE of allocated struct */
    *(Pnm_rgb) ptr = *dest_rgb;

    free(dest_rgb);
}

/* This function is an apply function which is passed to the
 *     mapping function in situations in which a transformation
 *     of 90 degrees is to be performed on an image.
 */
void rotate_90(int i, int j, A2 array2,
    A2Methods_Object *ptr, void *cl) {

    (void) array2;
    Pnm_ppm image = *(Pnm_ppm*)cl;
    /* get pointer to the struct Pnm_pgm in the image structure */
    Pnm_rgb source_rgb = ((Pnm_rgb)image->methods->at(image->pixels,
            j, image->height-i-1));
    /* make a pointer to a struct Pnm_pgm on the heap */
    Pnm_rgb dest_rgb = malloc(sizeof(struct Pnm_rgb));
    assert(dest_rgb != NULL);

    /* set VALUE of allocated struct equal to VALUE of struct from image */
    *dest_rgb = *source_rgb;
    /* set VALUE of space in A2 equal to VALUE of allocated struct */
    *(Pnm_rgb) ptr = *dest_rgb;

    free(dest_rgb);
}
