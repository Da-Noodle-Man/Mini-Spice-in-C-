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
using namespace std;

struct Entry
{
    char type = 'R';
    string name;
    int nodeA = 0;
    int nodeB = 0;
    double v1 = 0.0;
    double v2 = 0.0;
    double v3 = 0.0;
};


double askNumber(const string &prompt)
{
    double v = 0.0;
    cout << prompt;
    cin >> v;
    return v;
}

int askInt(const string &prompt)
{
    int v = 0;
    cout << prompt;
    cin >> v;
    return v;
}

void printInstructions()
{
    cout << "==================================================" << endl;
    cout << "   MiniSPICE - Build Your Own Circuit             " << endl;
    cout << "==================================================" << endl;
    cout << "HOW TO ENTER DATA:" << endl;
    cout << "  1. Pick a component from the menu by typing its number" << endl;
    cout << "     and pressing Enter. Repeat until you are done, then" << endl;
    cout << "     type 0 to finish and analyse the circuit." << endl;
    cout << "  2. NODES are whole numbers. Node 0 is GROUND." << endl;
    cout << "     Number your nodes 0, 1, 2, 3, ... with no gaps." << endl;
    cout << "  3. Every component sits between two nodes (Node A and Node B)." << endl;
    cout << "  4. Type values in BASE UNITS (no k, m, u suffixes)." << endl;
    cout << "     Scientific notation is allowed:" << endl;
    cout << "       1 kohm   -> 1000   or 1e3" << endl;
    cout << "       10 mH    -> 0.01   or 1e-2" << endl;
    cout << "       1 uF     -> 0.000001 or 1e-6" << endl;
    cout << "  5. Resistor, inductor and capacitor values must be > 0." << endl;
    cout << "  6. Exactly ONE voltage source (AC or DC) is required." << endl;
    cout << "     Node A is its POSITIVE terminal, Node B its NEGATIVE one" << endl;
    cout << "     (usually ground, node 0)." << endl;
    cout << "  7. AC source: amplitude = peak volts (>= 0)," << endl;
    cout << "     phase = degrees, frequency = Hz (> 0)." << endl;
    cout << "  Example: V1 between 1 and 0, R1 between 1 and 2," << endl;
    cout << "           C1 between 2 and 0." << endl;
    cout << "==================================================\n"
              << endl;
}
void printMenu()
{
    cout << "\n------------- COMPONENT MENU -------------" << endl;
    cout << "  1 - Add Resistor" << endl;
    cout << "  2 - Add Inductor" << endl;
    cout << "  3 - Add Capacitor" << endl;
    cout << "  4 - Add AC Voltage Source" << endl;
    cout << "  5 - Add DC Voltage Source" << endl;
    cout << "  0 - Finish and analyse circuit" << endl;
    cout << "------------------------------------------" << endl;
}

void printZ(complex<double> Z)
{
    cout << fixed << setprecision(2);
    if (abs(Z) > 1e12)
    {
        cout << "infinite (open circuit)" << endl;
        return;
    }
    cout << Z.real() << (Z.imag() < 0 ? " - j" : " + j")
              << abs(Z.imag()) << " ohm"
              << "   |Z| = " << abs(Z) << " ohm"
              << "   phase = " << arg(Z) * 180.0 / M_PI << " deg" << endl;
}

void printVoltage(complex<double> V)
{
    cout << fixed << setprecision(4);
    cout << V.real() << (V.imag() < 0 ? " - j" : " + j")
              << abs(V.imag()) << " V"
              << "   |V| = " << abs(V) << " V"
              << "   phase = " << arg(V) * 180.0 / M_PI << " deg" << endl;
}

void printCurrent(complex<double> I)
{
    cout << fixed << setprecision(6);
    cout << I.real() << (I.imag() < 0 ? " - j" : " + j")
              << abs(I.imag()) << " A"
              << "   |I| = " << abs(I) << " A"
              << "   phase = " << arg(I) * 180.0 / M_PI << " deg" << endl;
}

int main()
{
    printInstructions();
    vector<Entry> entries;
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
            e.name = "R" + to_string(resCount);
            cout << "Adding " << e.name << " (resistor)" << endl;
            e.nodeA = askInt("  Node A (whole number): ");
            e.nodeB = askInt("  Node B (whole number): ");
            e.v1 = askNumber("  Resistance in ohms (> 0, e.g. 1000): ");
            entries.push_back(e);
            cout << "  -> " << e.name << " added." << endl;
            break;
        }
        case 2:
        {
            Entry e;
            e.type = 'L';
            indCount++;
            e.name = "L" + to_string(indCount);
            cout << "Adding " << e.name << " (inductor)" << endl;
            e.nodeA = askInt("  Node A (whole number): ");
            e.nodeB = askInt("  Node B (whole number): ");
            e.v1 = askNumber("  Inductance in henries (> 0, e.g. 0.01 for 10 mH): ");
            entries.push_back(e);
            cout << "  -> " << e.name << " added." << endl;
            break;
        }
        case 3:
        {
            Entry e;
            e.type = 'C';
            capCount++;
            e.name = "C" + to_string(capCount);
            cout << "Adding " << e.name << " (capacitor)" << endl;
            e.nodeA = askInt("  Node A (whole number): ");
            e.nodeB = askInt("  Node B (whole number): ");
            e.v1 = askNumber("  Capacitance in farads (> 0, e.g. 1e-6 for 1 uF): ");
            entries.push_back(e);
            cout << "  -> " << e.name << " added." << endl;
            break;
        }
        case 4:
        {
            if (sourceCount >= 1)
            {
                cout << "A voltage source was already added. Only one is supported." << endl;
            }
            else
            {
                Entry e;
                e.type = 'A';
                e.name = "V1";
                cout << "Adding V1 (AC voltage source)" << endl;
                e.nodeA = askInt("  Positive node A (whole number): ");
                e.nodeB = askInt("  Negative node B (whole number, usually 0): ");
                e.v1 = askNumber("  Amplitude, peak volts (>= 0): ");
                e.v2 = askNumber("  Phase shift, degrees (any number, e.g. 0): ");
                e.v3 = askNumber("  Frequency, Hz (> 0): ");
                entries.push_back(e);
                sourceCount++;
                cout << "  -> V1 added." << endl;
            }
            break;
        }
        case 5:
        {
            if (sourceCount >= 1)
            {
                cout << "A voltage source was already added. Only one is supported." << endl;
            }
            else
            {
                Entry e;
                e.type = 'D';
                e.name = "V1";
                cout << "Adding V1 (DC voltage source)" << endl;
                e.nodeA = askInt("  Positive node A (whole number): ");
                e.nodeB = askInt("  Negative node B (whole number, usually 0): ");
                e.v1 = askNumber("  Voltage in volts (negative flips polarity): ");
                entries.push_back(e);
                sourceCount++;
                cout << "  -> V1 added." << endl;
            }
            break;
        }
        case 0:
            done = true;
            break;
        default:
            cout << "Invalid choice. Please type a number from 0 to 5." << endl;
            break;
        }
    }

    cout << "\n--- Building Circuit ---\n"<< endl;

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
            myCircuit.add(make_unique<Resistor>(e.name, e.nodeA, e.nodeB, e.v1));
            break;
        case 'L':
            myCircuit.add(make_unique<Inductor>(e.name, e.nodeA, e.nodeB, e.v1));
            break;
        case 'C':
            myCircuit.add(make_unique<Capacitor>(e.name, e.nodeA, e.nodeB, e.v1));
            break;
        case 'A':
            myCircuit.add(make_unique<ACVoltageSource>(e.name, e.nodeA, e.nodeB, e.v1, e.v2, e.v3));
            break;
        case 'D':
            myCircuit.add(make_unique<DCVoltageSource>(e.name, e.nodeA, e.nodeB, e.v1));
            break;
        }
        cout << " Added " << e.name << " between nodes "
                  << e.nodeA << "-" << e.nodeB << endl;
    }

    if (!myCircuit.hasSource())
    {
        cout << "Problem: Circuit has no voltage source. Add one (menu option 4 or 5)." << endl;
        return 1;
    }

    const VoltageSource *source = findSource(myCircuit);
    double f = source->getFrequency();

    cout << "\n  PART 1: Circuit Components            " << endl;

    myCircuit.listComponents();

    cout << "\nVoltage Source Details:" << endl;
    cout << "  Name: " << source->getName() << endl;
    cout << "  Type: " << source->getType() << endl;
    cout << "  Symbol: " << source->getSymbol() << endl;
    cout << "  Positive terminal: node " << source->getNodeA() << endl;
    cout << "  Negative terminal: node " << source->getNodeB();
    if (source->getNodeB() == myCircuit.getGroundNode())
    {
        cout << " (ground)";
    }
    cout << endl;
    if (f == 0.0)
    {
        cout << "  Frequency: 0 Hz (DC)\n"
                  << endl;
    }
    else
    {
        cout << "  Frequency: " << engFormat(f) << "Hz\n"
                  << endl;
    }
    NodalAnalyzer analyzer(myCircuit);
    AnalysisResult result = analyzer.solve(f);
    const auto &nodeVoltages = result.nodeVoltages;

    complex<double> V_source = source->getSourceVoltage(f);

    complex<double> I_source = -result.current(source->getName());

    cout << "  PART 2: Impedance Analysis            " << endl;
    cout << "  (Impedance seen by voltage source)    " << endl;

    cout << "--- Individual Component Impedances at " << engFormat(f) << "Hz ---\n"
              << endl;

    for (const auto &c : myCircuit.getComponents())
    {
        if (c->isSource())
        {
            continue;
        }
        cout << "  " << left << setw(5) << c->getName()
                  << setw(10) << c->getSymbol();
        printZ(c->getImpedance(f));
    }

    complex<double> Z_total;
    if (f > 0.0)
    {
        cout << "\n--- Circuit Impedance (using nodal analysis) ---\n"
                  << endl;
        Z_total = circuitImpedance(myCircuit, f);
    }
    else
    {
        cout << "\n--- Circuit Resistance at DC (V / I from nodal analysis) ---\n"
                  << endl;
        if (abs(I_source) < 1e-12)
        {
            Z_total = complex<double>(1e15, 0.0);
        }
        else
        {
            Z_total = V_source / I_source;
        }
    }
    cout << "  Z_total = ";
    printZ(Z_total);
    cout << endl;

    cout << "  PART 3: Nodal Analysis                " << endl;
    cout << "  (Node Voltages at Frequency)          " << endl;

    cout << "\nNode Voltages:\n"
              << endl;
    for (size_t i = 0; i < nodeVoltages.size(); i++)
    {
        cout << "  V[" << i << "] = ";
        printVoltage(nodeVoltages[i]);
    }

    cout << "\n  PART 4: Circuit Behavior Analysis     " << endl;

    cout << "Source Analysis:" << endl;
    cout << "  Source voltage: ";
    printVoltage(V_source);
    cout << "  Source current: ";
    printCurrent(I_source);

    double powerFactor = (f == 0.0) ? 1.0 : 0.5;
    complex<double> S = powerFactor * V_source * conj(I_source);

    cout << "  Real power delivered by source: " << fixed << setprecision(4)
              << S.real() << " W" << endl;
    cout << "  Reactive power (imaginary): " << S.imag() << " VAR" << endl;

    cout << "\nVoltage Drops and Currents Across Components:" << endl;
    for (const auto &b : result.branches)
    {
        if (b.type == "AC" || b.type == "DC")
        {
            continue;
        }
        cout << "\n  " << b.name << " (nodes " << b.nodeA << "-" << b.nodeB << ")" << endl;
        cout << "    Voltage: ";
        printVoltage(b.voltage);
        cout << "    Current: ";
        printCurrent(b.current);
    }

    cout << "\n   ANALYSIS COMPLETE      " << endl;

    return 0;
}