#include <iostream>
#include <iomanip>
#include <cmath>
#include <complex>
#include <string>
#include <vector>
#include <stdexcept>
#include "Circuit.h"
#include "Resistor.h"
#include "Inductor.h"
#include "Capacitor.h"
#include "ACVoltageSource.h"
#include "DCVoltageSource.h"
#include "Analysis.h"
#include "NodalAnalysis.h"

// ============================================
// Data collected from the user before the circuit is built
// ============================================
struct Entry
{
    char type = 'R'; // 'R', 'L', 'C', 'A' (AC source), 'D' (DC source)
    std::string name;
    int nodeA = 0;
    int nodeB = 0;
    double v1 = 0.0; // R: ohms | L: henries | C: farads | AC: amplitude | DC: volts
    double v2 = 0.0; // AC: phase in degrees
    double v3 = 0.0; // AC: frequency in Hz
};

// ============================================
// Helper Functions
// ============================================

double askNumber(const std::string &prompt)
{
    double v = 0.0;
    std::cout << prompt;
    std::cin >> v;
    return v;
}

int askInt(const std::string &prompt)
{
    int v = 0;
    std::cout << prompt;
    std::cin >> v;
    return v;
}

void printInstructions()
{
    std::cout << "==================================================" << std::endl;
    std::cout << "   MiniSPICE - Build Your Own Circuit             " << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << "HOW TO ENTER DATA:" << std::endl;
    std::cout << "  1. Pick a component from the menu by typing its number" << std::endl;
    std::cout << "     and pressing Enter. Repeat until you are done, then" << std::endl;
    std::cout << "     type 0 to finish and analyse the circuit." << std::endl;
    std::cout << "  2. NODES are whole numbers. Node 0 is GROUND." << std::endl;
    std::cout << "     Number your nodes 0, 1, 2, 3, ... with no gaps." << std::endl;
    std::cout << "  3. Every component sits between two nodes (Node A and Node B)." << std::endl;
    std::cout << "  4. Type values in BASE UNITS (no k, m, u suffixes)." << std::endl;
    std::cout << "     Scientific notation is allowed:" << std::endl;
    std::cout << "       1 kohm   -> 1000   or 1e3" << std::endl;
    std::cout << "       10 mH    -> 0.01   or 1e-2" << std::endl;
    std::cout << "       1 uF     -> 0.000001 or 1e-6" << std::endl;
    std::cout << "  5. Resistor, inductor and capacitor values must be > 0." << std::endl;
    std::cout << "  6. Exactly ONE voltage source (AC or DC) is required." << std::endl;
    std::cout << "     Node A is its POSITIVE terminal, Node B its NEGATIVE one" << std::endl;
    std::cout << "     (usually ground, node 0)." << std::endl;
    std::cout << "  7. AC source: amplitude = peak volts (>= 0)," << std::endl;
    std::cout << "     phase = degrees, frequency = Hz (> 0)." << std::endl;
    std::cout << "  Example: V1 between 1 and 0, R1 between 1 and 2," << std::endl;
    std::cout << "           C1 between 2 and 0." << std::endl;
    std::cout << "==================================================\n"
              << std::endl;
}

void printMenu()
{
    std::cout << "\n------------- COMPONENT MENU -------------" << std::endl;
    std::cout << "  1 - Add Resistor" << std::endl;
    std::cout << "  2 - Add Inductor" << std::endl;
    std::cout << "  3 - Add Capacitor" << std::endl;
    std::cout << "  4 - Add AC Voltage Source" << std::endl;
    std::cout << "  5 - Add DC Voltage Source" << std::endl;
    std::cout << "  0 - Finish and analyse circuit" << std::endl;
    std::cout << "------------------------------------------" << std::endl;
}

// Print impedance in engineering format
void printZ(std::complex<double> Z)
{
    std::cout << std::fixed << std::setprecision(2);
    if (std::abs(Z) > 1e12)
    {
        std::cout << "infinite (open circuit)" << std::endl;
        return;
    }
    std::cout << Z.real() << (Z.imag() < 0 ? " - j" : " + j")
              << std::abs(Z.imag()) << " ohm"
              << "   |Z| = " << std::abs(Z) << " ohm"
              << "   phase = " << std::arg(Z) * 180.0 / M_PI << " deg" << std::endl;
}

// Print voltage as a phasor
void printVoltage(std::complex<double> V)
{
    std::cout << std::fixed << std::setprecision(4);
    std::cout << V.real() << (V.imag() < 0 ? " - j" : " + j")
              << std::abs(V.imag()) << " V"
              << "   |V| = " << std::abs(V) << " V"
              << "   phase = " << std::arg(V) * 180.0 / M_PI << " deg" << std::endl;
}

// Print current as a phasor
void printCurrent(std::complex<double> I)
{
    std::cout << std::fixed << std::setprecision(6);
    std::cout << I.real() << (I.imag() < 0 ? " - j" : " + j")
              << std::abs(I.imag()) << " A"
              << "   |I| = " << std::abs(I) << " A"
              << "   phase = " << std::arg(I) * 180.0 / M_PI << " deg" << std::endl;
}

// ============================================
// Main Program
// ============================================

int main()
{
    printInstructions();

    // ============================================
    // Collect components from the user
    // ============================================
    std::vector<Entry> entries;
    int resCount = 0;
    int indCount = 0;
    int capCount = 0;
    int sourceCount = 0;
    bool done = false;

    while (!done)
    {
        printMenu();
        int choice = askInt("Your choice (0-5): ");

        switch (choice)
        {
        case 1:
        {
            Entry e;
            e.type = 'R';
            resCount++;
            e.name = "R" + std::to_string(resCount);
            std::cout << "Adding " << e.name << " (resistor)" << std::endl;
            e.nodeA = askInt("  Node A (whole number): ");
            e.nodeB = askInt("  Node B (whole number): ");
            e.v1 = askNumber("  Resistance in ohms (> 0, e.g. 1000): ");
            entries.push_back(e);
            std::cout << "  -> " << e.name << " added." << std::endl;
            break;
        }
        case 2:
        {
            Entry e;
            e.type = 'L';
            indCount++;
            e.name = "L" + std::to_string(indCount);
            std::cout << "Adding " << e.name << " (inductor)" << std::endl;
            e.nodeA = askInt("  Node A (whole number): ");
            e.nodeB = askInt("  Node B (whole number): ");
            e.v1 = askNumber("  Inductance in henries (> 0, e.g. 0.01 for 10 mH): ");
            entries.push_back(e);
            std::cout << "  -> " << e.name << " added." << std::endl;
            break;
        }
        case 3:
        {
            Entry e;
            e.type = 'C';
            capCount++;
            e.name = "C" + std::to_string(capCount);
            std::cout << "Adding " << e.name << " (capacitor)" << std::endl;
            e.nodeA = askInt("  Node A (whole number): ");
            e.nodeB = askInt("  Node B (whole number): ");
            e.v1 = askNumber("  Capacitance in farads (> 0, e.g. 1e-6 for 1 uF): ");
            entries.push_back(e);
            std::cout << "  -> " << e.name << " added." << std::endl;
            break;
        }
        case 4:
        {
            if (sourceCount >= 1)
            {
                std::cout << "A voltage source was already added. Only one is supported." << std::endl;
            }
            else
            {
                Entry e;
                e.type = 'A';
                e.name = "V1";
                std::cout << "Adding V1 (AC voltage source)" << std::endl;
                e.nodeA = askInt("  Positive node A (whole number): ");
                e.nodeB = askInt("  Negative node B (whole number, usually 0): ");
                e.v1 = askNumber("  Amplitude, peak volts (>= 0): ");
                e.v2 = askNumber("  Phase shift, degrees (any number, e.g. 0): ");
                e.v3 = askNumber("  Frequency, Hz (> 0): ");
                entries.push_back(e);
                sourceCount++;
                std::cout << "  -> V1 added." << std::endl;
            }
            break;
        }
        case 5:
        {
            if (sourceCount >= 1)
            {
                std::cout << "A voltage source was already added. Only one is supported." << std::endl;
            }
            else
            {
                Entry e;
                e.type = 'D';
                e.name = "V1";
                std::cout << "Adding V1 (DC voltage source)" << std::endl;
                e.nodeA = askInt("  Positive node A (whole number): ");
                e.nodeB = askInt("  Negative node B (whole number, usually 0): ");
                e.v1 = askNumber("  Voltage in volts (negative flips polarity): ");
                entries.push_back(e);
                sourceCount++;
                std::cout << "  -> V1 added." << std::endl;
            }
            break;
        }
        case 0:
            done = true;
            break;
        default:
            std::cout << "Invalid choice. Please type a number from 0 to 5." << std::endl;
            break;
        }
    }

    // ============================================
    // Build the circuit from the collected entries
    // ============================================
    std::cout << "\n--- Building Circuit ---\n"
              << std::endl;

    // Number of nodes = highest node number used + 1 (node 0 is ground)
    int maxNode = 0;
    for (const auto &e : entries)
    {
        if (e.nodeA > maxNode)
        {
            maxNode = e.nodeA;
        }
        if (e.nodeB > maxNode)
        {
            maxNode = e.nodeB;
        }
    }

    Circuit myCircuit(maxNode + 1);

    for (const auto &e : entries)
    {
        switch (e.type)
        {
        case 'R':
            myCircuit.add<Resistor>(e.name, e.nodeA, e.nodeB, e.v1);
            break;
        case 'L':
            myCircuit.add<Inductor>(e.name, e.nodeA, e.nodeB, e.v1);
            break;
        case 'C':
            myCircuit.add<Capacitor>(e.name, e.nodeA, e.nodeB, e.v1);
            break;
        case 'A':
            myCircuit.add<ACVoltageSource>(e.name, e.nodeA, e.nodeB, e.v1, e.v2, e.v3);
            break;
        case 'D':
            myCircuit.add<DCVoltageSource>(e.name, e.nodeA, e.nodeB, e.v1);
            break;
        }
        std::cout << " Added " << e.name << " between nodes "
                  << e.nodeA << "-" << e.nodeB << std::endl;
    }

    // Validate circuit
    auto problems = myCircuit.validate();
    if (!problems.empty())
    {
        for (const auto &p : problems)
        {
            std::cout << "Problem: " << p << std::endl;
        }
        return 1;
    }
    if (!myCircuit.hasSource())
    {
        std::cout << "Problem: Circuit has no voltage source. Add one (menu option 4 or 5)." << std::endl;
        return 1;
    }

    const VoltageSource *source = findSource(myCircuit);
    double f = source->getFrequency();

    // ============================================
    // PART 1: Components and Topology
    // ============================================
    std::cout << "\n  PART 1: Circuit Components            " << std::endl;

    myCircuit.listComponents();

    std::cout << "\nVoltage Source Details:" << std::endl;
    std::cout << "  Name: " << source->getName() << std::endl;
    std::cout << "  Type: " << source->getType() << std::endl;
    std::cout << "  Symbol: " << source->getSymbol() << std::endl;
    std::cout << "  Positive terminal: node " << source->getNodeA() << std::endl;
    std::cout << "  Negative terminal: node " << source->getNodeB();
    if (source->getNodeB() == myCircuit.getGroundNode())
    {
        std::cout << " (ground)";
    }
    std::cout << std::endl;
    if (f == 0.0)
    {
        std::cout << "  Frequency: 0 Hz (DC)\n"
                  << std::endl;
    }
    else
    {
        std::cout << "  Frequency: " << engFormat(f) << "Hz\n"
                  << std::endl;
    }

    // ============================================
    // Solve the circuit (nodal analysis)
    // ============================================
    NodalAnalyzer analyzer(myCircuit);
    AnalysisResult result = analyzer.solve(f);
    const auto &nodeVoltages = result.nodeVoltages;

    std::complex<double> V_source = source->getSourceVoltage(f);

    // Branch current of a source flows A -> B inside the source,
    // so the current the source SUPPLIES is the negative of it.
    std::complex<double> I_source = -result.current(source->getName());

    // ============================================
    // PART 2: Impedance Analysis
    // ============================================
    std::cout << "  PART 2: Impedance Analysis            " << std::endl;
    std::cout << "  (Impedance seen by voltage source)    " << std::endl;

    std::cout << "--- Individual Component Impedances at " << engFormat(f) << "Hz ---\n"
              << std::endl;

    for (const auto &c : myCircuit.getComponents())
    {
        if (c->isSource())
        {
            continue;
        }
        std::cout << "  " << std::left << std::setw(5) << c->getName()
                  << std::setw(10) << c->getSymbol();
        printZ(c->getImpedance(f));
    }

    std::complex<double> Z_total;
    if (f > 0.0)
    {
        std::cout << "\n--- Circuit Impedance (using nodal analysis) ---\n"
                  << std::endl;
        Z_total = circuitImpedance(myCircuit, f);
    }
    else
    {
        std::cout << "\n--- Circuit Resistance at DC (V / I from nodal analysis) ---\n"
                  << std::endl;
        if (std::abs(I_source) < 1e-12)
        {
            Z_total = std::complex<double>(1e15, 0.0);
        }
        else
        {
            Z_total = V_source / I_source;
        }
    }
    std::cout << "  Z_total = ";
    printZ(Z_total);
    std::cout << std::endl;

    // ============================================
    // PART 3: Nodal Analysis - Node Voltages
    // ============================================
    std::cout << "  PART 3: Nodal Analysis                " << std::endl;
    std::cout << "  (Node Voltages at Frequency)          " << std::endl;

    std::cout << "\nNode Voltages:\n"
              << std::endl;
    for (size_t i = 0; i < nodeVoltages.size(); i++)
    {
        std::cout << "  V[" << i << "] = ";
        printVoltage(nodeVoltages[i]);
    }

    // ============================================
    // PART 4: Circuit Behavior Analysis
    // ============================================
    std::cout << "\n  PART 4: Circuit Behavior Analysis     " << std::endl;

    std::cout << "Source Analysis:" << std::endl;
    std::cout << "  Source voltage: ";
    printVoltage(V_source);
    std::cout << "  Source current: ";
    printCurrent(I_source);

    // Peak phasors need a factor of 1/2 for average power; DC does not
    double powerFactor = (f == 0.0) ? 1.0 : 0.5;
    std::complex<double> S = powerFactor * V_source * std::conj(I_source);

    std::cout << "  Real power delivered by source: " << std::fixed << std::setprecision(4)
              << S.real() << " W" << std::endl;
    std::cout << "  Reactive power (imaginary): " << S.imag() << " VAR" << std::endl;

    std::cout << "\nVoltage Drops and Currents Across Components:" << std::endl;
    for (const auto &b : result.branches)
    {
        if (b.type == "AC" || b.type == "DC")
        {
            continue;
        }
        std::cout << "\n  " << b.name << " (nodes " << b.nodeA << "-" << b.nodeB << ")" << std::endl;
        std::cout << "    Voltage: ";
        printVoltage(b.voltage);
        std::cout << "    Current: ";
        printCurrent(b.current);
    }

    // ============================================
    // PART 5: Verification
    // ============================================
    std::cout << "\n  PART 5: Verification                  " << std::endl;

    // Kirchhoff's Current Law at every node except ground
    std::cout << "Kirchhoff's Current Law Check (net current leaving each node):" << std::endl;
    bool allPass = true;
    for (int n = 1; n < myCircuit.getNumNodes(); n++)
    {
        std::complex<double> net(0.0, 0.0);
        for (const auto &b : result.branches)
        {
            if (b.nodeA == n)
            {
                net += b.current;
            }
            if (b.nodeB == n)
            {
                net -= b.current;
            }
        }
        double err = std::abs(net);
        std::cout << "  Node " << n << ": error = " << std::fixed << std::setprecision(8)
                  << err << " A   ";
        if (err < 1e-6)
        {
            std::cout << "PASS" << std::endl;
        }
        else
        {
            std::cout << "FAIL" << std::endl;
            allPass = false;
        }
    }
    if (allPass)
    {
        std::cout << "  KCL is satisfied at all nodes." << std::endl;
    }
    else
    {
        std::cout << "  KCL violation found." << std::endl;
    }

    // Source constraint: V_source = V(nodeA) - V(nodeB)
    std::complex<double> V_source_check = nodeVoltages[source->getNodeA()] - nodeVoltages[source->getNodeB()];
    double source_error = std::abs(V_source - V_source_check);

    std::cout << "\nSource Voltage Constraint Check:" << std::endl;
    std::cout << "  Expected: ";
    printVoltage(V_source);
    std::cout << "  Calculated from nodes: ";
    printVoltage(V_source_check);
    std::cout << "  Error: " << std::fixed << std::setprecision(8) << source_error << " V" << std::endl;
    if (source_error < 1e-6)
    {
        std::cout << "  PASS: Source voltage constraint satisfied" << std::endl;
    }
    else
    {
        std::cout << "  FAIL: Source voltage mismatch" << std::endl;
    }

    // Power balance: power supplied by the source = power absorbed by the rest
    double balance = std::abs(result.totalPower());
    std::cout << "\nPower Balance Check (sum of all component powers):" << std::endl;
    std::cout << "  Net power: " << std::fixed << std::setprecision(8) << balance << " W" << std::endl;
    if (balance < 1e-6)
    {
        std::cout << "  PASS: Power supplied equals power absorbed" << std::endl;
    }
    else
    {
        std::cout << "  FAIL: Power mismatch" << std::endl;
    }

    std::cout << "\n   ANALYSIS COMPLETE      " << std::endl;

    return 0;
}