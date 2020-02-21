/* Duta Viorel-Ionut, 331CB */
#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>
#include <pthread.h>

int P, N;
double complex *out_ft = NULL;	// rezultatul transformarii
double *v;						// datele de intrare

typedef struct ft_arg {
	double *vec;				// pointer catre vectorul cu valori
	int thread_id;				// id-ul thread-ului
} FT;


void* ft_parallel(void *arg) {

	int k, t;
	FT arg_aux = *(FT *) arg;
	double complex sum = 0.0;
	int k_max = (arg_aux.thread_id + 1)*N / P;				// valoarea maxima pentru k

	for(k = (arg_aux.thread_id)*N / P; k < k_max ; k++) {	// transformarea discreta fourier
		sum = 0.0;
		for(t = 0; t < N; t++) {
			double u = 2 * M_PI * t * k / N;				// formula transformarii 
			sum += arg_aux.vec[t] * cexp(-u * I);
		}
		out_ft[k] = sum;
	}
	return NULL;
}

// citirea datelor din fisier
void read_f(char *nume_f_in) {

	int i;
	FILE *in_f;
	in_f = fopen(nume_f_in, "r");	// argv[1]

	fscanf(in_f, "%d\n", &N);		// numarul valorilor 
	v = calloc(N, sizeof(double));	// vectorul cu valorile primite ca parametru
	// vectorul cu rezultatul transformarii
	out_ft = calloc(N, sizeof(double complex));

	for(i = 0; i < N; i++) 
		fscanf(in_f, "%lg\n", &v[i] );

	fclose(in_f);					// inchiderea fisierului
}

// scrierea rezultatului transformarii
void write_f(double complex *out, char *nume_f_out) {

	int i;
	FILE *out_f;
	out_f = fopen(nume_f_out, "w+");// argv[2]

	fprintf(out_f, "%d\n", N);		// numarul de numere complexe

	// rezultatul transformarii (parte reala, parte imaginara)
	for(i = 0; i < N; i++) {
		fprintf(out_f, "%f %f\n", creal(out[i]), cimag(out[i]));
	}

	free(v);						// eliberarea memoriei
	free(out_ft);
	fclose(out_f);					// inchiderea fisierului
}


int main(int argc, char * argv[]) {

	if(argc < 4) {					// daca nu primesc toate argumentele
		exit(0);
	}

	int i;
	P = atoi(argv[3]);				// numarul de thread-uri

	read_f(argv[1]);				// citirea valorilor din fisierul argv[1]

	pthread_t tid[P];

	FT *thread_v = calloc(P, sizeof(FT));	// argumentele functiei
	for(i = 0; i < P; i++) {
		thread_v[i].thread_id = i;			// id-ul
		thread_v[i].vec = v;				// pointer catre vectorul de intrare
	}

	// apelul paralel
	for(i = 0; i < P; i++) {
		pthread_create(&(tid[i]), NULL, ft_parallel, &(thread_v[i]));
	}

	// inchiderea thread-urilor
	for(i = 0; i < P; i++) {
		pthread_join(tid[i], NULL);		
	}


	write_f(out_ft, argv[2]);		// scrierea rezultatului in argv[2]

	return 0;
}
