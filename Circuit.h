#ifndef CIRCUIT_H
#define CIRCUIT_H

#include "Component.h"
#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <iomanip>
using namespace std;
// Holds the components of a circuit. Node 0 is ground.
class Circuit
{
private:
    vector<unique_ptr<Component>> components;
    int numNodes;
    int groundNode;

public:
    Circuit(int nodes) : numNodes(nodes), groundNode(0) {}

    // Create a component and add it, e.g. circuit.add<Resistor>("R1", 1, 2, 1000.0);
    template <typename T, typename... Args>
    void add(Args &&...args)
    {
        components.push_back(make_unique<T>(forward<Args>(args)...));
    }

    int getNumNodes() const { return numNodes; }
    int getGroundNode() const { return groundNode; }

    const vector<unique_ptr<Component>> &getComponents() const
    {
        return components;
    }

    bool hasSource() const
    {
        for (const auto &c : components)
        {
            if (c->isSource())
            {
                return true;
            }
        }
        return false;
    }

    // Returns a list of problems (empty if the circuit is fine)
    vector<string> validate() const
    {
        vector<string> problems;

        if (components.empty())
        {
            problems.push_back("Circuit has no components");
        }

        int sources = 0;
        for (const auto &c : components)
        {
            if (c->isSource())
            {
                sources++;
            }
        }
        if (sources > 1)
        {
            problems.push_back("Only one voltage source is supported");
        }

        return problems;
    }

    void listComponents() const
    {
        cout << left << setw(8) << "Name"
             << setw(6) << "Type"
             << setw(10) << "Value"
             << setw(12) << "Nodes"
             << setw(20) << "Symbol" << endl;
        cout << string(56, '-') << endl;

        for (const auto &c : components)
        {
            string nodes = to_string(c->getNodeA()) + "-" + to_string(c->getNodeB());
            cout << left << setw(8) << c->getName()
                 << setw(6) << c->getType()
                 << setw(10) << engFormat(c->getValue())
                 << setw(12) << nodes
                 << setw(20) << c->getSymbol() << endl;
        }
    }
};

#endif // CIRCUIT_H