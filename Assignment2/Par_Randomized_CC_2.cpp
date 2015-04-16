#include<iostream>
#include<map>
#include<cilk/cilk.h>
#include<stdlib.h>
#include<math.h>
#include<algorithm>
#include<sys/time.h>
#include <string.h>

#define UDBG cout << "At :" << __func__ << ": " << __LINE__ << endl;

using namespace std;

class Edge {
	public:
		int u;
		int v;
		Edge(int v1, int v2) {
			u = v1;
			v = v2;
		}
		Edge() {
			u = 0;
			v = 0;
		}
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

int* Par_Prefix_Sum(int X[], int n) {
	int* S = initArr(n);

	if (n == 1)
		S[1] = X[1];
	else {
		int* Y = initArr(n/2);
		cilk_for (int i = 1; i <= n/2; ++i) {
			Y[i] = X[2*i-1]+X[2*i];
		}
		int* Z = Par_Prefix_Sum(Y, n/2);
		cilk_for (int i = 1; i <= n; ++i) {
			if (i == 1)
				S[1] = X[1];
			else if(i%2 == 0)
				S[i] = Z[i/2];
			else
				S[i] = Z[(i-1)/2]+X[i];
		}

		delete [] Y;
		delete [] Z;
	}

	return S;
}

int RANDOM(){
	return rand()%2;
}

void RANDOM_HOOK(int V[], int L[], int N[], int I[], int n) {
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

	delete [] C;
	delete [] H;
}

int gn;

void Par_Randomized_CC_2(int V[], Edge E[], int L[], int I[], int n, int m) {
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

	int verticesRounded = pow2roundup(n);
	int *SV = initArr(verticesRounded);
	cilk_for (int i = 1; i <= m; ++i) {
		int u = E[i].u;
		int v = E[i].v;
		if (L[v] != L[u]) {
			SV[I[L[v]]] = 1;
			SV[I[L[u]]] = 1;
		}
	}
	SV = Par_Prefix_Sum(SV, verticesRounded);
	int *W = initArr(SV[verticesRounded]);
	cilk_for(int i = 1; i <= m; ++i) {
		int u = E[i].u;
		int v = E[i].v;
		if (L[v] != L[u]) {
			W[SV[I[L[v]]]] = L[v];
			W[SV[I[L[u]]]] = L[u];
		}
	}
	cilk_for (int i = 1; i <= SV[verticesRounded]; ++i) {
		I[W[i]] = i;
	}
	int edgesRounded = pow2roundup(m);
	int *SE = initArr(edgesRounded);
	cilk_for (int i = 1; i <= m; ++i) {
		int u = E[i].u;
		int v = E[i].v;
		if (L[u] != L[v]) {
			SE[i] = 1;
		}
	}
	SE = Par_Prefix_Sum(SE, edgesRounded);
	Edge *F = new Edge[SE[edgesRounded]+1];
	cilk_for (int i = 1; i <= m; ++i) {
		int u = E[i].u;
		int v = E[i].v;
		if (L[u] != L[v]) {
			F[SE[i]] = Edge(L[u], L[v]);
		}
	}

	int tn = SV[verticesRounded];
	int tm = SE[edgesRounded];
	Par_Randomized_CC_2(W, F, L, I, tn, tm);

	cilk_for(int i = 1; i <= n; ++i) {
		int v = V[i];
		L[v] = L[L[v]];
	}

	delete [] N;
	delete [] SV;
	delete [] SE;
	delete [] W;
	delete [] F;
}

int main() {
	int n, m;
	struct timeval start_time, end_time;

	cin >> n >> m;

	gn = n;
	srand(time(NULL));

	Edge *E = new Edge[m+1];
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

        gettimeofday(&start_time, NULL);
	Par_Randomized_CC_2(V, E, L, I, n, m);
        gettimeofday(&end_time, NULL);

        map<int, int> componentMap;

        for (int i = 1; i <= n; ++i) {
                componentMap[L[i]]++;
        }

        int c = componentMap.size();
        int *components = new int[c];
        int index = 0;
        for (map<int, int>::iterator it = componentMap.begin(); it != componentMap.end(); it++) {
                components[index++] = it->second;
        }

        sort(components, components+c);

        cout << c << endl;
        for (int i = c-1; i >= 0; --i) {
                cout << components[i] << endl;
        }

        cout << (end_time.tv_sec * 1000000 + end_time.tv_usec) - (start_time.tv_sec * 1000000 + start_time.tv_usec);

        return 0;
}
