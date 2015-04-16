#include<iostream>
#include<map>
#include<cilk/cilk.h>
#include<stdlib.h>
#include<time.h>
#include<math.h>
#include<algorithm>
#include<sys/time.h>

using namespace std;
#define UDBG //cout <<"At: " << __func__ << ": " << __LINE__ << endl


class Edge
{
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

int *
initArr(int n)
{
	int *L = new int[n+1];

	cilk_for (int i = 0; i <= n; i++)
		L[i] = 0;

	return L;
}

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

void
Prefix_Sum(int X[], int n)
{
	for (int i = 1; i <= n; i++)
		X[i] += X[i-1];

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
        //delete[] X;
        return S;
}


/*
int * Par_Prefix_Sum(int X[], int n)
{
        int *S = initArr(n);
        if (n == 1)
                S[1] = X[1];
        else {
                int *Y = new int[n/2];
                cilk_for (int i = 1; i <= n/2; ++i) {
                        Y[i] = X[2*i-1]+X[2*i];
                }
                int *Z = Par_Prefix_Sum(Y, n/2);
                cilk_for (int i = 1; i <= n; ++i) {
                        if (i == 1)
                                S[1] = X[1];
                        else if(i%2 == 0)
                                S[i] = Z[i/2];
                        else
                                S[i] = Z[(i-1)/2]+X[i];
                }
        }
        return S;
}
*/

/*
int* Par_Randomized_CC(int n, Edge E[], int m, int L[])
{
        int edges = m;
        if (edges == 0)
                return L;
        int *C = new int[n+1];
        int *M = new int[n+1];
        int edgesRounded = pow2roundup(edges);

        int *S = new int[edgesRounded+1];

        cilk_for (int i = 1; i <= n; ++i) {
                srand(time(NULL));
                C[i] = rand()%2;
        }

        cilk_for (int i = 1; i <= edges; ++i) {
                int u = E[i].u;
                int v = E[i].v;
                if (C[u] == 0 && C[v] == 1)
                        L[u] = L[v];
        }

        cilk_for (int i = 1;i <= edges; ++i) {
                if (L[E[i].u] != L[E[i].v])
                        S[i] = 1;
                else
                        S[i] = 0;
        }

        S = Par_Prefix_Sum(S, edgesRounded);

        Edge *F = new Edge[S[edgesRounded]+1];

        cilk_for(int i = 1; i <= edges; ++i) {
                int u = E[i].u;
                int v = E[i].v;
                if (L[u] != L[v])
                        F[S[i]] = Edge(L[u], L[v]);
        }

        M = Par_Randomized_CC(n, F, S[edgesRounded], L);

        cilk_for (int i = 1; i <= edges; ++i) {
                int u = E[i].u;
                int v = E[i].v;
                if (v == L[u]) {
                        M[u] = M[v];
                }
        }

        delete[] C;
        delete[] S;

        return M;
}
*/

long
Par_Sum(int X[], long p, long r) {
        if (p < r) {
                long q = (p+r)/2;
                long sum1, sum2;
                sum1 = cilk_spawn Par_Sum(X, p, q);
                sum2 = Par_Sum(X, q+1, r);
                cilk_sync;
                return sum1+sum2;
        }
        else if (p == r) {
                return X[p];
        }
        return 0;
}

void
Par_Find_Roots(int n, int S[])
{
	int *F = initArr(n);
	cilk_for (int v = 1; v <= n; v++) {
		F[v] = v;
	}

	int flag = 1;
	int max = n;

	while (flag == 1) {
		flag = 0;
		int *E = initArr(max);

		cilk_for (int index = 1; index <= max; index++) {
			int v = F[index];
			S[v] = S[S[v]];
			if (S[v] != S[S[v]]) {
				flag = 1;
				E[index] = 1;
			}
		}

		E = Par_Prefix_Sum(E, max);

		cilk_for (int index = 1; index <= max; index++) {
			int prev = 0;
			if (index != 1)
				prev = E[index-1];
			if (E[index] - prev != 0)
				F[E[index]] = F[index];
		}

		max = E[max];
	}
}


void
Find_Roots(int n, int P[])
{
	int *S = P;
	int flag = 1;

	while (flag == 1) {
		flag = 0;
		cilk_for (int i = 1; i <= n; i++) {
			S[i] = S[S[i]];
			if (S[i] != S[S[i]])
				flag = true;
		}
	}

}


int *
Par_Deterministic_CC(Edge E[], int L[], int m, int n)
{
	if (m == 0)
		return L;

        UDBG;
	int n1 = pow2roundup(n);
	int *l2h = initArr(n1);
	int *h2l = initArr(n1);

	//?
	int m1 = pow2roundup(m);
	int *S = initArr(m1);

        UDBG;
	cilk_for (int i = 1; i <= m; i++) {
		if (E[i].u < E[i].v)
			l2h[E[i].u] = 1;
		else
			h2l[E[i].u] = 1;
	}

        UDBG;
	//l2h = Par_Prefix_Sum(l2h, n);
	//h2l = Par_Prefix_Sum(h2l, n);

	long sum1 = Par_Sum(l2h, 1, n1);
	long sum2 = Par_Sum(h2l, 1, n1);

        UDBG;
	cilk_for (int i = 1; i <= m; i++) {
		if (sum1 >= sum2 && E[i].u < E[i].v)
			L[E[i].u] = E[i].v;
		else if (sum1 < sum2 && E[i].u > E[i].v)
			L[E[i].u] = E[i].v;
	}

        UDBG;
	Par_Find_Roots(n, L);

        UDBG;
	cilk_for (int i = 1; i <= m; i++) {
		if (L[E[i].u] != L[E[i].v])
			S[i] = 1;
		else
			S[i] = 0;
	}

        UDBG;
	//cout<<"Before Prefix_Sum: m="<<m<<", S[m]="<<S[m]<<endl;
	S = Par_Prefix_Sum(S, m1);
	//cout<<"After Prefix_Sum: m="<<m<<", S[m]="<<S[m]<<endl;

        UDBG;
	//cout << "new F: m="<<m <<", S[m]="<<S[m]<<endl;
	Edge *F = new Edge[S[m]+1];

        UDBG;
	cilk_for(int i = 1; i <= m; i++) {
		int u = E[i].u;
		int v = E[i].v;
		if (L[u] != L[v])
			F[S[i]] = Edge(L[u], L[v]);
	}

	int S_size = S[m1];

        UDBG;
	delete[] E;
	delete[] l2h;
	delete[] h2l;
	delete[] S;

        UDBG;
	//S_size is correct ?
	m = S_size;
	L = Par_Deterministic_CC(F, L, m, n);

	return L;
}

int main()
{
        int n, m;
        struct timeval start_time, end_time;

        cin >> n >> m;
        UDBG;
        Edge *E = new Edge[m+1];
        for (int i = 1; i <= m; ++i) {
                int u, v;
                cin >> u >> v;
                E[i].u = u;
                E[i].v = v;
        }

        UDBG;
        int *L = initArr(n);
        for (int i = 1; i <= n; ++i) {
                L[i] = i;
        }

        UDBG;
        gettimeofday(&start_time, NULL);
        L = Par_Deterministic_CC(E, L, m, n);
        gettimeofday(&end_time, NULL);

        UDBG;
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

        cout << "Time: "<< (end_time.tv_sec * 1000000 + end_time.tv_usec) - (start_time.tv_sec * 1000000 + start_time.tv_usec);

        return 0;
}
