#include<iostream>
#include<cilkview.h>
#include<map>
#include<cilk/cilk.h>
#include<stdlib.h>
#include<math.h>
#include<algorithm>
#include<sys/time.h>
#include <string.h>

#define UDBG cout << "At : " << __func__ << __LINE__ << endl;

using namespace std;

class Edge {
	public:
		int u;
		int v;
		Edge(){};
		Edge(int v1, int v2) {
			u = v1;
			v = v2;
		}
		~Edge(){};
};

int pow2roundup(int x)
{
	if (x < 0)
		return 0;
	--x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x+1;
}

int* initArr(int n) {
	int *A = new int[n+1];
	cilk_for (int i = 0; i <= n; ++i)
		A[i] = 0;
	return A;
}

void Par_Prefix_Sum(int *X, int n) {
	if (n > 1) {
		int* Y = initArr(n/2);
		cilk_for (int i = 1; i <= n/2; ++i) {
			Y[i] = X[2*i-1]+X[2*i];
		}

		Par_Prefix_Sum(Y, n/2);
		int *T = initArr(n);
		cilk_for (int i = 1; i <= n; ++i) {
			if (i == 1)
				T[1] = X[1];
			else if(i%2 == 0)
				T[i] = Y[i/2];
			else
				T[i] = Y[(i-1)/2]+X[i];
		}
		cilk_for (int i = 1; i <= n; ++i) {
			X[i] = T[i];
		}

		delete [] T;
		delete [] Y;
	}
}

void Prefix_Sum(int X[], int S[], int n) {
	int *t = new int[1024*1024*100];
	for (int i = 1; i <= n; ++i) {
		S[i] += S[i-1];
	}
}

int RANDOM(){
	return rand()%2;
}

void RANDOM_HOOK(int *V, int *L, int *N, int *I, int n) {
	int* C = initArr(n);
	int* H = initArr(n);

	cilk_for(int i = 1; i <= n; ++i) {
		int u = V[i];
		C[I[u]] = RANDOM();
		H[I[u]] = 0;
	}

	cilk_for(int i = 1; i <= n; ++i) {
		int u = V[i];
		int v = N[I[u]];
		if (C[I[u]] == 0 && C[I[v]] == 1) {
			L[u] = v;
			H[I[u]] = 1;
			H[I[v]] = 1;
		}
	}

	cilk_for(int i = 1; i <= n; ++i) {
		int u = V[i];
		if (H[I[u]] == 1) {
			C[I[u]] = 1;
		}
		else if (C[I[u]] == 0) {
			C[I[u]] = 1;
		}
		else {
			C[I[u]] = 0;
		}
	}

	cilk_for(int i = 1; i <= n; ++i) {
		int u = V[i];
		int v = N[I[u]];
		if (C[I[u]] == 0 && C[I[v]] == 1) {
			L[u] = L[v];
		}
	}

	delete [] H;
	delete [] C;
}


void Par_Randomized_CC(int* V, Edge* E, int* L, int* I, int n, int m) {
	if (m == 0) {
		return;
	}

	int *N = initArr(n);
	cilk_for(int i = 1; i <= m; ++i) {
		int u = E[i].u;
		int v = E[i].v;
		N[I[u]] = v;
		N[I[v]] = u;
	}

	RANDOM_HOOK(V, L, N, I, n);
	delete [] N;

	int n1 = pow2roundup(n);
	int *SV = initArr(n1);
	cilk_for (int i = 1; i <= m; ++i) {
		int u = E[i].u;
		int v = E[i].v;
		if (L[v] != L[u]) {
			SV[I[L[v]]] = 1;
			SV[I[L[u]]] = 1;
		}
	}
	Par_Prefix_Sum(SV, n1);
	int *W = initArr(SV[n1]);
	cilk_for(int i = 1; i <= m; ++i) {
		int u = E[i].u;
		int v = E[i].v;
		if (L[v] != L[u]) {
			W[SV[I[L[v]]]] = L[v];
			W[SV[I[L[u]]]] = L[u];
		}
	}
	cilk_for (int i = 1; i <= SV[n1]; ++i) {
		I[W[i]] = i;
	}
	int tn = SV[n1];
	delete[] SV;

	int m1 = pow2roundup(m);
	int *SE = initArr(m1);
	cilk_for (int i = 1; i <= m; ++i) {
		int u = E[i].u;
		int v = E[i].v;
		if (L[u] != L[v]) {
			SE[i] = 1;
		}
	}
	Par_Prefix_Sum(SE, m1);
	int tm = SE[m1];
	Edge *F = new Edge[tm + 1];
	cilk_for (int i = 1; i <= m; ++i) {
		int u = E[i].u;
		int v = E[i].v;
		if (L[u] != L[v]) {
			F[SE[i]].u = L[u];
			F[SE[i]].v = L[v];
		}
	}
	cilk_for (int i = 1; i <= tm; ++i) {
		E[i] = F[i];
	}

	delete [] F;
	delete [] SE;

	Par_Randomized_CC(W, E, L, I, tn, tm);

	cilk_for(int i = 1; i <= n; ++i) {
		int v = V[i];
		L[v] = L[L[v]];
	}

	delete [] W;
}

int main() {
	int n, m;
	struct timeval start_time, end_time;
	cilkview_data_t start;

	cin >> n >> m;

	srand(time(NULL));

	Edge *E = new Edge[m + 1];
	for (int i = 1; i <= m; ++i) {
		int u, v;
		cin >> u >> v;
		E[i].u = u;
		E[i].v = v;
	}

	int *V = initArr(n);
	int *L = initArr(n);
	int *I = initArr(n);
	for (int i = 1; i <= n; ++i) {
		L[i] = i;
		V[i] = i;
		I[i] = i;
	}

	__cilkview_query(start);
        gettimeofday(&start_time, NULL);
	Par_Randomized_CC(V, E, L, I, n, m);
        gettimeofday(&end_time, NULL);
	__cilkview_report(&start, NULL, "ParallelRandomizedCC", CV_REPORT_WRITE_TO_RESULTS);

	 map<int, int> componentMap;

        for (int i = 1; i <= n; ++i) {
                componentMap[L[i]]++;
        }

        int c = componentMap.size();
        int *components = initArr(c);
        int index = 1;
        for (map<int, int>::iterator it = componentMap.begin(); it != componentMap.end(); it++) {
                components[index++] = it->second;
        }

        sort(components, components+c+1);

        cout << c << endl;
        for (int i = c; i >= 1; --i) {
                cout << components[i] << endl;
        }

        cout << (end_time.tv_sec * 1000000 + end_time.tv_usec) - (start_time.tv_sec * 1000000 + start_time.tv_usec);

	delete components;
	delete[] I;
	delete[] L;
	delete[] V;
	delete[] E;

        return 0;
}

