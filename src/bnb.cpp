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
#include <limits.h>
#include <queue>

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
int nos_visitados = 0;

void imprime_saida() {
    // Lembre-se: a primeira linha da saida deve conter n inteiros,
    // tais que o j-esimo inteiro indica o dia de gravacao da cena j!
    for (int j = 0; j < melhor_solucao.size(); j++)
        cout << melhor_solucao[j] << " ";
    
        cout << endl;
    // A segunda linha contem o custo (apenas de dias de espera!)
    cout << melhor_custo << endl;
    cout << melhor_custo << endl;
    cout << nos_visitados << endl;
}

void atualiza_solucao(const vector<int> &solucao, int custo) {
    escrevendo = 1;
    melhor_solucao = solucao;
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
/*Variáveis que definem o problema que esta sendo resolvido e guarda as suas informacoes */

int n, // number of scenes
    m; // number of actors

MAT(int) T;         // m x n
vector<int> cost;   // Vetor com os custos das cenas
vector<int> part;   // Vetor com a participacao dos atores

/* Calcula os s_i's do enunciado */
void compute_part();

/* Funcao que executa o branch and bound */
void solve();

/* Calcula os s_i's do enunciado */
void compute_part() {
    part.resize(m, 0);
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            part[i] += T[i][j];
        }
    }
}

/* Classe que representa uma atribuicao de cenas aos dias */
class Schedule {
  public:
    int depth;                  // Nivel na arvore de busca
    int l, r;                   // Indicam os locais de insercao de novas escolhas
    int boundVal;               // Limitante inferior da solucao
    bool complete;              // Indica se a solucao e completa
    vector<int> days;           // Permutacao das cenas
    vector<int> chosen;         // Indica quais cenas ja foram escolhidas
    vector<int> first;          // Indica o primeiro dia de cada ator
    vector<int> last;           // Indica o ultimo dia de cada ator
    vector<Schedule> offspring; // Cria os filhos do no atual

    /* Cria o objeto inicial (raiz) de Schedule */
    Schedule() {
        depth = 0;
        l = 0;
        r = n - 1;
        boundVal = 0;
        complete = false;
        days.resize(n, -1);
        chosen.resize(n, false);
        first.resize(m, n);
        last.resize(m, -1);
    }

    /* Cria objetos subsequentes de Schedule a partir de outro ja criado */
    Schedule(Schedule *dad, int choice) : 
        days(dad->days),
        chosen(dad->chosen),
        first(dad->first),
        last(dad->last) {
        /* Incrementa o nivel do no atual em relacao ao seu superior */
        depth = dad->depth + 1;
        complete = depth == n;
        chosen[choice] = true;

        /* Checa o lado do vetor que a nova escolha sera inserida,
		* atualiza os vetores que indicam o primeiro e ultimo dia
		* de cada ator e atualiza os indices direito e esquerdo */
        int chosen_day;
        if (depth % 2 == 1) {
            r = dad->r;
            l = dad->l + 1;
            chosen_day = l - 1;
        } else {
            l = dad->l;
            r = dad->r - 1;
            chosen_day = r + 1;
        }

        days[chosen_day] = choice;
        for (int i = 0; i < m; i++) {
            if (T[i][choice]) {
                if (chosen_day < first[i]) {
                    first[i] = chosen_day;
                }
                if (chosen_day > last[i]) {
                    last[i] = chosen_day;
                }
            }
        }
    }

    /* Retorna os filhos do no corrente */
    vector<Schedule> &branch() {

        for (int j = 0; j < n; j++) {
            if (not chosen[j]){
                Schedule child(this, j);
                offspring.push_back(child);
            }
        }

        return offspring;
    }

    /* Calcula o limitante do no */
    void bound() {
        int val = 0;

        for (int i = 0; i < m; i++) {
            // if i in A(P) <==> di != none != ei
            if (first[i] <= l and last[i] >= r) {
                val += cost[i] * (last[i] - first[i] + 1 - part[i]);
            }
        }

        boundVal = val;
    }

    void updateBest() {
        vector<int> scenes;
        int tam = days.size();
        scenes.resize(tam);
        for (int j = 0; j < tam; j++) {
            scenes[days[j]] = j + 1;
        }
        atualiza_solucao(scenes, boundVal);
    }
};

/* Funcao de comparacao para se obter um heap de minimo */
struct compare {
    bool operator()(const Schedule &l, const Schedule &r) {
        return l.boundVal > r.boundVal;
    }
};


/* Funcao que executa o branch and bound */
void solve() {
    //s_i's do enunciado
    compute_part();

    /* Fila de prioridade que contera os nos ativos */
    Schedule root;
    priority_queue<Schedule, vector<Schedule>, compare> pq;
    pq.push(root);

    /* Explora os nos ativos */
    while (not pq.empty()) {
        /* Retira o melhor no da fila */
        Schedule k = pq.top();
        pq.pop();

        /* Checa se o no atual pode dar uma solucao melhor
		 * do que a melhor encontrada ate o momento */
        if (k.boundVal < melhor_custo) {
        	nos_visitados++;
            /* Gera os filhos do no ativo, calcula os limitantes e continua o processo */
            vector<Schedule> &offspring = k.branch();
            /* Trata cada filho separadamente */
            for (int j = 0; j < offspring.size(); j++) {
                offspring[j].bound();
                if (offspring[j].boundVal < melhor_custo) {
                    if (offspring[j].complete) {
                        //bestCost = offspring[j].boundVal;
                        //best = offspring[j];
                        offspring[j].updateBest();
                        cout << offspring[j].l << " " << offspring[j].r << endl;
                    } else {
                        pq.push(offspring[j]);
                    }
                }
            }
        }
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

    FILE * inp;
    inp = fopen(argv[1], "r");
    if (not inp) {
        return 1;
    }

    /* Le os dados do problema */
    le_entrada(inp);

    /* Resolve o problema */
    solve();

    /* Imprime os resultados */
    imprime_saida();

    return 0;
}
