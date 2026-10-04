#ifndef CIRCUIT_H
#define CIRCUIT_H

#include "Component.h"
#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <iomanip>
using namespace std;

class Circuit
{
private:
    vector<unique_ptr<Component>> components;
    int numNodes;
    int groundNode;

public:
    Circuit(int nodes) : numNodes(nodes), groundNode(0) {}

    void add(unique_ptr<Component> c)
    {
        components.push_back(move(c));
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

    void listComponents() const
    {
        cout << left << setw(8) << "Name" << setw(6) << "Type" << setw(10) << "Value" << setw(12) << "Nodes" << setw(20) << "Symbol" << endl;
        cout << string(56, '-') << endl;

        for (const auto &c : components)
        {
            string nodes = to_string(c->getNodeA()) + "-" + to_string(c->getNodeB());
            cout << left << setw(8) << c->getName() << setw(6) << c->getType() << setw(10) << engFormat(c->getValue()) << setw(12) << nodes << setw(20) << c->getSymbol() << endl;
        }
    }
};

#endif