Faça um programa em C para simular o escalonamento de processos de acordo com os seguintes algoritmos:

    FCFS;
    SJF;
    SRTF;
    Prioridade preemptivo;
    Round-Robin.

A estrutura dos processos será informada em um arquivo texto. A linha i desse arquivo conterá a estrutura do processo Pi, composta por diversos números naturais onde:

    O primeiro representa a prioridade do processo: vai de 1 (menor) a 10 (maior);
    O segundo representa o instante de submissão do processo no sistema, em milissegundos;
    Uma sequência de tamanho ímpar, contendo o tempo, em milissegundos, do primeiro pico de CPU, primeiro pico de E/S (se houver), segundo pico de CPU (se houver), etc.;

O programa terá os seguintes argumentos de entrada, nessa ordem:

    Nome do arquivo texto de entrada;
    Tamanho do quantum para o algoritmo Round-Robin, em milissegundos;
    Opção -seq. Quando essa opção for informada as E/S dos processos serão simuladas de forma sequencial, considerando que há apenas 1 dispositivo de E/S. Quando essa opção não for informada, as operações de E/S serão simuladas de forma paralela, considerando que cada processo faz E/S em um dispositivo distinto.

A saída será um arquivo texto, com o mesmo nome do arquivo de entrada acrescido do sufixo ".out". Nesse arquivo de saída devem ser apresentados, para cada algoritmo:

    O diagrama de Gantt;
    A utilização da CPU (em %);
    O throughput (processos executados por segundo);
    O tempo de espera de cada processo e o tempo de espera médio (em milissegundos);
    O tempo de turnaround de cada processo e o tempo de turnaround médio (em milissegundos).