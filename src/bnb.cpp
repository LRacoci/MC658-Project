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

class Problem;

/* Classe que representa uma atribuicao de cenas aos dias */
class Schedule {
public:
	int depth; // Nivel na arvore de busca
	int l, r; // Indicam os locais de insercao de novas escolhas
	int boundVal; // Limitante inferior da solucao
	bool complete; // Indica se a solucao e completa
	vector < int > days; // Permutacao das cenas
	vector < int > chosen; // Indica quais cenas ja foram escolhidas
	vector < int > first; // Indica o primeiro dia de cada ator
	vector < int > last; // Indica o ultimo dia de cada ator
	Problem * p; // Ponteiro para a classe problema
	vector < Schedule > offspring; // Cria os filhos do no atual

	Schedule() {}

	/* Cria o objeto inicial de Schedule */
	Schedule(Problem * p);

	/* Cria objetos subsequentes de Schedule a partir de outro ja criado */
	Schedule(Schedule * dad, int choice);

	/* Retorna os filhos do no corrente */
	vector < Schedule > & branch();

	/* Calcula o limitante do no */
	void bound();

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
	vector < int > cost; // Vetor com os custos das cenas
	vector < int > part; // Vetor com a participacao dos atores
	int bestCost; // Melhor custo encontrado
	Schedule best; // Melhor solucao encontrada

	Problem() {
		bestCost = INF;
	}

	/* Calcula os s_i's do enunciado */
	void compute_part();
	
	/* Funcao que executa o branch and bound */
	void solve();

};

/* Cria o objeto inicial de Schedule */
Schedule::Schedule(Problem * p) {
	this->p = p;
	depth = 0;
	l = 0;
	r = p->n - 1;
	boundVal = INF;
	complete = false;
	days.resize(p->n, -1);
	chosen.resize(p->n, false);
	first.resize(p->m, p->n);
	last.resize(p->m, -1);
}

/* Cria objetos subsequentes de Schedule a partir de outro ja criado */
Schedule::Schedule(Schedule * dad, int choice) :
	p(dad->p),
	days(dad->days),
	chosen(dad->chosen),
	first(dad->first), 
	last(dad->last)
{
	/* Incrementa o nivel do no atual em relacao ao seu superior */
	depth = dad->depth + 1;
	complete = depth == p->n;

	/* Checa o lado do vetor que a nova escolha sera inserida,
	 * atualiza os vetores que indicam o primeiro e ultimo dia
	 * de cada ator e atualiza os indices direito e esquerdo */
	int chosen_day;
	if (depth % 2 == 1){
		r = dad->r;
		l = dad->l + 1;
		chosen_day = l - 1;
	} else {
		l = dad->l;
		r = dad->r - 1;
		chosen_day = r + 1;
	}

	days[chosen_day] = choice;
	for (int i = 0; i < p->m; i++){
		if (p->T[i][choice]){
			if (chosen_day < first[i]){
				first[i] = chosen_day;
			}
			if (chosen_day > last[i]){
				last[i] = chosen_day;
			}
		}
	}

}

/* Retorna os filhos do no corrente */
vector < Schedule > & Schedule::branch() {

	for (int j = 0; j < p->n; j++){
		if (not chosen[j]) {
			chosen[j] = true;
			Schedule child(this, j);
			offspring.push_back(child);
		}
	}
	
	return offspring;
}

/* Calcula o limitante do no */
void Schedule::bound() {
	int val = 0;

	for (int i = 0; i < p->m; i++) {
		// if i in A(P) <==> di != none != ei
		if (first[i] < p->n and last[i] >= 0) {
			val += p->cost[i] * (last[i] - first[i] + 1 - p->part[i]);
		}
	}

	boundVal = val;
}

/* Calcula os s_i's do enunciado */
void Problem::compute_part() {
	part.resize(m, 0);
	for(int i = 0; i < m; i++){
		for (int j = 0; j < n; j++){
			part[i] += T[i][j];
		}
	}
}

/* Funcao que executa o branch and bound */
void Problem::solve() {
	//s_i's do enunciado
	compute_part();

	/* Fila de prioridade que contera os nos ativos */
	Schedule root(this);
	priority_queue<Schedule, vector<Schedule>, compare > pq;
	pq.push(root);

	/* Explora os nos ativos */
	while (not pq.empty()) {
		/* Retira o melhor no da fila */
		Schedule k = pq.top();
		pq.pop();

		/* Gera os filhos do no ativo, calcula os limitantes e continua o processo */
		vector < Schedule > & offspring = k.branch();
		
		/* Trata cada filho separadamente */
		for (int j = 0; j < offspring.size(); j++){
			offspring[j].bound();
			if (offspring[j].boundVal < bestCost) {
				if (offspring[j].complete) {
					bestCost = offspring[j].boundVal;
					best = offspring[j];
				} else {
					pq.push(offspring[j]);
				}
			}

		}

	}

}

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

	os << p.best.days[0] + 1;
	for (j = 1; j < p.best.days.size(); j++) {
		os << ' ' << p.best.days[j] + 1;
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
