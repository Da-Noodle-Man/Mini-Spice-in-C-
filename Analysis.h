#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "Circuit.h"
#include "VoltageSource.h"
#include <vector>
#include <complex>

// ============================================
// Analysis: impedance seen between two nodes
// ============================================
// Idea: push 1 A of current into node p and out of node n.
// The voltage that appears between them equals the impedance (Z = V / I).
//
// Nodal analysis finds that voltage:
//   for each part with admittance y = 1/Z between nodes a and b:
//       Y[a][a] += y   Y[b][b] += y   Y[a][b] -= y   Y[b][a] -= y
//   then solve  Y * V = I  for the node voltages V.
// Ground (node 0) is the 0 V reference, so its row/column is left out.

using Complex = std::complex<double>;

// Solve A x = b with Gaussian elimination (partial pivoting)
inline std::vector<Complex> solveLinear(std::vector<std::vector<Complex>> A,std::vector<Complex> b) {
    int n = static_cast<int>(b.size());
    double biggest = 0.0;
    for (const auto& row : A)
        for (const auto& v : row) biggest = std::max(biggest, std::abs(v));

    for (int col = 0; col < n; ++col) {
        int pivot = col;                       // pick the largest entry in this column
        for (int r = col + 1; r < n; ++r)
            if (std::abs(A[r][col]) > std::abs(A[pivot][col])) pivot = r;
        if (std::abs(A[pivot][col]) < 1e-12 * biggest)
            throw std::runtime_error("Circuit cannot be solved (open or disconnected part)");
        std::swap(A[col], A[pivot]);
        std::swap(b[col], b[pivot]);

        for (int r = col + 1; r < n; ++r) {    // eliminate below the pivot
            Complex factor = A[r][col] / A[col][col];
            for (int k = col; k < n; ++k) A[r][k] -= factor * A[col][k];
            b[r] -= factor * b[col];
        }
    }

    std::vector<Complex> x(n);                 // back substitution
    for (int r = n - 1; r >= 0; --r) {
        Complex sum = b[r];
        for (int k = r + 1; k < n; ++k) sum -= A[r][k] * x[k];
        x[r] = sum / A[r][r];
    }
    return x;
}

// Impedance looking into the circuit between nodes p and n (voltage sources ignored)
inline Complex impedanceBetween(const Circuit& circuit, int p, int n, double frequency) {
    if (frequency <= 0.0) throw std::invalid_argument("Frequency must be greater than 0");
    if (p == n) throw std::invalid_argument("Nodes must be different");

    int size = circuit.getNumNodes() - 1;      // unknowns: nodes 1..N-1
    std::vector<std::vector<Complex>> Y(size, std::vector<Complex>(size, 0.0));
    std::vector<Complex> I(size, 0.0);
    auto idx = [](int node) { return node - 1; };   // node 0 (ground) has no row

    for (const auto& c : circuit.getComponents()) {
        if (c->isSource()) continue;           // the source terminals are the "port"
        Complex y = 1.0 / c->getImpedance(frequency);
        int a = c->getNodeA(), b = c->getNodeB();
        if (a != 0) Y[idx(a)][idx(a)] += y;
        if (b != 0) Y[idx(b)][idx(b)] += y;
        if (a != 0 && b != 0) { Y[idx(a)][idx(b)] -= y; Y[idx(b)][idx(a)] -= y; }
    }

    if (p != 0) I[idx(p)] += 1.0;              // 1 A in at p
    if (n != 0) I[idx(n)] -= 1.0;              // 1 A out at n

    std::vector<Complex> V = solveLinear(Y, I);
    Complex Vp = (p != 0) ? V[idx(p)] : 0.0;
    Complex Vn = (n != 0) ? V[idx(n)] : 0.0;
    return Vp - Vn;                            // Z = V / (1 A)
}

// The circuit's single voltage source (only one is supported for now)
inline const VoltageSource* findSource(const Circuit& circuit) {
    const VoltageSource* found = nullptr;
    for (const auto& c : circuit.getComponents())
        if (const auto* v = dynamic_cast<const VoltageSource*>(c.get())) {
            if (found) throw std::runtime_error("Only one voltage source is supported for now");
            found = v;
        }
    if (!found) throw std::runtime_error("Circuit has no voltage source");
    return found;
}

// Impedance of the whole circuit as seen by its voltage source
inline Complex circuitImpedance(const Circuit& circuit, double frequency) {
    const VoltageSource* src = findSource(circuit);
    return impedanceBetween(circuit, src->getNodeA(), src->getNodeB(), frequency);
}

#endif // ANALYSIS_H
