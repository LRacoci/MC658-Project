#include <vector>
#include <iostream>

using namespace std;

#define MAT(T) vector < vector <T> >

/* Programa que calcula o custo de uma solucao, utilizando os
 * dados de entrada e tambem o vetor solucao do branch and bound */
int main() {
	int i, j, n, m;
	MAT(int) T, P;
	vector < int > cost;
	vector < int > out;
	vector < int > sol;
	vector < int > part;
	int c = 0;

	cin >> n >> m;
	T.resize(m);
	for (i = 0; i < m; i++) {
		T[i].resize(n);
		for (j = 0; j < n; j++){
			cin >> T[i][j];
		}
	}

	cost.resize(m);
	for (i = 0; i < m; i++) {
		cin >> cost[i];
	}

	out.resize(n);
	for (i = 0; i < n; i++) {
		cin >> out[i];
		out[i] -= 1;
	}

	sol.resize(n);
	for (i = 0; i < n; i++) {
		sol[out[i]] = i;
	}

	P.resize(m);
	for (i = 0; i < m; i++) {
		P[i].resize(n);
		for (j = 0; j < n; j++){
			P[i][j] = T[i][sol[j]];
		}
	}

	part.resize(m, 0);
	for(int i = 0; i < m; i++){
		for (int j = 0; j < n; j++){
			part[i] += T[i][j];
		}
	}

	for (i = 0; i < m; i++) {
		int aux = 0;
		while (P[i][aux] != 1) {
			aux++;
		}
		for (j = aux; j < n; j++){
			if (P[i][j] == 1) {
				part[i] -= 1;
			} else {
				if (part[i] > 0) {
					c += cost[i];
				}
			}
		}
	}

	for (i = 0; i < m; i++) {
		for (j = 0; j < n; j++) {
			cout << P[i][j] << " ";
		}
		cout << endl;
	}

	cout << c << endl;

	return 0;
}
