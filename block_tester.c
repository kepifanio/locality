#include "uarray2b.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
void applyPrint(int col, int row, UArray2b_T arr,
      void *elem, void *cl);
int main() {
	
	UArray2b_T arr = UArray2b_new(1, 1, 64000, 1);

      // UArray2b_T new64 = UArray2b_new_64K_block(4, 4, 10000);
      printf("size: %d\n", UArray2b_size(arr));
      printf("width: %d\n", UArray2b_width(arr));
      printf("height: %d\n", UArray2b_height(arr));
      printf("blocksize: %d\n", UArray2b_blocksize(arr));
      UArray2b_map(arr, applyPrint, NULL);
      UArray2b_free(&arr);
      // UArray2b_free(&new64);
      return 0;
}

void applyPrint(int col, int row, UArray2b_T arr,
      void *elem, void *cl)
{
      (void) arr;
      (void) elem;
      (void) cl;
      printf("[%d][%d]\n", col, row);
}
