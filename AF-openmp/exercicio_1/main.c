#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <omp.h>

/*
Tempo(1) = 0.896 secs
speedup(1) = 1
speedup(2) = 0.896/0.459 = 1.95
speedup(4) = 0.896/0.459 = 2.26
speedup(8) = 0.896/0.383 = 2.34
speedup(16) = 0.896/0.382 = 2.34
speedup(32) = 0.896/0.384 = 2.33
*/

void calcular(double* c, int size, int n_threads) {
    /*Setando o número de threads*/
    #pragma omp parallel num_threads(n_threads)
    #pragma omp for schedule(guided, 2)
    for (long long int i = 0; i < size; i++) 
    {
        c[i] = sqrt(i * 32) + sqrt(i * 16 + i * 8) + sqrt(i * 4 + i * 2 + i);
        c[i] -= sqrt(i * 32 * i * 16 + i * 4 + i * 2 + i);
        c[i] += pow(i * 32, 8) + pow(i * 16, 12);
    }
}


int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Uso: %s threads [tamanho]\n", argv[0]);
        return 1;
    }
    int n_threads = atoi(argv[1]);

    int size = argc > 2 ? atoi(argv[2]) : 20000000;

    double *c = (double *) malloc (sizeof(double) * size);
    
    //Guarda ponto de início da computação
    double start = omp_get_wtime();
    calcular(c, size, n_threads);
    double duration = omp_get_wtime()-start; //quanto tempo passou
    printf("n_threads: %d, size: %d, tempo: %.3f secs\n", 
           n_threads, size, duration);

    free(c);

    return 0;
}

