# locality-
Uses two-dimensional array to measure the performance of image rotations by using three different array-access patterns with different locality properties. 

----------------------README-------------------------

----------mshiel04-----------------kepifa01----------

----------------------ppmtrans-----------------------

by Kate Epifanio and Milo Shields

Acknowledgements: We would like to thank the TA suite
profusely for being a beacon of hope in these dark
times. Additionally, we consulted a certain Edward
Hatfield a single time, but he provided no useful
information.

Degree of Implementation: We successfully implemented
the rotate 0, rotate 90, rotate 180, and time options
for the ppmtrans program. Our array2b is fully
functional (to the best of our knowledge). We also 
implemented row, column, and block-major mapping
options for these transform types.
However, we left unimplemented the flip and transpose
options as we grew closer to the deadline.

Solution Architecture:

uarray2b

Our UArray2b is implemented as a UArray2 containing
several UArrays, one corresponding to each block.
Mapping for this implementation involves retrieving
each block (UArray) and then iterating through that
block. However, the apply function is only applied
if that cell in the UArray2b is valid (within
originally specified bounds of width and height).
We can guarantee that cells are stored near
eachother in memory when in the same block, as
Hanson's UArray stores cells in a contiguous chunk
of memory.

ppmtrans

Our ppmtrans was implemented by using the provided
A2Methods suite, and works in general by using two
main data structures, a Pnm_ppm image structure,
and an external A2, which is initialized either
as a UArray2b or a UArray2 depending on mapping
option specified.

A function pointer is created to the rotation
function corresponding to the query, and that
function is then used as an apply function to 
map over the external 2d array, copying the
appropriate Pnm_rgb structures over to the array.
After this, the image is changed so that it points
to this external array instead of the original.


********* CPU TIME REPORT *********


All reported tests performed on same machine via halligan server

Rotation Performed:      90

Mapping Type:      Row Major

Number of Pixels:      13454208

Total Time:      1344347188.000000

Time Per Pixel:      99.920203

********* CPU TIME REPORT *********


All reported tests performed on same machine via halligan server

Rotation Performed:      90

Mapping Type:      Column Major

Number of Pixels:      13454208

Total Time:      1411699449.000000

Time Per Pixel:      104.926239

********* CPU TIME REPORT *********


All reported tests performed on same machine via halligan server

Rotation Performed:      90

Mapping Type:      Block Major

Number of Pixels:      13454208

Total Time:      1267469477.000000

Time Per Pixel:      94.206175

********* CPU TIME REPORT *********


All reported tests performed on same machine via halligan server

Rotation Performed:      180

Mapping Type:      Row Major

Number of Pixels:      13454208

Total Time:      695270118.000000

Time Per Pixel:      51.676778

********* CPU TIME REPORT *********


All reported tests performed on same machine via halligan server

Rotation Performed:      180

Mapping Type:      Column Major

Number of Pixels:      13454208

Total Time:      2076713396.000000

Time Per Pixel:      154.354191

********* CPU TIME REPORT *********


All reported tests performed on same machine via halligan server

Rotation Performed:      180

Mapping Type:      Block Major

Number of Pixels:      13454208

Total Time:      1435697021.000000

Time Per Pixel:      106.709887



Rotate 90:

Blocked access is the fastest as it reads an entire block
(a contiguous chunk) and writes to a contiguous chunk, giving
it less cache misses and less time per pixel (tpp).

Row-major access worked decently well, taking the average of 
the three times, as expected. Reading row-major allows for
contiguous reading from the Uarray, but may require non
contiguous writes. This gave it an okay tpp.

Column-major access was the worst, as expected. It cannot
read from contiguous memory when mapping, meaning the number
of cache misses likely skyrocket, resulting in an inferior
tpp.


Rotate 180:

Row-major access worked fantastically here. This is expected
behavior, as reading and writing to rows, or contiguous
chunks simultaneously allows it to have little or no cache
misses other than cold misses.

Blocked-major access was right in the middle of the two -
we are yet to formulate solid reasoning for why it didn't
have comparable performance to row-major.

This was column-major mapping's worst nightmare. It got
by far the highest tpp, due to the fact that it had cold
misses at nearly every turn in both reading and writing.
