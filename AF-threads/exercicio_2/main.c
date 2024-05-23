#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <pthread.h>

// Lê o conteúdo do arquivo filename e retorna um vetor E o tamanho dele
// Se filename for da forma "gen:%d", gera um vetor aleatório com %d elementos
//
// +-------> retorno da função, ponteiro para vetor malloc()ado e preenchido
// | 
// |         tamanho do vetor (usado <-----+
// |         como 2o retorno)              |
// v                                       v
double* load_vector(const char* filename, int* out_size);

void *sum(void * arg);

/*void * makeloop(void * arg) {
	int cont = * (int *) arg;
	for (int i = 0; i < cont; i++) {
		contador_global++;
	}
	pthread_exit(NULL);
}*/

struct thread_args
{
    double *a;
    double *b;
    double *result;
    int n_ops;
};


// Avalia o resultado no vetor c. Assume-se que todos os ponteiros (a, b, e c)
// tenham tamanho size.
void avaliar(double* a, double* b, double* c, int size);

void get_tasks_per_thread(int a_size, int n_threads, int* tasks_per_thread_map);

int main(int argc, char* argv[]) {
    // Gera um resultado diferente a cada execução do programa
    // Se **para fins de teste** quiser gerar sempre o mesmo valor
    // descomente o srand(0)
    srand(time(NULL)); //valores diferentes
    //srand(0);        //sempre mesmo valor

    //Temos argumentos suficientes?
    if(argc < 4) {
        printf("Uso: %s n_threads a_file b_file\n"
               "    n_threads    número de threads a serem usadas na computação\n"
               "    *_file       caminho de arquivo ou uma expressão com a forma gen:N,\n"
               "                 representando um vetor aleatório de tamanho N\n",
               argv[0]);
        return 1;
    }
  
    //Quantas threads?
    int n_threads = atoi(argv[1]);
    if (!n_threads) {
        printf("Número de threads deve ser > 0\n");
        return 1;
    }
    //Lê números de arquivos para vetores alocados com malloc
    int a_size = 0, b_size = 0;
    double* a = load_vector(argv[2], &a_size);
    if (!a) {
        //load_vector não conseguiu abrir o arquivo
        printf("Erro ao ler arquivo %s\n", argv[2]);
        return 1;
    }
    double* b = load_vector(argv[3], &b_size);
    if (!b) {
        printf("Erro ao ler arquivo %s\n", argv[3]);
        return 1;
    }
    
    //Garante que entradas são compatíveis
    if (a_size != b_size) {
        printf("Vetores a e b tem tamanhos diferentes! (%d != %d)\n", a_size, b_size);
        return 1;
    }

    //Limits that the number of threads can't be greater than the size of the vectors
    if (n_threads > a_size) {
        n_threads = a_size;
    }

    //Map the number od tasks per thread
    int tasks_per_thread_map[n_threads];
    get_tasks_per_thread(a_size, n_threads, tasks_per_thread_map);

    //Cria vetor do resultado 
    double* c = malloc(a_size*sizeof(double));

    struct thread_args *args;
    args = (struct thread_args *) malloc(sizeof(struct thread_args) * n_threads);

    pthread_t thread[n_threads];
    int counter = 0;
    for (int i = 0; i < n_threads; i++)
    {
        args[i].a = &a[counter];
        args[i].b = &b[counter];
        args[i].result = &c[counter];
        args[i].n_ops = tasks_per_thread_map[i];
        counter += tasks_per_thread_map[i];
        pthread_create(&thread[i], NULL, sum, (void *) &args[i]);
    }

    for (int i = 0; i < n_threads; i++)
    {
        pthread_join(thread[i], NULL);
    }

    // Calcula com uma thread só. Programador original só deixou a leitura 
    // do argumento e fugiu pro caribe. É essa computação que você precisa 
    // paralelizar

    //    +---------------------------------+
    // ** | IMPORTANTE: avalia o resultado! | **
    //    +---------------------------------+
    avaliar(a, b, c, a_size);
    

    //Importante: libera memória
    free(a);
    free(b);
    free(c);
    //free(tasks_per_thread_map);

    return 0;
}

void get_tasks_per_thread(int a_size, int n_threads, int* tasks_per_thread_map) {
    /*This function map the quantity of ops that each thread will execute*/
    if (a_size % n_threads == 0) {
        for (int i = 0; i < n_threads; i++) {
            tasks_per_thread_map[i] = a_size / n_threads;
        }
    } 
    else 
    {
        int tasks_per_thread = a_size / n_threads;
        int remaining_tasks = a_size % n_threads;
        /*plus one for each thread instead the remainder of the division between size
        and n_threads is equal to zero*/
        for (int i = 0; i < remaining_tasks; i++) 
        {
            tasks_per_thread_map[i] = tasks_per_thread+1;
        }
        for (int i = remaining_tasks; i < n_threads; i++) 
        {
            tasks_per_thread_map[i] = tasks_per_thread;
        }
    }
}

void *sum(void * arg)
{
    struct thread_args args = * ((struct thread_args *) arg);
    for (int i = 0; i < args.n_ops; i++)
    {
        *(args.result + i) += (* (args.a + i) + * (args.b + i));
    }
    pthread_exit(NULL);
}