#include <stdio.h>
#include <mpi.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>


typedef struct 
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
}pixel;

typedef struct {

	char type[2];
	unsigned int width;
	unsigned int height;
	unsigned int maxval;
	unsigned int numcolors;
	pixel **rgb_pixels;
	unsigned char **bw_pixels;

}image;

void readInput(const char * fileName, image *img);
void writeData(const char * fileName, image *img);
void resize(image *in, image * out);
void initMatrix(image *img); /*aloca spatiu unei matrici imagine alb-negru*/
void initRGBMatrix(image *img); /*aloca spatiu unei matrici imagine color*/

void readInput(const char * fileName, image *img) {
	FILE *fin;
	fin = fopen(fileName,"rb");
    
    /* citire detalii imagine */
    fscanf(fin,"%s\n%d %d\n%d\n",img->type,&img->width,&img->height,&img->maxval);

    /* citire input in matrice de structuri Pixel */
    if(strcmp(img->type,"P5") == 0){
    	/* alb-negru */
	    img->numcolors = 1;
	    initMatrix(img);
	    for (int i = 0; i < img->height; i++)
			fread((img->bw_pixels)[i], sizeof(unsigned char), img->width, fin);	
	}
	else{
		/* color */
		img->numcolors = 3;
		initRGBMatrix(img);
		for (int i = 0; i < img->height; i++)
			fread((img->rgb_pixels)[i], sizeof(pixel), img->width, fin);
	}
    
	fclose(fin);
	
}

void writeData(const char * fileName, image *img) {
	FILE *fout;
	fout = fopen(fileName,"wb");
    
    /* scriere detalii imagine de output*/
    fprintf(fout,"%s\n%d %d\n%d\n",img->type,img->width,img->height,img->maxval);
    
	unsigned int i;
	if (img->numcolors == 1){
		/* pt imagine alb-negru */
		for (i = 0; i < img->height; i++)
			fwrite((img->bw_pixels)[i], sizeof(unsigned char), img->width, fout);	
	}
	else if (img->numcolors == 3){
		/* pt imagine color */
		for (i = 0; i < img->height; i++)
			fwrite((img->rgb_pixels)[i], sizeof(pixel), img->width, fout);
	}
	fclose(fout);
}

void resize(image *in, image * out) { 
	/* completare campuri out */
	strcpy(out->type,in->type);
	out->width = in->width;
	out->height = in->height;
	out->maxval = in->maxval;
	out->numcolors = in->numcolors;

	if(out->numcolors == 1)
		initMatrix(out);
	else
		initRGBMatrix(out);

	// copiaza marginile din input in output
	
	for(int i = 1; i < in->height - 1; i++) {
		if(in->numcolors == 3) {
			(out->rgb_pixels[i][0]).R = (in->rgb_pixels[i][0]).R;
			(out->rgb_pixels[i][0]).G = (in->rgb_pixels[i][0]).G;
			(out->rgb_pixels[i][0]).B = (in->rgb_pixels[i][0]).B;

			(out->rgb_pixels[i][out->width - 1]).R = (in->rgb_pixels[i][out->width - 1]).R;
			(out->rgb_pixels[i][out->width - 1]).G = (in->rgb_pixels[i][out->width - 1]).G;
			(out->rgb_pixels[i][out->width - 1]).B = (in->rgb_pixels[i][out->width - 1]).B;

		}
		else {
			out->bw_pixels[i][0] = in->bw_pixels[i][0];
			out->bw_pixels[i][out->width - 1] = in->bw_pixels[i][out->width - 1];
		}
	}
	for(int i = 0; i < in->width; i++) {
		if(in->numcolors == 3) {
			(out->rgb_pixels[0][i]).R = (in->rgb_pixels[0][i]).R;
			(out->rgb_pixels[0][i]).G = (in->rgb_pixels[0][i]).G;
			(out->rgb_pixels[0][i]).B = (in->rgb_pixels[0][i]).B;

			(out->rgb_pixels[out->height - 1][i]).R = (in->rgb_pixels[out->height - 1][i]).R;
			(out->rgb_pixels[out->height - 1][i]).G = (in->rgb_pixels[out->height - 1][i]).G;
			(out->rgb_pixels[out->height - 1][i]).B = (in->rgb_pixels[out->height - 1][i]).B;
		}
		else {
			out->bw_pixels[0][i] = in->bw_pixels[0][i];
			out->bw_pixels[out->height - 1][i] = in->bw_pixels[out->height - 1][i];
		}
	}
 
}


////////////// alte functii /////////////////


void initRGBMatrix(image *img){
	img->rgb_pixels = malloc(sizeof(pixel *) * img->height);
    unsigned int i;
	for(i = 0; i < img->height; i++) {
		(img->rgb_pixels)[i] = malloc(sizeof(pixel) * img->width);
	}
}

void initMatrix(image *img){
	img->bw_pixels = malloc(sizeof(unsigned int *) * img->height);
    unsigned int i;
	for(i = 0; i < img->height; i++) {
		(img->bw_pixels)[i] = malloc(sizeof(unsigned int) * img->width);
	}
}


int main(int argc, char * argv[]) {

	

	// Smooth 
	float A[3][3] = {
		{1, 1, 1},
		{1, 1, 1},
		{1, 1, 1}
	};
	for(int i = 0; i < 3; i++)
		for(int j = 0; j < 3; j++)
			A[i][j] /= 9;

	//Approximative Filter Blur 
	float B[3][3] = {
		{1, 2, 1},
		{2, 4, 2},
		{1, 2, 1}
	};
	for(int i = 0; i < 3; i++)
		for(int j = 0; j < 3; j++)
			B[i][j] /= 16;

	// Sharpen 
	float C[3][3] = {
		{ 0, -2, 0},
		{-2, 11,-2},
		{ 0, -2, 0}
	};
	for(int i = 0; i < 3; i++)
		for(int j = 0; j < 3; j++)
			C[i][j] /= 3;

	// Mean Removal 
	float D[3][3] = {
		{-1,-1, -1},
		{-1, 9, -1},
		{ -1,-1,-1}
	};

	// Emboss
	float E[3][3] = {
		{0, 1, 0},
		{0, 0, 0},
		{0,-1, 0}
	};


	int rank;
	int nProcesses;
	int v;

	MPI_Init(&argc, &argv);
	MPI_Status stat;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);

	int start[nProcesses];
	int end[nProcesses];

	const int nitems=3;
    int          blocklengths[3] = {1,1,1};
    MPI_Datatype types[3] = {MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR};
    MPI_Datatype mpi_pixel_type;
    MPI_Aint     offsets[3];

    offsets[0] = offsetof(pixel, R);
    offsets[1] = offsetof(pixel, G);
    offsets[2] = offsetof(pixel, B);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_pixel_type);
    MPI_Type_commit(&mpi_pixel_type);

	image input;
	image output;

	int count = 0;
	
	//lista filtre
	int num_filters = argc - 3;
	float filter[argc - 3][3][3];
	for(int i = 0; i < argc - 3; i++){
		if (strcmp(argv[i + 3],"smooth") == 0) {
			for(int j = 0; j < 3; j++)
				for(int k = 0; k < 3; k++)
					filter[i][j][k] = A[j][k];
		}
		if (strcmp(argv[i + 3],"blur") == 0) {
			for(int j = 0; j < 3; j++)
				for(int k = 0; k < 3; k++)
					filter[i][j][k] = B[j][k];
		}
		if (strcmp(argv[i + 3],"sharpen") == 0) {
			for(int j = 0; j < 3; j++)
				for(int k = 0; k < 3; k++)
					filter[i][j][k] = C[j][k];
		}
		if (strcmp(argv[i + 3],"mean") == 0) {
			for(int j = 0; j < 3; j++)
				for(int k = 0; k < 3; k++)
					filter[i][j][k] = D[j][k];
		}
		if (strcmp(argv[i + 3],"emboss") == 0) {
			for(int j = 0; j < 3; j++)
				for(int k = 0; k < 3; k++)
					filter[i][j][k] = E[j][k];
		}
	}
	
	//struct timespec start, finish;
	//double elapsed;
	//clock_gettime(CLOCK_MONOTONIC, &start);
	//resize(&input, &output);
	//clock_gettime(CLOCK_MONOTONIC, &finish);
	//elapsed = (finish.tv_sec - start.tv_sec);
	//elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
	
	//clock_gettime(CLOCK_MONOTONIC, &finish);

	if(rank==0) {
		readInput(argv[1], &input);

		resize(&input, &output);

		if(nProcesses == 1) {
			// prelucreaza singur
			float sum1,sum2,sum3;
			unsigned int i,j,k;
		    
			for(k = 0; k < num_filters; k++){

				for(i = 0; i < input.height - 2; i++){
					for(j = 0; j < input.width - 2; j++){

							if(input.numcolors == 3){
									
								// calcul pixel pt Rosu 
								sum1 = ((input.rgb_pixels)[i][j]).R*filter[k][0][0] + ((input.rgb_pixels)[i][j+1]).R*filter[k][0][1] + ((input.rgb_pixels)[i][j+2]).R*filter[k][0][2]
									+ ((input.rgb_pixels)[i+1][j]).R*filter[k][1][0] + ((input.rgb_pixels)[i+1][j+1]).R*filter[k][1][1] + ((input.rgb_pixels)[i+1][j+2]).R*filter[k][1][2]
									+ ((input.rgb_pixels)[i+2][j]).R*filter[k][2][0] + ((input.rgb_pixels)[i+2][j+1]).R*filter[k][2][1] + ((input.rgb_pixels)[i+2][j+2]).R*filter[k][2][2];

								((output.rgb_pixels)[i+1][j+1]).R = sum1;

								//calcul pixel pt Galben 
								sum2 = ((input.rgb_pixels)[i][j]).G*filter[k][0][0] + ((input.rgb_pixels)[i][j+1]).G*filter[k][0][1] + ((input.rgb_pixels)[i][j+2]).G*filter[k][0][2]
									+ ((input.rgb_pixels)[i+1][j]).G*filter[k][1][0] + ((input.rgb_pixels)[i+1][j+1]).G*filter[k][1][1] + ((input.rgb_pixels)[i+1][j+2]).G*filter[k][1][2]
									+ ((input.rgb_pixels)[i+2][j]).G*filter[k][2][0] + ((input.rgb_pixels)[i+2][j+1]).G*filter[k][2][1] + ((input.rgb_pixels)[i+2][j+2]).G*filter[k][2][2];

								((output.rgb_pixels)[i+1][j+1]).G = sum2;

								// calcul pixel pt Albastru 
								sum3 = ((input.rgb_pixels)[i][j]).B*filter[k][0][0] + ((input.rgb_pixels)[i][j+1]).B*filter[k][0][1] + ((input.rgb_pixels)[i][j+2]).B*filter[k][0][2]
									+ ((input.rgb_pixels)[i+1][j]).B*filter[k][1][0] + ((input.rgb_pixels)[i+1][j+1]).B*filter[k][1][1] + ((input.rgb_pixels)[i+1][j+2]).B*filter[k][1][2]
									+ ((input.rgb_pixels)[i+2][j]).B*filter[k][2][0] + ((input.rgb_pixels)[i+2][j+1]).B*filter[k][2][1] + ((input.rgb_pixels)[i+2][j+2]).B*filter[k][2][2];

								((output.rgb_pixels)[i+1][j+1]).B = sum3 ;
							}

							// calcul pixel pentru imagine alb-negru 
							else{
								sum1 = ((input.bw_pixels)[i][j])*filter[k][0][0] + ((input.bw_pixels)[i][j+1])*filter[k][0][1] + ((input.bw_pixels)[i][j+2])*filter[k][0][2]
									+ ((input.bw_pixels)[i+1][j])*filter[k][1][0] + ((input.bw_pixels)[i+1][j+1])*filter[k][1][1] + ((input.bw_pixels)[i+1][j+2])*filter[k][1][2]
									+ ((input.bw_pixels)[i+2][j])*filter[k][2][0] + ((input.bw_pixels)[i+2][j+1])*filter[k][2][1] + ((input.bw_pixels)[i+2][j+2])*filter[k][2][2];

								((output.bw_pixels)[i+1][j+1]) = sum1;

							}
						//}
					}
				}
				// copiaza output in input;
				for(i = 0 ; i < input.height; i++)
					for(j = 0; j < input.width; j++){
						if(input.numcolors == 3){
							((input.rgb_pixels)[i][j]).R = ((output.rgb_pixels)[i][j]).R;
							((input.rgb_pixels)[i][j]).G = ((output.rgb_pixels)[i][j]).G;
							((input.rgb_pixels)[i][j]).B = ((output.rgb_pixels)[i][j]).B;
						}
						else
							input.bw_pixels[i][j] = output.bw_pixels[i][j];
					}
			}
		}
		else {
			for(int id = 1; id < nProcesses; id++){
				double division = ceil(((double)output.height)/(nProcesses - 1));
				double product = id * division;
				
				start[id] = (id - 1) * division;
				if(product > output.height) 
					end[id]= output.height;
				else
					end[id] = product;


				if((id != 1) && (id != (nProcesses - 1))){
					int send_height = end[id] - start[id] + 2;
					MPI_Ssend(&(input.numcolors), 1, MPI_INT, id, 1, MPI_COMM_WORLD);
					MPI_Ssend(&(send_height), 1, MPI_INT, id, 1, MPI_COMM_WORLD);
					MPI_Ssend(&(input.width), 1, MPI_INT, id, 1, MPI_COMM_WORLD);
					for(int i = start[id] - 1; i < end[id] + 1; i++){
						if(input.numcolors == 1)
							MPI_Ssend(input.bw_pixels[i], input.width, MPI_UNSIGNED_CHAR, id, i, MPI_COMM_WORLD);
						else
							MPI_Ssend(input.rgb_pixels[i], input.width, mpi_pixel_type, id, i, MPI_COMM_WORLD);
					}
				}

				// la inceput
				if((id == 1) && (nProcesses > 2)){
					int send_height = end[id] - start[id] + 1;
					MPI_Ssend(&(input.numcolors), 1, MPI_INT, id, 1, MPI_COMM_WORLD);
					MPI_Ssend(&(send_height), 1, MPI_INT, id, 1, MPI_COMM_WORLD);
					MPI_Ssend(&(input.width), 1, MPI_INT, id, 1, MPI_COMM_WORLD);
					for(int i = start[id]; i < end[id] + 1; i++){
						if(input.numcolors == 1)
							MPI_Ssend(input.bw_pixels[i], input.width, MPI_UNSIGNED_CHAR, id, i, MPI_COMM_WORLD);
						else
							MPI_Ssend(input.rgb_pixels[i], input.width, mpi_pixel_type, id, i, MPI_COMM_WORLD);
					}
				}

				// la sfarsit
				if(id == (nProcesses - 1) && (nProcesses > 2)) {
					int send_height = end[id] - start[id] + 1;
					MPI_Ssend(&(input.numcolors), 1, MPI_INT, id, 1, MPI_COMM_WORLD);
					MPI_Ssend(&(send_height), 1, MPI_INT, id, 1, MPI_COMM_WORLD);
					MPI_Ssend(&(input.width), 1, MPI_INT, id, 1, MPI_COMM_WORLD);
					for(int i = start[id] - 1; i < end[id]; i++){
						if(input.numcolors == 1)
							MPI_Ssend(input.bw_pixels[i], input.width, MPI_UNSIGNED_CHAR, id, i, MPI_COMM_WORLD);
						else
							MPI_Ssend(input.rgb_pixels[i], input.width, mpi_pixel_type, id, i, MPI_COMM_WORLD);
					}
				}

				if(nProcesses == 2){
					int send_height = end[id] - start[id];
					MPI_Ssend(&(input.numcolors), 1, MPI_INT, id, 1, MPI_COMM_WORLD);
					MPI_Ssend(&(send_height), 1, MPI_INT, id, 1, MPI_COMM_WORLD);
					MPI_Ssend(&(input.width), 1, MPI_INT, id, 1, MPI_COMM_WORLD);
					for(int i = start[id]; i < end[id]; i++){
						if(input.numcolors == 1){
							MPI_Ssend(input.bw_pixels[i], input.width, MPI_UNSIGNED_CHAR, id, i, MPI_COMM_WORLD);
						}
						else
							MPI_Ssend(input.rgb_pixels[i], input.width, mpi_pixel_type, id, i, MPI_COMM_WORLD);
					}
				}
			}

			//////////// asteapta sa se prelucreze ////////////////

			//////////////// primeste inapoi treaba prelucrata /////////////////////

			MPI_Status s;
			while( count < input.height - 2){
				if(input.numcolors == 1){
					unsigned char buffer[input.width];
					MPI_Recv(buffer, input.width, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &s);
					int index = s.MPI_TAG;
					for(int i = 1; i < input.width - 1; i++){
						output.bw_pixels[index][i] = buffer[i];
					}
				}
				else{
					pixel buffer[input.width];
					MPI_Recv(buffer, input.width, mpi_pixel_type, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &s);
					int index = s.MPI_TAG;
					for(int i = 1; i < input.width - 1; i++){
						output.rgb_pixels[index][i] = buffer[i];
					}
				}
				count++;	
			}
		}
		writeData(argv[2], &output);

	} else { // nu este procesul 0
		
		MPI_Recv(&(input.numcolors), 1, MPI_UNSIGNED, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(&(input.height), 1, MPI_UNSIGNED, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(&(input.width), 1, MPI_UNSIGNED, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Status s[input.height];

		if(input.numcolors == 1){

			initMatrix(&input);
			for(int i = 0; i < input.height; i++){
				MPI_Recv(input.bw_pixels[i], input.width, MPI_UNSIGNED, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &(s[i]));
			}
		}
		else{

			initRGBMatrix(&input);
			for(int i = 0; i < input.height; i++){
				MPI_Recv(input.rgb_pixels[i], input.width, mpi_pixel_type, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &(s[i]));
			}
		}

		// initializeaza si output
		output.width = input.width;
		output.height = input.height;
		output.maxval = input.maxval;
		output.numcolors = input.numcolors;

		if(output.numcolors == 1)
			initMatrix(&output);
		else
			initRGBMatrix(&output);


		////////// prelucreaza ////////////

		float sum1,sum2,sum3;
		unsigned int i,j,k;
	    
		for(k = 0; k < num_filters; k++){

			for(i = 0; i < input.height - 2; i++){
				for(j = 0; j < input.width - 2; j++){
					//if((j + 3 <= input.width) && (i + 3 <= input.height)){
						//daca este imagine color => calcul separat pt fiecare culoare 
						if(input.numcolors == 3){
								
							// calcul pixel pt Rosu 
							sum1 = ((input.rgb_pixels)[i][j]).R*filter[k][0][0] + ((input.rgb_pixels)[i][j+1]).R*filter[k][0][1] + ((input.rgb_pixels)[i][j+2]).R*filter[k][0][2]
								+ ((input.rgb_pixels)[i+1][j]).R*filter[k][1][0] + ((input.rgb_pixels)[i+1][j+1]).R*filter[k][1][1] + ((input.rgb_pixels)[i+1][j+2]).R*filter[k][1][2]
								+ ((input.rgb_pixels)[i+2][j]).R*filter[k][2][0] + ((input.rgb_pixels)[i+2][j+1]).R*filter[k][2][1] + ((input.rgb_pixels)[i+2][j+2]).R*filter[k][2][2];

							((output.rgb_pixels)[i+1][j+1]).R = sum1;

							//calcul pixel pt Galben 
							sum2 = ((input.rgb_pixels)[i][j]).G*filter[k][0][0] + ((input.rgb_pixels)[i][j+1]).G*filter[k][0][1] + ((input.rgb_pixels)[i][j+2]).G*filter[k][0][2]
								+ ((input.rgb_pixels)[i+1][j]).G*filter[k][1][0] + ((input.rgb_pixels)[i+1][j+1]).G*filter[k][1][1] + ((input.rgb_pixels)[i+1][j+2]).G*filter[k][1][2]
								+ ((input.rgb_pixels)[i+2][j]).G*filter[k][2][0] + ((input.rgb_pixels)[i+2][j+1]).G*filter[k][2][1] + ((input.rgb_pixels)[i+2][j+2]).G*filter[k][2][2];

							((output.rgb_pixels)[i+1][j+1]).G = sum2;

							// calcul pixel pt Albastru 
							sum3 = ((input.rgb_pixels)[i][j]).B*filter[k][0][0] + ((input.rgb_pixels)[i][j+1]).B*filter[k][0][1] + ((input.rgb_pixels)[i][j+2]).B*filter[k][0][2]
								+ ((input.rgb_pixels)[i+1][j]).B*filter[k][1][0] + ((input.rgb_pixels)[i+1][j+1]).B*filter[k][1][1] + ((input.rgb_pixels)[i+1][j+2]).B*filter[k][1][2]
								+ ((input.rgb_pixels)[i+2][j]).B*filter[k][2][0] + ((input.rgb_pixels)[i+2][j+1]).B*filter[k][2][1] + ((input.rgb_pixels)[i+2][j+2]).B*filter[k][2][2];

							((output.rgb_pixels)[i+1][j+1]).B = sum3 ;
						}

						// calcul pixel pentru imagine alb-negru 
						else{
							sum1 = ((input.bw_pixels)[i][j])*filter[k][0][0] + ((input.bw_pixels)[i][j+1])*filter[k][0][1] + ((input.bw_pixels)[i][j+2])*filter[k][0][2]
								+ ((input.bw_pixels)[i+1][j])*filter[k][1][0] + ((input.bw_pixels)[i+1][j+1])*filter[k][1][1] + ((input.bw_pixels)[i+1][j+2])*filter[k][1][2]
								+ ((input.bw_pixels)[i+2][j])*filter[k][2][0] + ((input.bw_pixels)[i+2][j+1])*filter[k][2][1] + ((input.bw_pixels)[i+2][j+2])*filter[k][2][2];

							((output.bw_pixels)[i+1][j+1]) = sum1;

						}
					//}
				}
			}
			// copiaza output in input;
			for(i = 1 ; i < input.height - 1; i++)
				for(j = 1; j < input.width - 1; j++){
					if(input.numcolors == 3){
						((input.rgb_pixels)[i][j]).R = ((output.rgb_pixels)[i][j]).R;
						((input.rgb_pixels)[i][j]).G = ((output.rgb_pixels)[i][j]).G;
						((input.rgb_pixels)[i][j]).B = ((output.rgb_pixels)[i][j]).B;
					}
					else
						input.bw_pixels[i][j] = output.bw_pixels[i][j];
				}

			if(nProcesses > 2) {
				// am nevoie de linia de deasupra si/sau de cea de dedesubt facut de procesele vecine
				// trebuie sa trimit prima si ultima linie proceselor vecine
				MPI_Status r1, r2;
				if(input.numcolors == 3){

					pixel buff1[input.width];
					pixel buff2[input.width];
					if(rank % 2 == 0){

						if(rank != nProcesses-1){
							MPI_Ssend(input.rgb_pixels[1], input.width, mpi_pixel_type, rank - 1, s[1].MPI_TAG, MPI_COMM_WORLD);
							MPI_Ssend(input.rgb_pixels[input.height - 2], input.width, mpi_pixel_type, rank + 1, s[input.height - 2].MPI_TAG, MPI_COMM_WORLD);

							MPI_Recv(buff1, input.width, mpi_pixel_type, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &r1);
							MPI_Recv(buff2, input.width, mpi_pixel_type, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &r2);

							if(r1.MPI_SOURCE == rank - 1){
								// cea primit de sus
								for(int k = 1; k < input.width; k++)
									input.rgb_pixels[0][k] = buff1[k];
								for(int k = 1; k < input.width; k++)
									input.rgb_pixels[input.height-1][k] = buff2[k];
							}
							else{ // inseamna ca r1 = rank + 1 => proces de mijloc (primul nu poate fi par si ultimul nu primeste de jos)
								// ce a primit de sus
								for(int k = 1; k < input.width; k++)
									input.rgb_pixels[0][k] = buff2[k];
								// ce a primit de jos
								for(int k = 1; k < input.width; k++)
									input.rgb_pixels[input.height-1][k] = buff1[k];
							}
						}
						else { // rank == nProcesses -1
							MPI_Ssend(input.rgb_pixels[1], input.width, mpi_pixel_type, rank - 1, s[1].MPI_TAG, MPI_COMM_WORLD); // trimite doar sus
							MPI_Recv(buff1, input.width, mpi_pixel_type, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &r1); // primeste doar de sus
							for(int k = 1; k < input.width; k++)
								input.rgb_pixels[0][k] = buff1[k];
						}
						
					}
					else { // rank este impar
						if(rank != nProcesses-1 && rank != 1){

							MPI_Recv(buff1, input.width, mpi_pixel_type, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &r1);
							MPI_Recv(buff2, input.width, mpi_pixel_type, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &r2);

							if(r1.MPI_SOURCE == rank - 1){
								// cea primit de sus
								for(int k = 1; k < input.width; k++)
									input.rgb_pixels[0][k] = buff1[k];
								for(int k = 1; k < input.width; k++)
									input.rgb_pixels[input.height-1][k] = buff2[k];
							}
							else{ // inseamna ca r1 = rank + 1 => proces de mijloc (primul nu poate fi par si ultimul nu primeste de jos)
								// ce a primit de sus
								for(int k = 1; k < input.width; k++)
									input.rgb_pixels[0][k] = buff2[k];
								// ce a primit de jos
								for(int k = 1; k < input.width; k++)
									input.rgb_pixels[input.height-1][k] = buff1[k];
							}
							MPI_Ssend(input.rgb_pixels[1], input.width, mpi_pixel_type, rank - 1, s[1].MPI_TAG, MPI_COMM_WORLD);
							MPI_Ssend(input.rgb_pixels[input.height - 2], input.width, mpi_pixel_type, rank + 1, s[input.height - 2].MPI_TAG, MPI_COMM_WORLD);
						}

						if(rank == nProcesses-1 && nProcesses > 2) { 

							MPI_Recv(buff1, input.width, mpi_pixel_type, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &r1); // primeste doar de sus
							for(int k = 1; k < input.width; k++)
								input.rgb_pixels[0][k] = buff1[k];
							MPI_Ssend(input.rgb_pixels[1], input.width, mpi_pixel_type, rank - 1, s[1].MPI_TAG, MPI_COMM_WORLD); // trimite doar sus
						}

						if(rank == 1 && nProcesses > 2) {
							MPI_Recv(buff1, input.width, mpi_pixel_type, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &r1); // primeste doar de sus
							for(int k = 1; k < input.width; k++)
								input.rgb_pixels[input.height-1][k] = buff1[k];
							MPI_Ssend(input.rgb_pixels[input.height-2], input.width, mpi_pixel_type, rank + 1, s[input.height-2].MPI_TAG, MPI_COMM_WORLD); // trimite doar sus
						}
					}
				}
				else { // grayscale

					unsigned char buff1[input.width];
					unsigned char buff2[input.width];
					if(rank % 2 == 0){

						if(rank != nProcesses-1){
							MPI_Ssend(input.bw_pixels[1], input.width, MPI_UNSIGNED_CHAR, rank - 1, s[1].MPI_TAG, MPI_COMM_WORLD);
							MPI_Ssend(input.bw_pixels[input.height - 2], input.width, MPI_UNSIGNED_CHAR, rank + 1, s[input.height - 2].MPI_TAG, MPI_COMM_WORLD);

							MPI_Recv(buff1, input.width, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &r1);
							MPI_Recv(buff2, input.width, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &r2);

							if(r1.MPI_SOURCE == rank - 1){
								// cea primit de sus
								for(int k = 1; k < input.width; k++)
									input.bw_pixels[0][k] = buff1[k];
								for(int k = 1; k < input.width; k++)
									input.bw_pixels[input.height-1][k] = buff2[k];
							}
							else{ // inseamna ca r1 = rank + 1 => proces de mijloc (primul nu poate fi par si ultimul nu primeste de jos)
								// ce a primit de sus
								for(int k = 1; k < input.width; k++)
									input.bw_pixels[0][k] = buff2[k];
								// ce a primit de jos
								for(int k = 1; k < input.width; k++)
									input.bw_pixels[input.height-1][k] = buff1[k];
							}
						}
						else { // rank == nProcesses -1
							MPI_Ssend(input.bw_pixels[1], input.width, MPI_UNSIGNED_CHAR, rank - 1, s[1].MPI_TAG, MPI_COMM_WORLD); // trimite doar sus
							MPI_Recv(buff1, input.width, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &r1); // primeste doar de sus
							for(int k = 1; k < input.width; k++)
								input.bw_pixels[0][k] = buff1[k];
						}
						
					}
					else { // rank este impar
						if(rank != nProcesses-1 && rank != 1){

							MPI_Recv(buff1, input.width, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &r1);
							MPI_Recv(buff2, input.width, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &r2);

							if(r1.MPI_SOURCE == rank - 1){
								// cea primit de sus
								for(int k = 1; k < input.width; k++)
									input.bw_pixels[0][k] = buff1[k];
								for(int k = 1; k < input.width; k++)
									input.bw_pixels[input.height-1][k] = buff2[k];
							}
							else{ // inseamna ca r1 = rank + 1 => proces de mijloc (primul nu poate fi par si ultimul nu primeste de jos)
								// ce a primit de sus
								for(int k = 1; k < input.width; k++)
									input.bw_pixels[0][k] = buff2[k];
								// ce a primit de jos
								for(int k = 1; k < input.width; k++)
									input.bw_pixels[input.height-1][k] = buff1[k];
							}
							MPI_Ssend(input.bw_pixels[1], input.width, MPI_UNSIGNED_CHAR, rank - 1, s[1].MPI_TAG, MPI_COMM_WORLD);
							MPI_Ssend(input.bw_pixels[input.height - 2], input.width, MPI_UNSIGNED_CHAR, rank + 1, s[input.height - 2].MPI_TAG, MPI_COMM_WORLD);
						}

						if(rank == nProcesses-1 && nProcesses > 2) { 

							MPI_Recv(buff1, input.width, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &r1); // primeste doar de sus
							for(int k = 1; k < input.width; k++)
								input.bw_pixels[0][k] = buff1[k];
							MPI_Ssend(input.bw_pixels[1], input.width, MPI_UNSIGNED_CHAR, rank - 1, s[1].MPI_TAG, MPI_COMM_WORLD); // trimite doar sus
						}

						if(rank == 1 && nProcesses > 2) {
							MPI_Recv(buff1, input.width, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &r1); // primeste doar de sus
							for(int k = 1; k < input.width; k++)
								input.bw_pixels[input.height-1][k] = buff1[k];
							MPI_Ssend(input.bw_pixels[input.height-2], input.width, MPI_UNSIGNED_CHAR, rank + 1, s[input.height-2].MPI_TAG, MPI_COMM_WORLD); // trimite doar sus
						}
					}
				}
			}
		}

		///////// a terminat de prelucrat ///////////////////

		for(int i = 1; i < input.height - 1; i++){
			if(input.numcolors == 1)
				MPI_Ssend(input.bw_pixels[i], input.width, MPI_UNSIGNED_CHAR, 0, s[i].MPI_TAG, MPI_COMM_WORLD);
			else
				MPI_Ssend(input.rgb_pixels[i], input.width, mpi_pixel_type, 0, s[i].MPI_TAG, MPI_COMM_WORLD);
		}
	}

	MPI_Type_free(&mpi_pixel_type);
	MPI_Finalize();
	return 0;
}

/*
		float sum1,sum2,sum3;
		unsigned int i,j,k;
	    
		for(k = 0; k < num_filters; k++){

			for(i = 0; i < input.height - 2; i++){
				for(j = 0; j < input.width - 2; j++){

					//if((j + 3 <= input.width) && (i + 3 <= input.height)){
						//daca este imagine color => calcul separat pt fiecare culoare 
						if(input.numcolors == 3){
								
							// calcul pixel pt Rosu 
							sum1 = ((input.rgb_pixels)[i][j]).R*filter[k][0][0] + ((input.rgb_pixels)[i][j+1]).R*filter[k][0][1] + ((input.rgb_pixels)[i][j+2]).R*filter[k][0][2]
								+ ((input.rgb_pixels)[i+1][j]).R*filter[k][1][0] + ((input.rgb_pixels)[i+1][j+1]).R*filter[k][1][1] + ((input.rgb_pixels)[i+1][j+2]).R*filter[k][1][2]
								+ ((input.rgb_pixels)[i+2][j]).R*filter[k][2][0] + ((input.rgb_pixels)[i+2][j+1]).R*filter[k][2][1] + ((input.rgb_pixels)[i+2][j+2]).R*filter[k][2][2];

							((output.rgb_pixels)[i+1][j+1]).R = sum1;

							//calcul pixel pt Galben 
							sum2 = ((input.rgb_pixels)[i][j]).G*filter[k][0][0] + ((input.rgb_pixels)[i][j+1]).G*filter[k][0][1] + ((input.rgb_pixels)[i][j+2]).G*filter[k][0][2]
								+ ((input.rgb_pixels)[i+1][j]).G*filter[k][1][0] + ((input.rgb_pixels)[i+1][j+1]).G*filter[k][1][1] + ((input.rgb_pixels)[i+1][j+2]).G*filter[k][1][2]
								+ ((input.rgb_pixels)[i+2][j]).G*filter[k][2][0] + ((input.rgb_pixels)[i+2][j+1]).G*filter[k][2][1] + ((input.rgb_pixels)[i+2][j+2]).G*filter[k][2][2];

							((output.rgb_pixels)[i+1][j+1]).G = sum2;

							// calcul pixel pt Albastru 
							sum3 = ((input.rgb_pixels)[i][j]).B*filter[k][0][0] + ((input.rgb_pixels)[i][j+1]).B*filter[k][0][1] + ((input.rgb_pixels)[i][j+2]).B*filter[k][0][2]
								+ ((input.rgb_pixels)[i+1][j]).B*filter[k][1][0] + ((input.rgb_pixels)[i+1][j+1]).B*filter[k][1][1] + ((input.rgb_pixels)[i+1][j+2]).B*filter[k][1][2]
								+ ((input.rgb_pixels)[i+2][j]).B*filter[k][2][0] + ((input.rgb_pixels)[i+2][j+1]).B*filter[k][2][1] + ((input.rgb_pixels)[i+2][j+2]).B*filter[k][2][2];

							((output.rgb_pixels)[i+1][j+1]).B = sum3 ;
						}

						// calcul pixel pentru imagine alb-negru 
						else{
							sum1 = ((input.bw_pixels)[i][j])*filter[k][0][0] + ((input.bw_pixels)[i][j+1])*filter[k][0][1] + ((input.bw_pixels)[i][j+2])*filter[k][0][2]
								+ ((input.bw_pixels)[i+1][j])*filter[k][1][0] + ((input.bw_pixels)[i+1][j+1])*filter[k][1][1] + ((input.bw_pixels)[i+1][j+2])*filter[k][1][2]
								+ ((input.bw_pixels)[i+2][j])*filter[k][2][0] + ((input.bw_pixels)[i+2][j+1])*filter[k][2][1] + ((input.bw_pixels)[i+2][j+2])*filter[k][2][2];

							((output.bw_pixels)[i+1][j+1]) = sum1;

						}
					//}
				}
			}
			// copiaza output in input;
			for(i = 0 ; i < input.height; i++)
				for(j = 0; j < input.width; j++){
					if(input.numcolors == 3){
						((input.rgb_pixels)[i][j]).R = ((output.rgb_pixels)[i][j]).R;
						((input.rgb_pixels)[i][j]).G = ((output.rgb_pixels)[i][j]).G;
						((input.rgb_pixels)[i][j]).B = ((output.rgb_pixels)[i][j]).B;
					}
					else
						input.bw_pixels[i][j] = output.bw_pixels[i][j];
				}
		}
		*/