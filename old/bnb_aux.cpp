/*
	Nome (RA)
	Lucas Alves Racoci (156331)
	Luiz Fernando Fonseca (156475)
*/

#include <vector>
#include <list>
#include <stdio.h>
#include <queue>
#include <iostream>
#include <bits/stdc++.h>
#include <limits.h>



using namespace std;

#define MAX(a,b) ( (a) < (b) ? (b) : (a) )
#define MIN(a,b) ( (a) > (b) ? (b) : (a) )

#define INF INT_MAX

#define EVERY_INDEX(i, s, e) (int i = (s); i < (e); i++)
#define EVERY_OTHER_INDEX(i, s, e, p) (int i = (s); i < (e); i+=p)

#define MAT(T) vector < vector <T> >

#define elif else if


typedef enum _ {NOWHERE, LEFT, RIGHT} Position;

// Funções para depurar
template <typename T, typename S>
ostream& operator<<(ostream& os, const pair <T,S>& p) {  
	return os << '(' << p->first << ' ' << p->second << ')';  
} 
template <typename T>
ostream& operator<<(ostream& os, const list <T>& lst) {  
	typename list<T>::const_iterator it = lst.begin(), end = lst.end();
	os << '[' << *it;
	for(it++ ; it != end ; ++it){
		os << ' ' << *it;
	}
	os << ']';
	return os;  
}

template <typename T>
ostream& operator<<(ostream& os, const vector <T>& vec) {  
	os << '{' << endl;
	for(unsigned i = 0; i < vec.size(); i++){

		os << '\t' << i << ": " << vec[i] << endl;
	}
	os << '}' << endl;
	return os;  
} 



class Problem;

class DaySchedule {
public:
	Problem * p;
	int depth;
	int l,r;
	bool bounded;
	int  boundVal;
	bool complete;
	vector < int > day;
	vector < bool > chosen;
	vector < DaySchedule > offspring;
	DaySchedule * dad;

	vector < int > first;
	vector < int > last;
	
	DaySchedule(Problem * p);

	DaySchedule(DaySchedule * dad, int choice) ;

	int operator[] (int j) const;
	bool operator< ( const DaySchedule other) const ;
	void brew();
	int bound();
	vector < DaySchedule > & branch();
};

ostream& operator<<(ostream& os, const DaySchedule& s);

class Problem {
public:
	int n, // number of scenes
		m; // number of actors

	MAT(int) T; // m x n
	vector < int > cost;
	
	vector < int > participation;

	int bestCost;


	Problem(){
		bestCost = INF;
	}

	void compute_participation();
	
	DaySchedule solve();

};


	DaySchedule::DaySchedule(Problem * p): 
		p(p), 
		day(p->n, -1), 
		chosen(p->n, false),
		first(p->m, p->n), 
		last(p->m, -1)
	{
		bounded = false;
		depth = 0;
		boundVal = INF;
		complete = false;
		l = 0;
		r = p->n;
		dad = this;

	}

	DaySchedule::DaySchedule(DaySchedule * dad, int choice): 
		dad(dad), 
		p(dad->p), 
		day(dad->day),
		chosen(dad->chosen),
		first(dad->first), 
		last(dad->last)
	{
		bounded = false;
		depth = dad->depth + 1;
		int chosen_day;
		if (depth % 2 == 1){
			r = dad->r;
			l = dad->l + 1;
			chosen_day = l-1;
		} else {
			l = dad->l;
			r = dad->r - 1;
			chosen_day = r;
		}

		day[chosen_day] = choice;

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
		complete = depth == p->n;
		boundVal = INF;
		boundVal = bound();
		cout << "choice = day[" << chosen_day << "] = " << choice << endl;
		cout << *this << endl;

	}

	int DaySchedule::operator[] (int j) const{
		return day[j];
	}
	bool DaySchedule::operator< (const DaySchedule other) const{
		//return (depth == other.depth) ? (boundVal < other.boundVal) : (depth < other.depth);
		//return boundVal < other.boundVal or (boundVal == other.boundVal and depth > other.depth);
		//return depth < other.depth;
		return depth < other.depth or (depth == other.depth and boundVal > other.boundVal);
	}
	void DaySchedule::brew(){
		return;
	}
	int DaySchedule::bound(){
		if (bounded) {
			return boundVal;
		}
		bounded = true;

		bool flag = false;
		boundVal = 0;
		for(int i = 0; i < p->m; i++){
			// if i in A(P) <==> di != none != ei
			if(first[i] < p->m and last[i] >= 0){
				boundVal += p->cost[i] * (last[i] - first[i] + 1 - p->participation[i]);
				flag = true;
			}
		}
		if (flag == false){
			boundVal = INF;
		}

		return boundVal;
	}
	vector < DaySchedule > & DaySchedule::branch(){
		int k = 0;
		for (int j = 0; j < p->n; j++){
			if (not chosen[j]){
				chosen[j] = true;
				DaySchedule child(this, j);
				offspring.push_back(child);
				/*
				cout << "Branch with j = " << j;
				cout << " this->depth = " << this->depth;
				cout << endl;
				cout << offspring[k];
				*/
				k++;
			}
		}
		return offspring;
	}

/*
ostream& operator<<(ostream& os, const DaySchedule& s) {  
	os << s[0];
	for EVERY_INDEX(j, 1,s.p->n) {
		//printf(" %d", s[j]);
		os << ' ' << s[j];
	}
	os << endl;

	os << s.p->bestCost << endl << s.boundVal << endl;

	return os;  
} 
*/

// Debuging
ostream& operator<<(ostream& os, const DaySchedule& s) {
	/*
	Actors 	first 	Cenas	last
	*/
	os << "depth = " << s.depth << endl;
	os << "Actors\tfirst";
	for EVERY_INDEX(j, 0,s.p->n) {
		os << '\t';
		if (s[j] != -1){
			os << s[j];
		}
	}
	os << "\tlast\tparticipation\tcost";
	os << endl;
	for EVERY_INDEX(i, 0, s.p->m){
		os << i << '\t' << s.first[i];
		for EVERY_INDEX(j, 0,s.p->n) {
			os << '\t';
			if (s[j] != -1){
				os << s.p->T[i][s[j]];
			}
		}
		os << '\t' << s.last[i];
		os << '\t' << s.p->participation[i];
		os << '\t' << s.p->cost[i] << endl;
	}

	os << s.p->bestCost << endl << s.boundVal << endl;

	return os;  
}

	void Problem::compute_participation(){
		participation.resize(m, 0);
		for(int i = 0; i < m; i++){
			for (int j = 0; j < n; j++){
				participation[i] += T[i][j];
			}
		}
	}

/*
B&B(k); (* considerando problema de maximização *)
	Nós-ativos ← {nó raiz}; 
	melhor-solução ← {}; 
	z ← −∞;
	Enquanto (Nós-ativos não está vazia) faça
		Escolher um nó k em Nós-ativos para ramificar;
		Remover k de Nós-ativos;
		Gerar os filhos de k: n1, . . . , nq e computar os zni correspondentes;
		(* zni ← −∞ se restriç˜oes impl´ıcitas não são satisfeitas *)
		Para j = 1 at´e q faça
			se (zni ≤ z) então amadurecer o nó ni;
			se não
				Se (ni representa uma solução completa) então
				z ← zni; 
				melhor-solução ← {solução de ni};
			se não adicionar ni à lista Nós-ativos.
			fim-se
		fim-para
	fim-enquanto
fim.
*/

	DaySchedule Problem::solve(){
		//s_i's do enunciado
		compute_participation();
			
		DaySchedule best(this);
		priority_queue<DaySchedule> pq;
		pq.push(best);

		while(not pq.empty()){
			DaySchedule dad = pq.top();
			pq.pop();

			cout << "#dad daySchedule depth = " << dad.depth << endl;
			// Branch
			vector < DaySchedule > & offspring = dad.branch();
			// Bound
			for(int j = 0; j < offspring.size(); j++){
				DaySchedule child = offspring[j];
				int boundVal = child.bound();
				if (boundVal > bestCost){
					child.brew();
				} else if (child.complete) {
					bestCost = boundVal;
					best = child;
				} else {
					pq.push(child);
				}

			}

		}
		return best;
	}

istream & operator>> (istream& is, Problem & p){
	is >> p.n >> p.m;
	p.T.resize(p.m);
	for EVERY_INDEX(i, 0,p.m) {
		p.T[i].resize(p.n);
		for EVERY_INDEX(j, 0,p.n){
			//scanf("%d", &T[i][j]);
			is >> p.T[i][j];
		}
	}
	p.cost.resize(p.m);
	for EVERY_INDEX(i, 0,p.m) {
		//scanf("%d", &cost[i]);
		is >> p.cost[i];
	}
	return is;
} 

int main(){
	Problem p;
	cin >> p;
	cout << p.solve();
	/*
	for EVERY_INDEX(i,0,m) {
		printf("%4d", T[i][0]);
		for EVERY_INDEX(j, 1,n) {
			printf(" %4d", T[i][j]);
		}
		printf("\n");
	}

	printf("%4d", cost[0]);
	for EVERY_INDEX(i, 1,m) {
		printf(" %4d", cost[i]);
	}
	*/
	
}






