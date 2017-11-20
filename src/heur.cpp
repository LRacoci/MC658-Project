/*
    Nome (RA)
    Lucas Alves Racoci (156331)
    Luiz Fernando Fonseca (156475)
*/
#include <stdio.h>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <algorithm>
#include <limits.h>

using namespace std;

// Flags para controlar a interrupcao por tempo
volatile int pare = 0;       // Indica se foi recebido o sinal de interrupcao
volatile int escrevendo = 0; // Indica se estah atualizando a melhor solucao

/* Definicoes de macros que facilitam o desenvolvimento do codigo */
#define INF INT_MAX
#define MAT(T) vector< vector <T> >

// Variaveis para guardar a melhor solucao encontrada
vector<int> melhor_solucao;
int melhor_custo = INF;

/*Variáveis que definem o problema que esta sendo resolvido e guarda as suas informacoes */
int n, // number of scenes
    m; // number of actors

MAT(int) T;         // m x n
vector<int> cost;   // Vetor com os custos das cenas
vector<int> part;   // Vetor com a participacao dos atores

void imprime_saida() {
    // Lembre-se: a primeira linha da saida deve conter n inteiros,
    // tais que o j-esimo inteiro indica o dia de gravacao da cena j!
    for (int j = 0; j < melhor_solucao.size(); j++)
        cout << melhor_solucao[j] << " ";
    // A segunda linha contem o custo (apenas de dias de espera!)
    cout << endl << melhor_custo << endl;
}

void atualiza_solucao(const vector<int> &solucao, int custo) {
    escrevendo = 1;
    for (int i = 0; i < n; i++) {
        melhor_solucao[solucao[i]] = i + 1;
    }
    melhor_custo = custo;
    escrevendo = 0;
    if (pare == 1) {
        // Se estava escrevendo a solucao quando recebeu o sinal,
        // espera terminar a escrita e apenas agora imprime a saida e
        // termina
        imprime_saida();
        exit(0);
    }
}

void interrompe(int signum) {
    pare = 1;
    if (escrevendo == 0) {
        // Se nao estava escrevendo a solucao, pode imprimir e terminar
        imprime_saida();
        exit(0);
    }
}

/* Calcula os s_i's do enunciado */
void compute_part() {
    part.resize(m, 0);
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            part[i] += T[i][j];
        }
    }
}

/* Constantes que podem variar no problema */
#define TAM_POPULACAO 20
#define TAM_MUTACAO 5
#define TAM_CRUZAMENTO 10

/* Classe que representa um individuo, ou solucao */
class Schedule {
public:
    vector<int> scenes;
    int aptidao;

    Schedule() {
        scenes.resize(n);
        for (int i = 0; i < n; i++) {
            scenes[i] = i;
        }
    }

    /* Cria uma permutacao aleatoria */
    void criaSchedule() {
        random_shuffle(scenes.begin(),scenes.end());
    }

    /* Calcula a aptidao do individuo */
    void calculaAptidao() {
        aptidao = 0;
        for (int i = 0; i < m; i++) {
            int aux = 0, part_aux = part[i];
            while (T[i][scenes[aux]] != 1) {
                aux++;
            }
            for (int j = aux; part_aux > 0; j++){
                if (T[i][scenes[j]] == 1) {
                    part_aux -= 1;
                } else {
                    if (part_aux > 0) {
                        aptidao += cost[i];
                    }
                }
            }
        }
    }

    void swap(int index1, int index2) {
        int aux = scenes[index1];
        scenes[index1] = scenes[index2];
        scenes[index2] = aux;
    }

};

/* Classe que representa uma populacao */
class Populacao {
public:
    vector<Schedule> P;

    Populacao() {
        P.resize(TAM_POPULACAO);
        inicializaPopulacao();
    }

    /* Inicializa a populacao com permutacoes aleatorias */
    void inicializaPopulacao() {

        for (int i = 0; i < TAM_POPULACAO; i++) {
            P[i].criaSchedule();
        }

        atualizaMelhor();

    }

    /* Atualiza o melhor individuo */
    void atualizaMelhor() {
        for (int i = 0; i < TAM_POPULACAO; i++) {
            P[i].calculaAptidao();
            if (P[i].aptidao < melhor_custo) {
                atualiza_solucao(P[i].scenes, P[i].aptidao);
            }
        }
    }

};

/* Faz cruzamentos na populacao */
void cruzamentos(Populacao & Pop) {

}

/* Faz mutacao na populacao */
void mutacoes(Populacao & Pop) {
    for (int i = 0; i < TAM_POPULACAO; i++) {
        int index1 = rand() % n;
        int index2 = rand() % n;
        Pop.P[i].swap(index1, index2);
    }
}

/* Funcao que executa o algoritmo genetico */
void solve() {

    /* Seta a semente do algoritmo pseudo-aleatorio */
    srand(unsigned(time(NULL)));
    
    /* Cria a populacao inicial */
    Populacao Pop;
    
    /* Fica recalculando iterativamente o algoritmo ate o tempo acabar */
    while (1) {
        cruzamentos(Pop);
        mutacoes(Pop);
        Pop.atualizaMelhor();
    }

}

/* Funcao que faz a entrada dos dados e os coloca dentro de uma classe problema */
void le_entrada(FILE * inp) {
    int i, j;
    if (not inp) {
        exit(1);
    }
    //cin >> n >> m;
    if (fscanf(inp, "%d\n%d", &n,&m)==EOF) {
        exit(1);
    }
    T.resize(m);
    for (i = 0; i < m; i++) {
        T[i].resize(n);
        for (j = 0; j < n; j++) {
            //cin >> T[i][j];
            if (fscanf(inp, "%d",&T[i][j])==EOF) {
                exit(1);
            }
        }
    }

    cost.resize(m);
    for (i = 0; i < m; i++) {
        //cin >> cost[i];
        if (fscanf(inp, "%d",&cost[i])==EOF) {
            exit(1);
        }
    }

}

int main(int argc, char * argv[]) {
    // Registra a funcao que trata o sinal
    signal(SIGINT, interrompe);

    // Continue sua implementacao aqui. Sempre que encontrar uma nova
    // solucao, utilize a funcao atualiza_solucao para atualizar as
    // variaveis globais.

    if (not (1 < argc)) {
        exit(1);
    }

    FILE * dataFile;
    dataFile = fopen(argv[1], "r");
    if (not dataFile) {
        return 1;
    }

    /* Le os dados do problema */
    le_entrada(dataFile);

    melhor_solucao.resize(n);
    compute_part();

    /* Resolve o problema */
    solve();

    return 0;
}
