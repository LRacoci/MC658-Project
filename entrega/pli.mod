/* declaracoes de conjuntos e parametros */

/* quantidade de cenas e dias */
param n, integer, >=1;  
/* quantidade de atores */
param m, integer, >= 1; 

/* conjunto de indices de atores */
set ATORES := {1..m}; 
/* conjunto de indices de cenas */
set CENAS := {1..n};  
/* conjunto de indices de dias de gravacao */
set DIAS := {1..n}; 

set OPCOES := {1..4};

/* matriz T */
param T{a in ATORES, c in CENAS};

/* custo diário de espera */
param c{a in ATORES};

/* numero de cenas nas quais cada ator participa */
param participacao{a in ATORES} := sum{sc in CENAS} T[a,sc];

param nA := m;
param nC := n;
param nD := n;
param salario{a in ATORES} := c[a];

/* ===> variaveis: */
/* Permutação: Indica se a cena sc será gravada no dia d */
var Permutacao{sc in CENAS, d in DIAS},binary;
/* Indica se o ator a ainda não teve que gravar nenhuma cena até o dia d */
var Inicio{a in ATORES, d in DIAS}, binary; 
/* Indica se o ator já gravou todas as suas cenas no dia d */
var Final{a in ATORES, d in DIAS}, binary;

/* Guarda o custo total a ser minimizado (util para imprimir)*/
var custoTotal, >= 0;

/* ===> funcao objetivo */
minimize custo_minimo:
	custoTotal;

/* ===> restricoes */

/* Define a função objetivo a ser minimizada */
s.t. define_custoTotal:
	custoTotal == sum{a in ATORES} salario[a] * (nD - participacao[a] - sum{d in DIAS} (Inicio[a,d] + Final[a,d])); 
/* Uma cena sera gravada a cada dia */
s.t. unicidade_de_cenas{d in DIAS}:
	sum{sc in CENAS} Permutacao[sc,d] == 1;

/* Um dia para cada cena ser gravada */
s.t. unicidade_de_dias{sc in CENAS}:
	sum{d in DIAS} Permutacao[sc,d] == 1;

/* Um dia de início é precedido apenas por dias de início */
s.t. inicio_decrescente{a in ATORES, d in {1..nD-1}}:
	Inicio[a,d] >= Inicio[a,d+1];

/* Um dia de final é sucedido apenas por dias de final */
s.t.  final_crescente {a in ATORES, d in {1..nD-1}}:
	Final[a,d] <= Final[a,d+1];

/* Garante que não haerá espera negativa ou maior que 1 */
s.t. restringe_espera{a in ATORES, d in DIAS}:
	0 <= Inicio[a,d] + sum{sc in CENAS} (T[a,sc] * Permutacao[sc,d]) + Final[a,d] <= 1;

/* resolve problema */
solve;

printf: "%d", sum{d in DIAS} d*Permutacao[1,d];
printf{sc in {2..nD}} " %d", sum{d in DIAS} d*Permutacao[sc,d];
printf: "\n";

printf: "%d\n", custoTotal;


end;
