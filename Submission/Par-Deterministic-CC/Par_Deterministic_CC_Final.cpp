#include<iostream>
#include<map>
#include<cilk/cilk.h>
#include<stdlib.h>
#include<time.h>
#include<math.h>
#include<algorithm>
#include<sys/time.h>
#include<cilkview.h>

using namespace std;
#define UDBG cout <<"At: " << __func__ << ": " << __LINE__ << endl;


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
        return S;
}


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

	int n1 = pow2roundup(n);
	int *l2h = initArr(n1);
	int *h2l = initArr(n1);

	int m1 = pow2roundup(m);
	int *S = initArr(m1);

	cilk_for (int i = 1; i <= m; i++) {
		if (E[i].u < E[i].v)
			l2h[E[i].u] = 1;
		else
			h2l[E[i].u] = 1;
	}

	long sum1 = Par_Sum(l2h, 1, n1);
	long sum2 = Par_Sum(h2l, 1, n1);

	cilk_for (int i = 1; i <= m; i++) {
		if (sum1 >= sum2 && E[i].u < E[i].v)
			L[E[i].u] = E[i].v;
		else if (sum1 >= sum2 && E[i].u > E[i].v)
			L[E[i].v] = E[i].u;
		else if (sum1 < sum2 && E[i].u > E[i].v)
			L[E[i].u] = E[i].v;
		else if (sum1 < sum2 && E[i].u < E[i].v)
			L[E[i].v] = E[i].u;
	}

	Par_Find_Roots(n, L);

	cilk_for (int i = 1; i <= m; i++) {
		if (L[E[i].u] != L[E[i].v])
			S[i] = 1;
		else
			S[i] = 0;
	}

	S = Par_Prefix_Sum(S, m1);
	Edge *F = new Edge[S[m]+1];

	cilk_for(int i = 1; i <= m; i++) {
		int u = E[i].u;
		int v = E[i].v;
		if (L[u] != L[v])
			F[S[i]] = Edge(L[u], L[v]);
	}

	int S_size = S[m1];

	delete[] E;
	delete[] l2h;
	delete[] h2l;
	delete[] S;

	m = S_size;
	L = Par_Deterministic_CC(F, L, m, n);

	return L;
}

int main()
{
        int n, m;
        struct timeval start_time, end_time;
	cilkview_data_t start;

        cin >> n >> m;
        Edge *E = new Edge[m+1];
        for (int i = 1; i <= m; ++i) {
                int u, v;
                cin >> u >> v;
                E[i].u = u;
                E[i].v = v;
        }

        int *L = initArr(n);
        for (int i = 1; i <= n; ++i) {
                L[i] = i;
        }

        gettimeofday(&start_time, NULL);

	__cilkview_query(start);
        L = Par_Deterministic_CC(E, L, m, n);
	__cilkview_report(&start, NULL, "ParallelDeterministicCC", CV_REPORT_WRITE_TO_RESULTS);

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

        cout << "Time: "<< (end_time.tv_sec * 1000000 + end_time.tv_usec) - (start_time.tv_sec * 1000000 + start_time.tv_usec);


        //delete [] E;
	delete [] L;
        return 0;
}

