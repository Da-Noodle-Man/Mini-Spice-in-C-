#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "Circuit.h"
#include "VoltageSource.h"
#include <vector>
#include <complex>
#include <utility>
using namespace std;
using Complex = complex<double>;

vector<Complex> solveLinear(vector<vector<Complex>> A, vector<Complex> b)
{
    int n = static_cast<int>(b.size());
    for (int col = 0; col < n; ++col)
    {
        int pivot = col;
        for (int r = col + 1; r < n; ++r)
        {
            if (abs(A[r][col]) > abs(A[pivot][col]))
            {
                pivot = r;
            }
        }
        swap(A[col], A[pivot]);
        swap(b[col], b[pivot]);
        for (int r = col + 1; r < n; ++r)
        {
            Complex factor = A[r][col] / A[col][col];
            for (int k = col; k < n; ++k)
            {
                A[r][k] -= factor * A[col][k];
            }
            b[r] -= factor * b[col];
        }
    }

    vector<Complex> x(n);
    for (int r = n - 1; r >= 0; --r)
    {
        Complex sum = b[r];
        for (int k = r + 1; k < n; ++k)
        {
            sum -= A[r][k] * x[k];
        }
        x[r] = sum / A[r][r];
    }
    return x;
}

Complex impedanceBetween(const Circuit &circuit, int p, int n, double frequency)
{
    int size = circuit.getNumNodes() - 1;
    vector<vector<Complex>> Y(size, vector<Complex>(size, 0.0));
    vector<Complex> I(size, 0.0);
    auto row = [](int node)
    {
        return node - 1;
    };

    for (const auto &c : circuit.getComponents())
    {
        if (c->isSource())
        {
            continue;
        }
        Complex y = 1.0 / c->getImpedance(frequency);
        int a = c->getNodeA(), b = c->getNodeB();
        if (a != 0)
        {
            Y[row(a)][row(a)] += y;
        }
        if (b != 0)
        {
            Y[row(b)][row(b)] += y;
        }
        if (a != 0 && b != 0)
        {
            Y[row(a)][row(b)] -= y;
            Y[row(b)][row(a)] -= y;
        }
    }

    if (p != 0)
    {
        I[row(p)] += 1.0;
    }
    if (n != 0)
    {
        I[row(n)] -= 1.0;
    }

    vector<Complex> V = solveLinear(Y, I);
    Complex Vp = (p != 0) ? V[row(p)] : 0.0;
    Complex Vn = (n != 0) ? V[row(n)] : 0.0;
    return Vp - Vn;
}

const VoltageSource *findSource(const Circuit &circuit)
{
    const VoltageSource *found = nullptr;
    for (const auto &c : circuit.getComponents())
    {
        if (const auto *v = dynamic_cast<const VoltageSource *>(c.get()))
        {
            found = v;
        }
    }
    return found;
}

Complex circuitImpedance(const Circuit &circuit, double frequency)
{
    const VoltageSource *src = findSource(circuit);
    return impedanceBetween(circuit, src->getNodeA(), src->getNodeB(), frequency);
}

#endif 