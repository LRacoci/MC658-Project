/*
	Nome (RA)
	Lucas Alves Racoci (156331)
	Luiz Fernando Fonseca (156475)
*/
#include <stdio.h>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <limits.h>

#include <vector>
#include <algorithm>
#include <queue>

using namespace std;

//#define DEBUG

// Flags para controlar a interrupcao por tempo
volatile int pare = 0;       // Indica se foi recebido o sinal de interrupcao
volatile int escrevendo = 0; // Indica se estah atualizando a melhor solucao

/* Definicoes de macros que facilitam o desenvolvimento do codigo */
#define INF INT_MAX
#define MAT(T) vector< vector <T> >

// Variaveis para guardar a melhor solucao encontrada
vector<int> melhor_solucao;
int melhor_custo = INF;
int melhor_limitante = 0;
int nos_visitados = 0;

/* Variáveis que definem o problema que esta sendo resolvido e guarda as suas informacoes */

int nScenes, // number of scenes
	nActors; // number of actors

MAT(int) T;         // nActors x nScenes
vector<int> cost;   // Vetor com os custos das cenas
vector<int> part;   // Vetor com a participacao dos atores
vector<int> scenePart;

void imprime_saida() {
	// Lembre-se: a primeira linha da saida deve conter nScenes inteiros,
	// tais que o j-esimo inteiro indica o dia de gravacao da cena j!
	for (int j = 0; j < melhor_solucao.size(); j++)
		cout << melhor_solucao[j] + 1 << " ";
	
		cout << endl;
	// A segunda linha contem o custo (apenas de dias de espera!)
	cout << melhor_custo << endl;
	cout << melhor_limitante << endl;
	cout << nos_visitados << endl;
}

void atualiza_solucao(const vector<int> &solucao, int custo) {
	escrevendo = 1;
	for (int i = 0; i < nScenes; i++) {
        melhor_solucao[solucao[i]] = i;
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
void initializeGlobals();

/* Funcao que executa o branch and bound */
void solve();

/* Calcula os s_i's do enunciado */
void initializeGlobals() {
	part.resize(nActors, 0);
	scenePart.resize(nScenes, 0);
	for (int i = 0; i < nActors; i++) {
		for (int j = 0; j < nScenes; j++) {
			part[i] += T[i][j];
			scenePart[j] += T[i][j];
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

/* Executa a metaheuristica de algoritmo genetico */
void metaheuristica() {
	/* Seta a semente do algoritmo pseudo-aleatorio */
    srand(unsigned(time(NULL)));
    
    /* Cria a populacao inicial */
    Populacao Pop;

    int iteracoes = nScenes * nScenes * nScenes;
    /* Fica recalculando iterativamente o algoritmo ate o tempo acabar */
    for (int i = 0; i < iteracoes; i++) {
        cruzamentos(Pop);
        mutacoes(Pop);
        Pop.atualizaMelhor();
    }

}

// Debug
#ifdef DEBUG
enum Partition {A, B_E, B_D, Unknown};
ostream& operator<<(ostream& os, const Partition& s) {
	switch(s){
		case A: 
			os << "A";
			break;
		case B_E: 
			os << "B_E";
			break;
		case B_D:
			os << "B_D"; 
			break;
		case Unknown: 
			os << "Unknown";
			break;
	}
	return os;

}
#endif // DEBUG

#ifdef DEBUG
template <typename T>
ostream& operator<<(ostream& os, const vector <T>& vec) {  
	for(unsigned i = 0; i < vec.size(); i++){
		os << '\t' << vec[i];
	}
	return os;  
}

// Debug
template <typename T, typename S>
ostream& operator<<(ostream& os, const pair <T,S>& p) {  
	return os << '(' << p.first << ' ' << p.second << ')';  
} 
#endif // DEBUG 


struct descending
{
    template<class T>
    bool operator()(T const &a, T const &b) const { return a > b; }
};

/* Classe que representa uma atribuicao de cenas aos dias */
class Schedule {
  public:
	// Nivel na arvore de busca
	int depth;
	// Indicam os locais de insercao de novas escolhas
	int firstUnknown, lastUnknown;

	// Limitante inferior da solucao
	int boundVal;               
	// Indica se a solucao e completa
	bool complete;
	// day[j] indica o dia que foi gravada a cena j
	vector<int> day;
	// scene[l] indica a cena a ser gravada no dia l
	vector<int> scene;

	int newScene;
	
	#ifdef DEBUG

	// Debug: indica em que conjunto está o ator A(P), B_E(P) ou B_D(P)
	vector<Partition> where;
	// Debug: Initial waiting
	vector<int> initialWait;
	// Debug: Final waiting
	vector<int> finalWait;

	#endif // DEBUG

	//vector<Schedule> offspring; // Cria os filhos do no atual

	/* Cria o objeto inicial (raiz) de Schedule */
	Schedule() {
		depth = 0;
		firstUnknown = 0;
		lastUnknown = nScenes - 1;
		boundVal = 0;
		complete = false;
		day.resize(nScenes, -1);
		scene.resize(nScenes, -1);

		newScene = -1;
		#ifdef DEBUG
	
		where.resize(nActors, Unknown);
		initialWait.resize(nActors, 0);
		finalWait.resize(nActors, 0);

		#endif // DEBUG
	}

	/* Cria objetos subsequentes de Schedule a partir de outro ja criado */
	Schedule(Schedule *dad, int chosen_scene) : 
		#ifdef DEBUG

		where(dad->where),
		initialWait(nActors, 0),
		finalWait(nActors, 0),

		#endif // DEBUG

		day(dad->day),
		scene(dad->scene)
	{
		/* Incrementa o nivel do no atual em relacao ao seu superior */
		depth = dad->depth + 1;
		newScene = chosen_scene;

		complete = depth == nScenes;
		
		/* Checa o lado do vetor que a nova escolha sera inserida,
		* atualiza os vetores que indicam o primeiro e ultimo dia
		* de cada ator e atualiza os indices direito e esquerdo */
		int chosen_day;
		if (depth % 2 == 1) {
			lastUnknown = dad->lastUnknown;
			firstUnknown = dad->firstUnknown + 1;
			chosen_day = firstUnknown - 1;
		} else {
			firstUnknown = dad->firstUnknown;
			lastUnknown = dad->lastUnknown - 1;
			chosen_day = lastUnknown + 1;
		}

		day[chosen_scene] = chosen_day;
		scene[chosen_day] = chosen_scene;
	}

	/* Retorna os filhos do no corrente */
	void branch(vector<Schedule> &offspring) {

		for (int j = 0; j < nScenes; j++) {
			if (day[j] == -1){
				Schedule child(this, j);
				if(depth != 2 or child.scene[0] < child.scene[nScenes-1]){
					offspring.push_back(child);
				}
			}
		}

		//return offspring;
	}

	friend ostream& operator<<(ostream& os, const Schedule& s);

	vector< int > packCost(vector< pair<int, int > > & actorSet){
		// [(scenePart, scene)]
		vector< pair<int, int > > scenesOrder;
		// packSet[j] indica cenas com atores soh em uma delas
		// packSets = {packSet : para todo a em atores : 1 = sum {j em packSet} T[a,j]}
		// |packSet| <= (lastUnknown - firstUnknown + 1 - max{part[i] - nKnownDays[i]| i in atores} + 1)
		vector<bool> packSet(nScenes, false);
		// Cenas ainda não decididas.
		for (int j = 0; j < nScenes; j++){
			if (day[j] == -1){
				scenesOrder.push_back(make_pair(scenePart[j],j));
				packSet[j] = true;
			}
		}
		// sort scenesOrder descending
		stable_sort(scenesOrder.begin(), scenesOrder.end());

		#ifdef DEBUG

		clog << endl << "packCost function \t";
		clog << "sceneOrder: " << scenesOrder << endl << "packCost function \t";
		
		clog << "Cenas em ordem:" << endl << "packCost function \t";
		// Imprime primeira linha
		clog << "actorSet";
		for(int j = 0; j < scenesOrder.size(); j++){
			int sc = scenesOrder[j].second;
			clog << " \t Cena " << sc;
		}
		clog << endl << "packCost function \t";
		// Imprime outras linhas
		for (int i = 0; i < actorSet.size(); i++){
			int a = actorSet[i].second;
			clog << a;
			for(int j = 0; j < scenesOrder.size(); j++){
				int sc = scenesOrder[j].second;
				clog << '\t';
				if (T[a][sc]){
					clog << 1;
				}
			}
			clog << endl << "packCost function \t";
		}

		#endif // DEBUG


		for (int i = 0; i < actorSet.size(); i++){
			int a = actorSet[i].second;
			int scenesVisited = 0;
			for(int j = 0; j < scenesOrder.size(); j++){
				int sc = scenesOrder[j].second;
				if(packSet[sc]){
					scenesVisited += T[a][sc];
					if (scenesVisited > 1){
						packSet[sc] = false;
						#ifdef DEBUG
						clog << "\t("<< a << " tira " << sc << ")";
						#endif // DEBUG
					}
				}
			}
			#ifdef DEBUG
			clog << endl << "packCost function \t";
			#endif // DEBUG
		}
		#ifdef DEBUG
		clog << "Empacotamento:" << endl << "packCost function \t";
		// Imprime primeira linha
		clog << "actorSet";
		for(int j = 0; j < scenesOrder.size(); j++){
			int sc = scenesOrder[j].second;
			if( packSet[sc]){
				clog << "\tCena " << sc;
			}
		} 
		clog << endl << "packCost function \t";
		// Imprime outras linhas
		for (int i = 0; i < actorSet.size(); i++){
			int a = actorSet[i].second;
			clog << a;
			for(int j = 0; j < scenesOrder.size(); j++){
				int sc = scenesOrder[j].second;
				if( packSet[sc]){
					clog << '\t';
					if (T[a][sc]){
						clog << 1;
					}
				}
			}
			clog << endl << "packCost function \t";
		}
		#endif // DEBUG
		
		// [sceneCost]
		vector< int > packCost;
		for(int j = 0; j < scenesOrder.size(); j++){
			int sc = scenesOrder[j].second;
			if (packSet[sc]){
				int sceneCost = 0;
				for(int i = 0; i < actorSet.size(); i++){
					int a = actorSet[i].second;
					sceneCost += cost[a] * T[a][sc];
				}
				packCost.push_back(sceneCost);
			}
		}
		#ifdef DEBUG
		clog << "packCost before sort: \t " << packCost << endl << "packCost function \t";
		#endif // DEBUG

		stable_sort(packCost.begin(), packCost.end(), descending());

		return packCost;
	}

	/* Calcula o limitante do no */
	void bound() {
		int chosen_day;
		// Indica o primeiro dia de cada ator
		vector<int> first(nActors, nScenes);
		// Indica o ultimo dia de cada ator
		vector<int> last(nActors, -1);
		// Indica numero de dias conhecidos por ator
		vector<int> nKnownDays(nActors, 0);
		// Ultima cena escolhida

		for (int i = 0; i < nActors; i++) {
			for (int l = 0; l < nScenes; l++){
				if(scene[l] != -1 and T[i][scene[l]]){
					nKnownDays[i]++;
					if (l < first[i]) first[i] = l;
					if (l >  last[i])  last[i] = l;
				}
			}
			/*
			for (int l = firstUnknown - 1; l >= 0; l--){
				if (T[i][scene[l]]){
					nKnownDays[i]++;
					if (l < first[i]){
						first[i] = l;
					}
				}
			}
			for (int l = lastUnknown + 1; l < nScenes; l++){
				if (T[i][scene[l]]){
					nKnownDays[i]++;
					if (l > last[i]){
						last[i] = l;
					}
				}
			}
			*/
		}

		int k1 = 0;
		// Calcula k1
		for (int i = 0; i < nActors; i++) {
			// if (first[i] < nScenes and last[i] >= 0) {
			/*
				PS: O novo if é mais restrito e acho que não precisa mais testar se aux > 0
				mas eu deixei assim por enquanto
			*/
			// i in A(P) <==> first[i] < firstUnknown and last[i] >= lastUnknown
			if ((first[i] < firstUnknown and last[i] >= lastUnknown) or nKnownDays[i] == part[i]){
				#ifdef DEBUG
				where[i] = A;
				#endif // DEBUG
				int aux = cost[i] * (last[i] - first[i] + 1 - part[i]);
				if (aux > 0) {
					k1 += aux;
				}
			}
		}
		int k2 = 0;
		// Calcula k2

		for (int i = 0; i < nActors; i++){
			// i in B_E(P) <==> first[i] < firstUnknown and not in A(P)
			if (first[i] < firstUnknown and last[i] < lastUnknown and nKnownDays[i] < part[i]){
				int sum = 0;
				#ifdef DEBUG
				where[i] = B_E;
				#endif // DEBUG
				// Soma 0's de first[i] até firstUnknown
				for (int l = first[i]; l < firstUnknown; l++){
					sum += 1 - T[i][scene[l]];
				}
				#ifdef DEBUG
				initialWait[i] = sum;
				#endif // DEBUG
				k2 += cost[i] * sum;
			}
		}

		for (int i = 0; i < nActors; i++){
			// i in B_D(P) <==> last[i] >= lastUnknown and not in A(P)
			if (last[i] >= lastUnknown and first[i] >= firstUnknown and nKnownDays[i] < part[i]){
				int sum = 0;
				#ifdef DEBUG
				where[i] = B_D;
				#endif // DEBUG
				for (int l = last[i]; l > lastUnknown; l--){
					sum += 1 - T[i][scene[l]];
				}
				#ifdef DEBUG
				finalWait[i] = sum;
				#endif // DEBUG
				k2 += cost[i] * sum;
			}

		}

		// Calcula k3

		// Encontra subconjunto de cenas ainda não decididas com 1 ator de B_E em cada

		// [(numero de cenas desconhecidas de que o ator nao participa, ator)]
		vector< pair<int, int > > vecB_E;
		for (int i = 0; i < nActors; i++){
			// if i in B_E
			if (first[i] < firstUnknown and last[i] < lastUnknown and nKnownDays[i] < part[i]){
				vecB_E.push_back(make_pair(lastUnknown+1-firstUnknown-(part[i]-nKnownDays[i]),i));
			}
		}
		// sort vecB_E descending
		stable_sort(vecB_E.begin(), vecB_E.end(), descending());

		#ifdef DEBUG
		clog << "vecB_E: " << vecB_E << endl;
		#endif // DEBUG

		vector<int> packCostB_E = packCost(vecB_E);

		#ifdef DEBUG
		clog << "packCostB_E: \t " << packCostB_E << endl;
		#endif // DEBUG

		//Calcula k3
		int k3 = 0;
		for(int j = 0; j < packCostB_E.size(); j++){
			k3 += j * packCostB_E[j];
		}
		#ifdef DEBUG
		if (k3){
			clog << "k3 \t" << k3 << endl;
		}
		#endif // DEBUG

		//Calcula k4

		// [(numero de cenas desconhecidas de que o ator nao participa, ator)]
		vector< pair<int, int > > vecB_D;
		for (int i = 0; i < nActors; i++){
			// if i in B_D
			if (last[i] >= lastUnknown and first[i] >= firstUnknown and nKnownDays[i] < part[i]){
				vecB_D.push_back(make_pair(lastUnknown+1-firstUnknown-(part[i]-nKnownDays[i]),i));
			}
		}
		// sort vecB_D descending
		stable_sort(vecB_D.begin(), vecB_D.end(), descending());
		
		#ifdef DEBUG
		clog << "vecB_D: " << vecB_D << endl;
		#endif // DEBUG

		vector<int> packCostB_D = packCost(vecB_D);
		#ifdef DEBUG
		clog << "packCostB_D: \t " << packCostB_D << endl;
		#endif // DEBUG


		int k4 = 0;

		for(int j = 0; j < packCostB_D.size(); j++){
			k4 += j * packCostB_D[j];
		}
		#ifdef DEBUG
		if (k4){
			clog << "k4 \t" << k4 << endl;
		}
		#endif // DEBUG

		#ifdef DEBUG
		clog << "boundVal \t " << '=' << k1 << '+' << k2 << '+' << k3 << '+' << k4 << endl;
		#endif // DEBUG
		boundVal = k1 + k2 + k3 + k4;
	}

	void updateBest() {
		atualiza_solucao(scene, boundVal);
	}

};

// Debuging
#ifdef DEBUG 
ostream& operator<<(ostream& os, const Schedule& s) {
	/*
		Actors 	 first 	Cenas 	last where
	*/
	os << "depth \t " << s.depth << endl;
	os << "Actors \t where \t first \t initialWait";
	for (int l = 0; l < nScenes; l ++) {
		os << '\t';
		if (s.scene[l] != -1){
			os << s.scene[l];
		}
	}
	os << "\t finalWait \t last \t part \t cost";
	os << endl;
	for (int a = 0; a < nActors; a++) {
		os << a ;
		os << '\t' << s.where[a];
		os << '\t' << s.first[a];
		os << '\t' << s.initialWait[a];
		for (int l = 0; l < nScenes; l++) {
			os << '\t';
			if (s.scene[l] != -1){
				if(T[a][s.scene[l]]){
					os << 1;
				}
			}else{
				os << '?';
			}
		}
		os << '\t' << s.finalWait[a]; 
		os << '\t' << s.last[a];
		os << '\t' << part[a];
		os << '\t' << cost[a]; 
		os << endl;
	}
	if (s.boundVal > 0){
		os << "limitante \t" << s.boundVal << endl;
	}
	os << "firstUnknown \t " << s.firstUnknown << endl;
	os << "lastUnknown \t " << s.lastUnknown << endl;

	return os;
}
#endif // DEBUG 

/* Funcao de comparacao para se obter um heap de minimo */
struct compare {
    bool operator()(const Schedule &l, const Schedule &r) {
        return l.boundVal > r.boundVal or (l.boundVal == r.boundVal and l.depth < r.depth);
    }
};

/* Fila de prioridade que contera os nos ativos */
priority_queue<Schedule, vector<Schedule>, compare> ativos;

/* Funcao que executa o branch and bound */
void solve() {
	Schedule root;
	
	ativos.push(root);

	/* Explora os nos ativos */
	while (not ativos.empty()) {
		/* Retira o melhor no da fila */
		Schedule topAtivo = ativos.top();
		ativos.pop();
		
		/* Checa se o no atual pode dar uma solucao melhor
		 * do que a melhor encontrada ate o momento */
		if (topAtivo.boundVal < melhor_custo) {
			melhor_limitante = topAtivo.boundVal;
			/* Gera os filhos do no ativo, calcula os limitantes e continua o processo */
			nos_visitados++;
			vector<Schedule> offspring;
			topAtivo.branch(offspring);
			/* Trata cada filho separadamente */
			for (int j = 0; j < offspring.size(); j++) {
				offspring[j].bound();
				if (offspring[j].boundVal < melhor_custo) {
					if (offspring[j].complete) {
						offspring[j].updateBest();
					} else {
						ativos.push(offspring[j]);
					}
				}
			}
		}
	}
	melhor_limitante = melhor_custo;
}

/* Funcao que faz a entrada dos dados e os coloca dentro de uma classe problema */
void le_entrada(FILE * dataFile) {
	int i, j;
	if (not dataFile) {
		exit(1);
	}
	//cin >> nScenes >> nActors;
	if (fscanf(dataFile, "%d\n%d", &nScenes,&nActors)==EOF) {
		exit(1);
	}
	T.resize(nActors);
	for (i = 0; i < nActors; i++) {
		T[i].resize(nScenes);
		for (j = 0; j < nScenes; j++) {
			//cin >> T[i][j];
			if (fscanf(dataFile, "%d",&T[i][j])==EOF) {
				exit(1);
			}
		}
	}

	cost.resize(nActors);
	for (i = 0; i < nActors; i++) {
		//cin >> cost[i];
		if (fscanf(dataFile, "%d",&cost[i])==EOF) {
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
	fclose(dataFile);

	//s_i's do enunciado
	initializeGlobals();
	melhor_solucao.resize(nScenes);

	/* Roda a metaheuristica de algoritmo genetico antes
	 * de executar o codigo do branch and bound */
	metaheuristica();

	/* Resolve o problema */
	solve();

	/* Imprime os resultados */
	imprime_saida();

	return 0;
}
