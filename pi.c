#include "pi.h"

int createReport(const Report *report) {
    FILE *file = fopen("controle.txt", "r");
    int id;

    if (file == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return FALSE;
    }

    fscanf(file, "%d", &id);

    /*De acordo com o identificado que esta no arquivo, printa as informações na tela.
        0 - processo pai.
        1 - processo filho 1
        2 - processo filho 2
    */
    if (id == 0) {
        printf("\n%s\n\n", report->programName);
        printf("%s\n", report->message1);
        printf("%s\n\n", report->message2);
    } else if (id == 1) {
        printf("- Processo Filho: %s\n\n", report->processReport1.identification);
        printf("\t%s\n\n", report->processReport1.numberOfThreads);
        printf("\t%s\n", report->processReport1.start);
        printf("\t%s\n", report->processReport1.end);
        printf("\t%s\n\n", report->processReport1.duration);
        printf("\t%s\n\n", report->processReport1.pi);
    } else if (id == 2) {
        printf("- Processo Filho: %s\n\n", report->processReport2.identification);
        printf("\t%s\n\n", report->processReport2.numberOfThreads);
        printf("\t%s\n", report->processReport2.start);
        printf("\t%s\n", report->processReport2.end);
        printf("\t%s\n\n", report->processReport2.duration);
        printf("\t%s\n\n", report->processReport2.pi);
    }

    fclose(file);
    return TRUE;
}

int createFile(const FileName fileName, String description, const Threads threads){
    FILE *file = fopen(fileName, "w+"); 
    char info[256]; 
    char time[32];
    double total = 0.0;

    if (file == NULL) {
        printf("\nErro ao criar o arquivo %s\n\n", fileName);
        return FALSE;
    }

    fprintf(file, "Arquivo: %s\n", fileName);
    fprintf(file, "Descrição: %s\n\n", description);

    for(int i = 0; i < NUMBER_OF_THREADS; i++){
        total += threads[i].time;
        sprintf(time, "%.2f", threads[i].time);
        pontoParaVirgula(time);
        sprintf(info, "TID %d: %s",  threads[i].tid, time);
        fprintf(file, "%s\n", info);
    }

    fprintf(file, "\nTotal: %.2f s", total);
    fclose(file);
    return TRUE;
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
    unsigned int ultimoTermo = primeiroTermo + PARTIAL_NUMBER_OF_TERMS;  

    for (unsigned int i = primeiroTermo; i < ultimoTermo; i++) {
        double termo = (i % 2 == 0) ? 1.0 / (2.0 * i + 1) : -1.0 / (2.0 * i + 1);
        somaParcial += termo;
    }

    info->somaParcial = somaParcial;
    info->thread.tid = gettid();

    return (void*)info;
}

double calculationOfNumberPi(unsigned int terms){
    DadosThread dadosThreads[NUMBER_OF_THREADS];
    pthread_t threads[NUMBER_OF_THREADS];
    double somaTotal = 0.0;


    //Cria todas as threads e executa a função sumPartial().
    for (int i = 0; i < NUMBER_OF_THREADS; i++) {
        double start_time = getCurrentTimeMillis();
        dadosThreads[i].primeiroTermo = i * PARTIAL_NUMBER_OF_TERMS;
        threads[i] = createThread((unsigned int *)&dadosThreads[i]);
        dadosThreads[i].thread.threadID = threads[i];
        double end_time = getCurrentTimeMillis();
        dadosThreads[i].thread.time = end_time - start_time;
    }

    //Pega o retorno de cada thread que executou a função sumPartial() usando o pthread_join(),
    //e realiza a soma parcial dos resultados.
    for (int j = 0; j < NUMBER_OF_THREADS; j++) {
        void *resultado;
        pthread_join(threads[j], &resultado);
        DadosThread *info = (DadosThread *)resultado;
        somaTotal += info->somaParcial;
    }
    
    /*Pega as informações da thread armazenadas em dadosThreads[i].thread e passa para outro vetor 
    que será utilizado para enviar os dados que serão armazenados no arquivo.*/
    Threads threadsInfo;
    for(int i = 0; i < NUMBER_OF_THREADS; i++){
        threadsInfo[i] = dadosThreads[i].thread;
    }

    FILE *file = fopen("controle.txt", "r");
    int id;

    if (file == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return FALSE;
    }

    fscanf(file, "%d", &id);

    //Verifica qual filho esta em execução para criar o arquivo referente a ele corretamente.
    if(id == 1){
        String descricao;
        FileName fileName;
        strcpy(descricao, "Tempo em segundos das 16 threads do processo filho pi1.");
        strcpy(fileName, "pi1.txt");
        createFile(fileName, descricao, threadsInfo);
    }else if(id == 2){
        String descricao;
        FileName fileName;
        strcpy(descricao, "Tempo em segundos das 16 threads do processo filho pi2.");
        strcpy(fileName, "pi2.txt");
        createFile(fileName, descricao, threadsInfo);
    }

    fclose(file);

    return 4.0 * somaTotal;
}

/*
Pega um string e substitui o ponto por virgula quando encontra.
*/
void pontoParaVirgula(char *str) {
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] == '.') 
            str[i] = ',';
    }
}

/*
Pega o tempo do sistema em milissegundos.
*/
double getCurrentTimeMillis() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec * 1000.0 + (double)tv.tv_usec / 1000.0;
}

/*
Obtem o arquivo de controle e a armazena nele um identificador, para que seja possível ver
qual processo esta sendo executado de arcordo com o número armazenado no arquivo.
0 - processo pai
1 - processo filho 1
2 - processo filho 2
*/
void arquivoControle(int identificador){
    FILE *file = fopen("controle.txt", "w");
    fprintf(file, "%d", identificador);
    fclose(file);
}

void processoPai() {
    Report report;
    int pi1, pi2;
    char message2[100]; 
    pid_t parentPID = getppid();

    strcpy(report.programName, "Cálculo do Número π");
    strcpy(report.message1, "Criando os processos filhos pi1 e pi2...");
    sprintf(message2, "Processo pai (PID %d) finalizou sua execução.", parentPID);
    strcpy(report.message2, message2);
    
    arquivoControle(0);
    createReport(&report);

    // Inicializa os campos da estrutura dos processos filhos
    memset(&report.processReport1, 0, sizeof(report.processReport1));
    memset(&report.processReport2, 0, sizeof(report.processReport2));

    pi1 = fork();

    if (pi1 < 0) {
        printf("Falha ao executar pi %d.\n", 1);
        exit(EXIT_FAILURE);
    }

    // Processo filho 1
    if (pi1 == 0) {
        char identification1[100];

        arquivoControle(1);
        report.processReport1 =  processoFilho(1);
        sprintf(identification1, "pi1 (PID %d)", getpid());
        strcpy(report.processReport1.identification, identification1);

        arquivoControle(1);
        createReport(&report);
        exit(EXIT_SUCCESS);
    }      

    pi2 = fork();

    if (pi2 < 0) {
        printf("Falha ao executar pi %d.\n", 2);
        exit(EXIT_FAILURE);
    }

    // Processo filho 2
    if (pi2 == 0) {
        char identification2[100];

        arquivoControle(2);
        report.processReport2 =  processoFilho(2);
        sprintf(identification2, "pi2 (PID %d)", getpid());
        strcpy(report.processReport2.identification, identification2);
        
        arquivoControle(2);
        createReport(&report);    
        exit(EXIT_SUCCESS);
    }


    if (pi1 > 0 && pi2 > 0) {
        wait(NULL);
        wait(NULL);
    }
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
    
    double start_time = getCurrentTimeMillis();
    //Armazena o horário que começou a ser executado.
    sprintf(processReport.start, "Início: %.2d:%.2d:%.2d",tm->tm_hour,tm->tm_min,tm->tm_sec);
    strcpy(processReport.numberOfThreads, "Nº de threads: 16");

    double soma = calculationOfNumberPi(MAXIMUM_NUMBER_OF_TERMS);
    sprintf(processReport.pi, "Pi = %.9f", soma);
    pontoParaVirgula(processReport.pi);

    //Tempo final.
    time(&segundos);   
    tm = localtime(&segundos);

    //Armazena o horário que terminou a execução.
    sprintf(processReport.end, "Fim: %.2d:%.2d:%.2d",tm->tm_hour,tm->tm_min,tm->tm_sec); 

    //Calcula qual foi o tempo de duração e armazena na variável.
    double end_time = getCurrentTimeMillis();
    double tempoSegundos = (end_time - start_time) / 1000.0;
    sprintf(processReport.duration, "Duração: %.2f s", tempoSegundos); 
    pontoParaVirgula(processReport.duration);
    
    return processReport;
}

int pi(){
    processoPai();
    return EXIT_SUCCESS;
}

int main(){
    setlocale(LC_ALL, "pt-BR"); 
    return pi();
}