# Image-filters-MPI
distributed program that applies one or multiple filters (blur, sharpen, smooth, emboss, mean) on ian image

- download sources (includeng in, ref, Makefile)
- (optional)in Makefile, under "distrib", uncomment another line if you like
- run command: "make" than "make distrib"
- output is file "lenna"
- you can compare my output to the ones in ref:
    -> example: diff lenna ref/lenna_color_emboss.pnm
