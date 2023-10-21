#include "pi.h"


int createReport(const Report *report){
    return EXIT_SUCCESS;
}

int createFile(const FileName fileName, String description, const Threads *threads){
    return EXIT_SUCCESS;
}

pthread_t createThread(unsigned int *terms){
    pthread_t thread_id;

    if (pthread_create(&thread_id, NULL, sumPartial, terms) != 0) {
        printf("Erro ao criar a thread %lu\n", thread_id);
        return (pthread_t)-1;
    }

    return thread_id;
}

void* sumPartial(void *terms){
    DadosThread *info = (DadosThread *)terms;
    double somaParcial = 0.0;
    unsigned int primeiroTermo = info->primeiroTermo;
    unsigned int ultimoTermo = primeiroTermo + NUM_PARCIAL_TERMOS;  

    for (unsigned int i = primeiroTermo; i < ultimoTermo; i++) {
        double termo = (i % 2 == 0) ? 1.0 / (2.0 * i + 1) : -1.0 / (2.0 * i + 1);
        somaParcial += termo;
    }

    info->somaParcial = somaParcial;

    return (void*)info;
}

double calculationOfNumberPi(unsigned int terms){
    DadosThread dadosThreads[NUMBER_OF_THREADS];
    pthread_t threads[NUMBER_OF_THREADS];
    double somaTotal = 0.0;

    for (int i = 0; i < NUMBER_OF_THREADS; i++) {
        dadosThreads[i].primeiroTermo = i * NUM_PARCIAL_TERMOS;
        threads[i] = createThread((unsigned int *)&dadosThreads[i]);
    }

    for (int j = 0; j < NUMBER_OF_THREADS; j++) {
        void *resultado;
        pthread_join(threads[j], &resultado);
        DadosThread *info = (DadosThread *)resultado;
        somaTotal += info->somaParcial;
    }
    
    return 4.0 * somaTotal;
}

void criaProcesso() {
    Report report;
    int pi;

    strcpy(report.programName, "Cálculo do Número π");
    strcpy(report.message1, "Criando os processos filhos pi1 e pi2...");

    for(int i=1; i <= 2; i++){
        pi = fork();

        if (pi < 0) {
            printf("Falha ao executar pi %d.\n", i);
            exit(EXIT_FAILURE);
        }

        if (pi == 0) {
            char identification[100];
            
            if(i == 1){
                report.processReport1 =  processoFilho(i);
                sprintf(identification, "pi1 (PID %d)", getpid());
                strcpy(report.processReport1.identification, identification);
                printf("Processo %d finalizado. %s \n\n", i, report.processReport1.pi);
                break;
            }else if(i == 2){
                report.processReport2 =  processoFilho(i);
                sprintf(identification, "pi2 (PID %d)", getpid());
                strcpy(report.processReport2.identification, identification);
                printf("Processo %d finalizado. %s \n\n", i, report.processReport2.pi);
            }
        }
    }

    // Processo pai após a criação dos filhos.
    for(int i=1; i <= 2; i++)
        wait(NULL); 

    char message2[100]; 
    pid_t parentPID = getppid();
    sprintf(message2, "Processo pai (PID %d) finalizou sua execução.", parentPID);
    strcpy(report.message2, message2);
}

ProcessReport processoFilho(int idFilho){
    //Tempo inicial.
    time_t tempoInicial = time(NULL), segundos;
    time(&segundos);
    tm = localtime(&segundos);
    clock_t contagem[2];
    contagem[0] = clock();
    contagem[1] = clock();
    time_t rawtime;
    struct tm * timeinfo;

    ProcessReport processReport;
    
    //Armazena o horário que começou a ser executado.
    sprintf(processReport.start, " Início: %.2d:%.2d:%.2d",tm->tm_hour,tm->tm_min,tm->tm_sec);
    strcpy(processReport.numberOfThreads, "Nº de threads: 16");

    double soma = calculationOfNumberPi(NUM_MAX_TERMOS);
    sprintf(processReport.pi, "Pi = %.9f", soma);

    //Tempo final.
    time(&segundos);   
    tm = localtime(&segundos);

    //Armazena o horário que terminou a execução.
    sprintf(processReport.end, "Fim: %.2d:%.2d:%.2d",tm->tm_hour,tm->tm_min,tm->tm_sec); 

    //Calcula qual foi o tempo de duração e armazena na variável.
    double tempoSegundos = difftime(time(NULL),tempoInicial);
    double tempoMS = (contagem[1] - contagem[0]) * 1000.0 / CLOCKS_PER_SEC;
    sprintf(processReport.duration, "Duração: %.2f s", tempoSegundos); 
    
    return processReport;
}

int pi(){
    criaProcesso();
    return EXIT_SUCCESS;
}

int main(){
    setlocale(LC_ALL, "pt-BR"); 
    return pi();
}