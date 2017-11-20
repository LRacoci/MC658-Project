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

int nScenes, // number of scenes
	nActors; // number of actors

MAT(int) T;         // nActors x nScenes
vector<int> cost;   // Vetor com os custos das cenas
vector<int> part;   // Vetor com a participacao dos atores

/* Calcula os s_i's do enunciado */
void initializeGlobals();

/* Funcao que executa o branch and bound */
void solve();

vector<int> sceneCost;
vector<int> scenePart;

/* Calcula os s_i's do enunciado */
void initializeGlobals() {
	part.resize(nActors, 0);
	sceneCost.resize(nScenes, 0);
	scenePart.resize(nScenes, 0);
	for (int i = 0; i < nActors; i++) {
		for (int j = 0; j < nScenes; j++) {
			sceneCost[j] += T[i][j] * cost[i];
			
			part[i] += T[i][j];
			scenePart[j] += T[i][j];
		}
	}
}
// Debug
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
// Debug
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


/* Classe que representa uma atribuicao de cenas aos dias */
class Schedule {
  public:
	// Nivel na arvore de busca
	int depth;
	// Indicam os locais de insercao de novas escolhas
	int lastInitial, firstFinal;
	// Limitante inferior da solucao
	int boundVal;               
	// Indica se a solucao e completa
	bool complete;
	// day[j] indica o dia que foi gravada a cena j
	vector<int> day;
	// scene[l] indica a cena a ser gravada no dia l
	vector<int> scene;
	// Indica o primeiro dia de cada ator
	vector<int> first;
	// Indica o ultimo dia de cada ator
	vector<int> last;
	// Indica numero de dias conhecidos por ator
	vector<int> nKnownDays;
	// Debug: indica em que conjunto está o ator A(P), B_E(P) ou B_D(P)
	vector<Partition> where;
	// Debug: Initial waiting
	vector<int> initialWait;
	// Debug: Final waiting
	vector<int> finalWait;


	vector<Schedule> offspring; // Cria os filhos do no atual

	/* Cria o objeto inicial (raiz) de Schedule */
	Schedule() {
		depth = 0;
		lastInitial = 0;
		firstFinal = nScenes - 1;
		boundVal = 0;
		complete = false;
		day.resize(nScenes, -1);
		scene.resize(nScenes, -1);
		first.resize(nActors, nScenes);
		last.resize(nActors, -1);
		nKnownDays.resize(nActors, 0);
		where.resize(nActors, Unknown);
		initialWait.resize(nActors, 0);
		finalWait.resize(nActors, 0);
	}

	/* Cria objetos subsequentes de Schedule a partir de outro ja criado */
	Schedule(Schedule *dad, int chosen_scene) : 
		day(dad->day),
		scene(dad->scene),
		first(dad->first),
		last(dad->last),
		nKnownDays(dad->nKnownDays),
		where(dad->where),
		initialWait(nActors, 0),
		finalWait(nActors, 0)
	{
		/* Incrementa o nivel do no atual em relacao ao seu superior */
		depth = dad->depth + 1;
		complete = depth == nScenes;

		/* Checa o lado do vetor que a nova escolha sera inserida,
		* atualiza os vetores que indicam o primeiro e ultimo dia
		* de cada ator e atualiza os indices direito e esquerdo */
		int chosen_day;
		if (depth % 2 == 1) {
			firstFinal = dad->firstFinal;
			lastInitial = dad->lastInitial + 1;
			chosen_day = lastInitial - 1;
		} else {
			lastInitial = dad->lastInitial;
			firstFinal = dad->firstFinal - 1;
			chosen_day = firstFinal + 1;
		}

		day[chosen_scene] = chosen_day;
		scene[chosen_day] = chosen_scene;

		for (int i = 0; i < nActors; i++) {
			if (T[i][chosen_scene]) {
				nKnownDays[i]++;
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

		for (int j = 0; j < nScenes; j++) {
			if (day[j] == -1){
				Schedule child(this, j);
				offspring.push_back(child);
			}
		}

		return offspring;
	}

	friend ostream& operator<<(ostream& os, const Schedule& s);

	/* Calcula o limitante do no */
	void bound() {
		int k1 = 0;
		// Calcula k1
		for (int i = 0; i < nActors; i++) {
			// if (first[i] < nScenes and last[i] >= 0) {
			/*
				PS: O novo if é mais restrito e acho que não precisa mais testar se aux > 0
				mas eu deixei assim por enquanto
			*/
			// i in A(P) <==> first[i] < lastInitial and last[i] >= firstFinal
			if ((first[i] < lastInitial and last[i] >= firstFinal) or nKnownDays[i] == part[i]){
				where[i] = A;
				int aux = cost[i] * (last[i] - first[i] + 1 - part[i]);
				if (aux > 0) {
					k1 += aux;
				}
			}
		}
		int k2 = 0;
		// Calcula k2

		for (int i = 0; i < nActors; i++){
			// i in B_E(P) <==> first[i] < lastInitial and not in A(P)
			if (first[i] < lastInitial and last[i] < firstFinal and nKnownDays[i] < part[i]){
				int sum = 0;
				where[i] = B_E;
				// Soma 0's de first[i] até lastInitial
				for (int l = first[i]; l < lastInitial; l++){
					sum += 1 - T[i][scene[l]];
				}
				initialWait[i] = sum;
				k2 += cost[i] * sum;
			}
		}

		for (int i = 0; i < nActors; i++){
			// i in B_D(P) <==> last[i] >= firstFinal and not in A(P)
			if (last[i] >= firstFinal and first[i] >= lastInitial and nKnownDays[i] < part[i]){
				int sum = 0;
				where[i] = B_D;
				for (int l = last[i]; l > firstFinal; l--){
					sum += 1 - T[i][scene[l]];
				}
				finalWait[i] = sum;
				k2 += cost[i] * sum;
			}

		}

		// Encontra subconjunto de cenas ainda não decididas com 1 ator em cada

		int k3 = 0;
		// empac[j] indica cenas com atores soh em uma delas
		// empacs = {empac : para todo a em atores : 1 = sum {j em empac} T[a,j]}
		// |empac| <= (firstFinal - lastInitial + 1 - max{part[i] - nKnownDays[i]| i in atores} + 1)
		
		// [(numero de cenas desconhecidas de que o ator nao participa, ator)]
		vector< pair<int, int > > actorsOrder;
		for (int i = 0; i < nActors; i++){
			// if i in B_E
			if (first[i] < lastInitial and last[i] < firstFinal and nKnownDays[i] < part[i]){
				actorsOrder.push_back(make_pair(firstFinal+1-lastInitial-(part[i]-nKnownDays[i]),i));
			}
		}
		// sort actorsOrder decrescente
		stable_sort(actorsOrder.begin(), actorsOrder.end());

		clog << "actorsOrder: " << actorsOrder << endl;

		// [(sceneCost, scene)]
		vector< pair<int, int > > scenesOrder;
		// Cenas ainda não decididas.
		for (int j = 0; j <= nScenes; j++){
			if (day[j] == -1){
				scenesOrder.push_back(make_pair(scenePart[j],j));
			}
		}
		stable_sort(scenesOrder.begin(), scenesOrder.end());
		clog << "sceneOrder: " << scenesOrder << endl;
		// sort scenesOrder decrescente

		/*
		clog << "Cenas em ordem:" << endl;
		// Imprime primeira linha
		clog << "B_E";
		for(int j = 0; j < scenesOrder.size(); j++){
			int sc = scenesOrder[j].second;
			clog << " \t Cena " << sc;
		}
		clog << endl;
		// Imprime outras linhas
		for (int i = 0; i < actorsOrder.size(); i++){
			int a = actorsOrder[i].second;
			clog << a;
			for(int j = 0; j < scenesOrder.size(); j++){
				int sc = scenesOrder[j].second;
				clog << '\t';
				if (T[a][sc]){
					clog << 1;
				}
			}
			clog << endl;
		}
		*/


		vector<bool> empac(nScenes, true);
		for (int i = 0; i < actorsOrder.size(); i++){
			int a = actorsOrder[i].second;
			int scenesVisited = 0;
			for(int j = 0; j < scenesOrder.size(); j++){
				int sc = scenesOrder[j].second;
				if(empac[sc]){
					scenesVisited += T[a][sc];
					if (scenesVisited > 1){
						empac[sc] = false;
					}
				}
			}
			clog << endl;
		}
		
		clog << "Empacotamento:" << endl;
		// Imprime primeira linha
		clog << "B_E";
		for(int j = 0; j < scenesOrder.size(); j++){
			int sc = scenesOrder[j].second;
			if( empac[sc]){
				clog << "\tCena " << sc;
			}
		} 
		clog << endl;
		// Imprime outras linhas
		for (int i = 0; i < actorsOrder.size(); i++){
			int a = actorsOrder[i].second;
			clog << a;
			for(int j = 0; j < scenesOrder.size(); j++){
				int sc = scenesOrder[j].second;
				if( empac[sc]){
					clog << '\t';
					if (T[a][sc]){
						clog << 1;
					}
				}
			}
			clog << endl;
		}
		
		//Calcula k3


		if (k3){
			clog << "k3 \t" << k3 << endl;
		}

		int k4 = 0; 
		//Calcula k4

		if (k4){
			clog << "k4 \t" << k4 << endl;
		}
		

		boundVal = k1 + k2 + k3 + k4;
	}

	void updateBest() {
		atualiza_solucao(day, boundVal);
	}
};

// Debuging
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
	os << "lastInitial \t " << s.lastInitial << endl;
	os << "firstFinal \t " << s.firstFinal << endl;

	return os;
}

/* Funcao de comparacao para se obter um heap de minimo */
struct compare {
    bool operator()(const Schedule &l, const Schedule &r) {
        return l.depth < r.depth or (l.depth == r.depth and l.boundVal > r.boundVal);
    }
};

/* Fila de prioridade que contera os nos ativos */
priority_queue<Schedule, vector<Schedule>, compare> ativos;

/* Funcao que executa o branch and bound */
void solve() {
	//s_i's do enunciado
	initializeGlobals();

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
			nos_visitados++;
			/* Gera os filhos do no ativo, calcula os limitantes e continua o processo */
			vector<Schedule> &offspring = topAtivo.branch();
			/* Trata cada filho separadamente */
			for (int j = 0; j < offspring.size(); j++) {
				offspring[j].bound();
				if (offspring[j].boundVal < melhor_custo) {
					if (offspring[j].complete) {
						//melhor_custo = offspring[j].boundVal;
						//best = offspring[j];
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

	/* Resolve o problema */
	solve();

	/* Imprime os resultados */
	imprime_saida();

	return 0;
}
