build: homework
homework: homework.c
	mpicc -o homework homework.c -lm -Wall
serial: homework
	mpirun -np 1 homework imagini.in
distrib: homework
	mpirun -np 4 homework in/lenna_color.pnm lenna emboss
	#mpirun -np 4 homework in/lenna_color.pnm lenna sharpen
	#mpirun -np 4 homework in/lenna_bw.pgm lenna blur sharpen smooth emboss mean blur sharpen smooth emboss mean
clean:
	rm -f homework
