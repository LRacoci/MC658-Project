/*
	Nome (RA)
	Lucas Alves Racoci (156331)
	Luiz Fernando Fonseca (156475)
*/

#include <vector>
#include <queue>
#include <iostream>
#include <limits.h>

using namespace std;

/* Definicoes de macros que facilitam o desenvolvimento do codigo */
#define INF INT_MAX
#define MAT(T) vector < vector <T> >

class Schedule {
public:
	int next;
	int boundVal;
	vector < int > days;
	vector < int > chosen;

	void init(int next, int boundVal, int n) {
		this->next = next;
		this->boundVal = boundVal;
		days.resize(n, 0);
		chosen.resize(n, false);
	}

};

/* Funcao de comparacao para se obter um heap de minimo */
struct compare {  
	bool operator()(const Schedule & l, const Schedule & r) {  
		return l.boundVal > r.boundVal;
	}  
};

/* Classe que define o problema que esta sendo resolvido e guarda as suas informacoes */
class Problem {
public:
	int n, // number of scenes
		m; // number of actors

	MAT(int) T; // m x n
	vector < int > cost;
	vector < int > part;
	int bestCost;
	Schedule best;

	Problem(){
		bestCost = INF;
	}

	/* Calcula os s_i's do enunciado */
	void compute_part() {
		part.resize(m, 0);
		for(int i = 0; i < m; i++){
			for (int j = 0; j < n; j++){
				part[i] += T[i][j];
			}
		}
	}
	
	/* Funcao que executa o branch and bound */
	void solve(){
		//s_i's do enunciado
		compute_part();

		/* Fila de prioridade que contera os nos ativos */
		best.init(0, 0, n);
		priority_queue<Schedule, vector<Schedule>, compare > pq;
		pq.push(best);

		/* Explora os nos ativos */
		while (not pq.empty()) {
			/* Retira o melhor no da fila */
			Schedule k = pq.top();
			pq.pop();

			/* Gera os filhos do no ativo, calcula os limitantes e continua o processo */
			

		}

	}

};

/* Funcao que faz a entrada dos dados e os coloca dentro de uma classe problema */
istream & operator>> (istream& is, Problem & p){
	int i, j;

	is >> p.n >> p.m;
	p.T.resize(p.m);
	for (i = 0; i < p.m; i++) {
		p.T[i].resize(p.n);
		for (j = 0; j < p.n; j++){
			is >> p.T[i][j];
		}
	}

	p.cost.resize(p.m);
	for (i = 0; i < p.m; i++) {
		is >> p.cost[i];
	}

	return is;
}

/* Funcao que faz a saida dos dados calculados pelo problema */
ostream& operator<<(ostream& os, const Problem & p) {  
	int j;

	os << p.best.days[0];
	for (j = 1; j < p.best.days.size(); j++) {
		os << ' ' << p.best.days[j];
	}
	os << endl;

	os << p.bestCost << endl << p.best.boundVal << endl;

	return os;  
}

int main(){
	Problem p;

	/* Le os dados do problema */
	cin >> p;

	/* Resolve o problema */
	p.solve();

	/* Imprime os resultados */
	cout << p;

	return 0;
}
