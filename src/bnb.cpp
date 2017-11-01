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
#define EVERY(i, s, e) (i = (s); i < (e); i++)


// Funções para depurar
template <typename T, typename S>
ostream& operator<<(ostream& os, const pair <T,S>& p) {  
	return os << '(' << p.first << ' ' << p.second << ')';  
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

typedef vector < list < pair <int, int> > > AdjList;


int main(){
	int n /*number of scenes*/, m /*number of actors*/;
	scanf("%d\n%d", &n,&m);
	int i,j;
	vector< vector <int> > T;
	T.resize(m);
	for EVERY(i, 0,m) {
		T[i].resize(n);
		for EVERY(j, 0,n){
			scanf("%d", &T[i][j]);
		}
	}

	vector< int > cost;
	cost.resize(m);
	for EVERY(i, 0,m) {
		scanf("%d", &cost[i]);
	}

	for EVERY(i,0,m) {
		printf("%4d", T[i][0]);
		for EVERY(j, 1,n) {
			printf(" %4d", T[i][j]);
		}
		printf("\n");
	}

	printf("%4d", cost[0]);
	for EVERY(i, 1,m) {
		printf(" %4d", cost[i]);
	}
	printf("\n");


}






