/* Duta Viorel-Ionut, 331CB */
#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>
#include <pthread.h>

typedef double complex cplx;

typedef struct th_param
{
    cplx *in;           // pointer catre vectorul 1
    cplx *out;          // pointer catre vectorul 2
    int step;           // numarul de iteratii (2^(iteratie) )
} THP;


int N, P;               // numarul de valori si procese
int P_aux = 0;          // valoare auxiliara pentru cazul P == 4
cplx *vec;              // vector de numere complexe 
cplx *out_fft;          // vector de numere complexe 
pthread_t tid[4];       // P poate sa fie 1,2 sau 4

/*   Algoritmul de pe Rosetta code modificat ca sa primeasca un
singur parametru. Acest lucru permite apelarea cu pthread_create.
https://rosettacode.org/wiki/Fast_Fourier_transform#C */
void *fft_parallel(void *arg) {

    THP arg_ = *(THP *) arg;    

    if (arg_.step < N) {
        int stp = arg_.step;

        THP par1, par2;
        par1.step = stp * 2;
        par1.in = &(arg_.out[0]);
        par1.out = &(arg_.in[0]);

        par2.step =stp* 2;
        par2.in = &(arg_.out[stp]);
        par2.out = &(arg_.in[stp]);

        fft_parallel(&par1);
        fft_parallel(&par2);

        int i;
        for (i = 0; i < N; i += 2 * stp) {
            cplx t = cexp(-I * M_PI * i / N);
            t = t * arg_.in[i + stp];
            arg_.out[i / 2]     = arg_.in[i] + t;
            arg_.out[(i + N)/2] = arg_.in[i] - t;
        }
    }
    return NULL;
}

/*  Functie care apeleaza paralel functia fft_parallel. O folosesc
pentru cazul P == 4. Nu am nevoie de bariera datorita modului in 
care sunt folosite datele.
*/
void *fft_4(void *arg) {
    THP arg_ = *(THP *) arg;
        int i;

    if (arg_.step < N) {
        int stp = arg_.step;
        int id_join1, id_join2;                 // valori auxiliare pentru join

        THP par1, par2;
        par1.step = stp * 2;
        par1.in = &(arg_.out[0]);
        par1.out = &(arg_.in[0]);
        par2.step =stp* 2;
        par2.in = &(arg_.out[stp]);
        par2.out = &(arg_.in[stp]);

        // apelul paralel
        pthread_create(&(tid[P_aux]), NULL, fft_parallel, &par1);
        id_join1 = P_aux++;
        pthread_create(&(tid[P_aux]), NULL, fft_parallel, &par2);
        id_join2 = P_aux++;

        pthread_join(tid[id_join1], NULL);
        pthread_join(tid[id_join2], NULL);
        
        for (i = 0; i < N; i += 2 * stp) {
            cplx t = cexp(-I * M_PI * i / N);
            t = t * arg_.in[i + stp];
            arg_.out[i / 2]     = arg_.in[i] + t;
            arg_.out[(i + N)/2] = arg_.in[i] - t;
        }
    }
    return NULL;
}

/* functie pentru citirea datelor din fisier */
void read_f(char *nume_f_in) {

    int i;
    double aux;

    FILE *in_f;
    in_f = fopen(nume_f_in, "r");

    fscanf(in_f, "%d\n", &N);               // numarul elementelor
    vec = calloc(N, sizeof(cplx));          // datele citite din fisier
    out_fft = calloc(N, sizeof(cplx));      // rezultatul transformarii

    for(i = 0; i < N; i++) {
        fscanf(in_f, "%lg\n", &aux );       // in fisier sunt numere reala
        vec[i] = aux;                       // vec si out sunt egale la inceput
        out_fft[i] = vec[i];
    }

    fclose(in_f);
}

/* functie pentru scrierea datelor din fisier */
void write_f(char *nume_f_out) {

    int i;
    FILE *out_f;
    out_f = fopen(nume_f_out, "w+");

    fprintf(out_f, "%d\n", N);

    for(i = 0; i < N; i++)
        fprintf(out_f, "%f %f\n", creal(out_fft[i]), cimag(out_fft[i]));

    fclose(out_f);
    free(vec);
    free(out_fft);
}

int main(int argc, char * argv[]) {

    int i;

    if(argc < 4) {              // ./a.out f.in f.out P
        exit(0);
    }

    P = atoi(argv[3]);          // numarul de thread-uri(1, 2 sau 4)

    read_f(argv[1]);            

    THP arg;
    arg.step = 1;
    arg.in = vec;
    arg.out = out_fft;

    if(P == 1) {            // cazul secvential
        fft_parallel(&arg);
    } else if(P == 2) {
        THP arg2;
        arg.step = 2;
        arg.in = out_fft; arg.out = vec;
        arg2.step = 2;
        arg2.in = &out_fft[1]; arg2.out = &vec[1];
        
        /* apelez paralel primele doua functii, iar datorita recursivitatii,
        functia se executa paralel  */
        pthread_create(&(tid[0]), NULL, fft_parallel, &arg);
        pthread_create(&(tid[1]), NULL, fft_parallel, &arg2);

        pthread_join(tid[0], NULL);
        pthread_join(tid[1], NULL);

        for (i = 0; i < N; i += 2) {
            cplx t = cexp(-I * M_PI * i / N);
            t = t * vec[i + 1];
            out_fft[i / 2]     = vec[i] + t;
            out_fft[(i + N)/2] = vec[i] - t;    
        }
    } else if(P == 4) {
        THP arg2;
        arg.step = 2;
        arg.in = out_fft; arg.out = vec;
        arg2.step = 2;
        arg2.in = &out_fft[1]; arg2.out = &vec[1];
        
        fft_4(&arg);
        fft_4(&arg2);

        for (i = 0; i < N; i += 2) {
            cplx t = cexp(-I * M_PI * i / N);
            t = t * vec[i + 1];
            out_fft[i / 2]     = vec[i] + t;
            out_fft[(i + N)/2] = vec[i] - t;    
        }
    }

    write_f(argv[2]);           // rezultatul transformarii si eliberarea memoriei

    return 0;
}
