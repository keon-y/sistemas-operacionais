Faça um programa multithread que funcione da seguinte forma:

    Inicialmente, a thread principal cria 3 trheads adicionais;
    Em seguida, a thread principal realiza a leitura de números naturais do arquivo texto in.txt, armazenando-os no final de uma lista simplesmente encadeada nomeada L1;
    Uma trhead adicional analisa os números armazenados em L1 e cria outra lista simplesmente encadeada, L2, que contém os mesmos elementos de L1 excetuando os pares maiores que 2;
    Outra thread adicional analisa os números armazenados em L2 e cria outra lista encadeada, L3, que contém os mesmos elementos de L2, excetuando os não primos;
    Outra thread adicional imprime os números primos armazenados em L3.

Para a sincronização, utilize apenas semáforos de contagem nomeados (veja página sem_overview da seção 7 do manual do Linux). Projete uma solução simples, com um número pequeno e fixo de semáforos, porém permitindo todas as threads progredirem simultaneamente.