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
int nScenes, // number of scenes
    nActors; // number of actors

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
    for (int i = 0; i < nScenes; i++) {
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
    part.resize(nActors, 0);
    for (int i = 0; i < nActors; i++) {
        for (int j = 0; j < nScenes; j++) {
            part[i] += T[i][j];
        }
    }
}

/* Constantes que podem variar no problema */
#define TAM_POPULACAO 10
#define TAM_MUTACAO 20
#define TAM_CRUZAMENTO 10

/* Classe que representa um individuo, ou solucao */
class Permutation {
public:
    vector<int> scenes;
    int aptidao;

    Permutation() {
        scenes.resize(nScenes);
        for (int i = 0; i < nScenes; i++) {
            scenes[i] = i;
        }
        aptidao = INF;
    }

    /* Cria uma permutacao aleatoria */
    void criaPermutation() {
        random_shuffle(scenes.begin(),scenes.end());
    }

    /* Calcula a aptidao do individuo */
    void calculaAptidao() {
        aptidao = 0;
        for (int i = 0; i < nActors; i++) {
            int aux = 0, part_aux = part[i];
            if (part_aux > 0) {
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
    }

    /* Troca dois elementos da permutacao */
    void troca(int index1, int index2) {
        int aux = scenes[index1];
        scenes[index1] = scenes[index2];
        scenes[index2] = aux;
    }

};

/* Classe que representa uma populacao */
class Populacao {
public:
    vector<Permutation> P;
    int tamanho_real;

    Populacao() {
        P.resize(TAM_POPULACAO + TAM_CRUZAMENTO + TAM_MUTACAO);
        tamanho_real = 0;
        inicializaPopulacao();
    }

    /* Inicializa a populacao com permutacoes aleatorias */
    void inicializaPopulacao() {
        /* Gera individuos diferentes entre si */
        for (int i = 0; i < TAM_POPULACAO; i++) {
            tamanho_real++;
            do {
                P[i].criaPermutation();
            } while (existeDuplicata(i));
        }

        /* Calcula a aptidao */
        for (int i = 0; i < TAM_POPULACAO; i++) {
            P[i].calculaAptidao();
        }

    }

    /* Checa se a permutacao i esta repetida */
    bool existeDuplicata(int i) {
        int j, k;
        bool igual;

        for (j = 0; j < tamanho_real; j++) {
            if (i != j) {
                igual = true;
                for (k = 0; k < nScenes; k++) {
                    if (P[i].scenes[k] != P[j].scenes[k]) {
                        igual = false;
                        break;
                    }
                }
                if (igual) {
                    return true;
                }
            }
        }

        return false;
    }

    /* Atualiza o melhor individuo */
    void atualizaMelhor() {
        vector<Permutation> aux(TAM_POPULACAO);
        int index;

        /* Calcula a aptidao dos novos individuos */
        for (int i = TAM_POPULACAO; i < TAM_POPULACAO + TAM_CRUZAMENTO + TAM_MUTACAO; i++) {
            P[i].calculaAptidao();
        }

        /* Procura o menor individuo */
        int min = 0;
        for (int i = 1; i < TAM_POPULACAO + TAM_CRUZAMENTO + TAM_MUTACAO; i++) {
            if (P[i].aptidao <= P[min].aptidao) {
                min = i;
            }
        }
        aux[0] = P[min];

        /* Sorteia o resto dos individuos da populacao */
        for (int i = 1; i < TAM_POPULACAO; i++) {
            index = (rand() % (TAM_POPULACAO + TAM_MUTACAO + TAM_CRUZAMENTO - 1)) + 1;
            aux[i] = P[index];
        }

        for (int i = 0; i < TAM_POPULACAO; i++) {
            P[i] = aux[i];
        }

        /* Atualiza o melhor custo e a melhor solucao */
        if (P[0].aptidao < melhor_custo) {
            atualiza_solucao(P[0].scenes, P[0].aptidao);
        }
    }

};

/* Variaveis que selecionam o metodo de cruzamento ou
 * mutacao utilizando um esquema de round-robin */
int metodo_cruzamento = 0;
int metodo_mutacao = 0;

/* Faz cruzamentos na populacao */
void cruzamentos(Populacao & Pop) {
    int pai1, pai2, index1, index2, aux, j, k;
    vector<bool> escolhidos;
    escolhidos.resize(nScenes);

    for (int i = TAM_POPULACAO + TAM_MUTACAO; i < TAM_POPULACAO + TAM_MUTACAO + TAM_CRUZAMENTO; i++) {
        /* Escolhe dois pais para o cruzamento */
        do {
            pai1 = rand() % TAM_POPULACAO;
            pai2 = rand() % TAM_POPULACAO;
        } while (pai1 == pai2);
        fill(escolhidos.begin(), escolhidos.end(), false);

        /* Escolhe entre os cruzamentos: pega a parte inicial de um dos pais
         * e depois copia os elementos faltantes do outro pai na ordem em que
         * aparecem, ou faz o mesmo processo escolhendo uma regiao interior
         * do primeiro pai e completa com a ordem dos elementos do outro pai */
        if (metodo_cruzamento % 2 == 0) {
            index1 = rand() % nScenes;
            j = 0;
            while (j <= index1) {
                aux = Pop.P[pai1].scenes[j];
                escolhidos[aux] = true;
                Pop.P[i].scenes[j] = aux;
                j++;
            }
            for (k = 0; k < nScenes; k++) {
                aux = Pop.P[pai2].scenes[k];
                if (!escolhidos[aux]) {
                    escolhidos[aux] = true;
                    Pop.P[i].scenes[j] = aux;
                    j++;
                }
            }
        } else {
            do {
                index1 = rand() % nScenes;
                index2 = rand() % nScenes;
            } while (index1 == index2);
            if (index1 > index2) {
                aux = index1;
                index1 = index2;
                index2 = aux;
            }
            j = index1;
            while (j <= index2) {
                aux = Pop.P[pai1].scenes[j];
                escolhidos[aux] = true;
                Pop.P[i].scenes[j] = aux;
                j++;
            }
            j = 0;
            k = 0;
            while (k < index1) {
                aux = Pop.P[pai2].scenes[j];
                if (!escolhidos[aux]) {
                    escolhidos[aux] = true;
                    Pop.P[i].scenes[k] = aux;
                    k++;
                }
                j++;
            }
            k = index2 + 1;
            while (k < nScenes) {
                aux = Pop.P[pai2].scenes[j];
                if (!escolhidos[aux]) {
                    escolhidos[aux] = true;
                    Pop.P[i].scenes[k] = aux;
                    k++;
                }
                j++;
            }
        }
        metodo_cruzamento = (metodo_cruzamento + 1) % 2;
    }
}

/* Faz mutacao na populacao atraves de tres metodos diferentes */
void mutacoes(Populacao & Pop) {
    int index1, index2, individuo, aux;

    for (int i = TAM_POPULACAO; i < TAM_POPULACAO + TAM_MUTACAO; i++) {
        /* Escolhe dois indices distintos e um individuo da populacao */
        do {
            index1 = rand() % nScenes;
            index2 = rand() % nScenes;
        } while (index1 == index2);
        individuo = rand() % TAM_POPULACAO;
        Pop.P[i] = Pop.P[individuo];

        /* Escolhe entre as mutacoes: troca dois elementos da permutacao,
         * traz um segundo elemento para ao lado do primeiro no vetor, ou
         * seleciona um trecho e inverte os valores da permutacao */
        if (metodo_mutacao % 3 == 0) {
            Pop.P[i].troca(index1, index2);
        } else if (metodo_mutacao % 3 == 1) {
            if (index1 > index2) {
                aux = index1;
                index1 = index2;
                index2 = aux;
            }
            aux = Pop.P[i].scenes[index2];
            while (index2 > index1 + 1) {
                Pop.P[i].scenes[index2] = Pop.P[i].scenes[index2 - 1];
                index2--;
            }
            Pop.P[i].scenes[index2] = aux;
        } else {
            if (index1 > index2) {
                aux = index1;
                index1 = index2;
                index2 = aux;
            }
            /* Exclui as bordas para fazer o inverso, pois
             * inverter toda a permutacao mantem o custo */
            if (index1 == 0 and index2 == nScenes - 1) {
                index1++;
                index2--;
            }
            while (index1 < index2) {
                Pop.P[i].troca(index1, index2);
                index1++;
                index2--;
            }
        }
        metodo_mutacao = (metodo_mutacao + 1) % 3;
    }
}

/* Funcao que executa o algoritmo genetico */
void solve() {

    /* Seta a semente do algoritmo pseudo-aleatorio */
    srand(unsigned(time(NULL)));
    
    /* Utiliza o algoritmo genetico varias vezes */
    while (1) {
        /* Cria a populacao inicial */
        Populacao Pop;

        /* Fica recalculando iterativamente o algoritmo ate o tempo acabar */
        for (int i = 0; i < 10000; i++) {
            cruzamentos(Pop);
            mutacoes(Pop);
            Pop.atualizaMelhor();
        }
    }

}

/* Funcao que faz a entrada dos dados e os coloca dentro de uma classe problema */
void le_entrada(FILE * inp) {
    int i, j;
    if (not inp) {
        exit(1);
    }
    //cin >> nScenes >> nActors;
    if (fscanf(inp, "%d\n%d", &nScenes,&nActors)==EOF) {
        exit(1);
    }
    T.resize(nActors);
    for (i = 0; i < nActors; i++) {
        T[i].resize(nScenes);
        for (j = 0; j < nScenes; j++) {
            //cin >> T[i][j];
            if (fscanf(inp, "%d",&T[i][j])==EOF) {
                exit(1);
            }
        }
    }

    cost.resize(nActors);
    for (i = 0; i < nActors; i++) {
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

    melhor_solucao.resize(nScenes);
    compute_part();

    /* Resolve o problema */
    solve();

    return 0;
}
