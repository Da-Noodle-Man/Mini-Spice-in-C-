#ifndef CIRCUIT_H
#define CIRCUIT_H

#include "Component.h"
#include <vector>
#include <memory>
#include <iostream>
#include <set>
#include <string>

// ============================================
// Circuit Class
// ============================================
// A container that holds components and checks they are wired sensibly.
// Nodes are numbered 0..N-1, and node 0 is ground.
// (The maths lives in Analysis.h, to keep this file small.)

class Circuit {
private:
    std::vector<std::unique_ptr<Component>> components;
    int numNodes;
    int groundNode;

public:
    Circuit(int nodes) : numNodes(nodes), groundNode(0) {
        if (nodes < 2)
            throw std::invalid_argument("Circuit must have at least 2 nodes");
    }

    // Add a component (checks nodes are valid and the name is unique)
    void addComponent(std::unique_ptr<Component> comp) {
        int a = comp->getNodeA(), b = comp->getNodeB();
        if (a < 0 || a >= numNodes || b < 0 || b >= numNodes)
            throw std::out_of_range("Component '" + comp->getName() + "' nodes out of range");
        if (a == b)
            throw std::invalid_argument("Component '" + comp->getName() + "' has both ends on one node");
        for (const auto& c : components)
            if (c->getName() == comp->getName())
                throw std::invalid_argument("Duplicate component name: " + comp->getName());
        components.push_back(std::move(comp));
    }

    int getNumNodes() const { return numNodes; }
    int getGroundNode() const { return groundNode; }
    const auto& getComponents() const { return components; }

    // Returns a list of problems (empty = fine):
    // every node must be reachable from ground through the components.
    std::vector<std::string> validate() const {
        std::set<int> reached = {groundNode};
        for (bool grew = true; grew;) {
            grew = false;
            for (const auto& c : components) {
                bool a = reached.count(c->getNodeA()) > 0;
                bool b = reached.count(c->getNodeB()) > 0;
                if (a != b) {
                    reached.insert(a ? c->getNodeB() : c->getNodeA());
                    grew = true;
                }
            }
        }
        std::vector<std::string> problems;
        for (int n = 0; n < numNodes; ++n)
            if (!reached.count(n))
                problems.push_back("Node " + std::to_string(n) + " is not connected to ground");
        return problems;
    }

    // Print every component with the two nodes it connects
    void listComponents() const {
        std::cout << "Nodes: " << numNodes << " (node " << groundNode << " = ground)\n" << std::endl;
        for (const auto& c : components)
            std::cout << "  " << std::left << std::setw(5) << c->getName()
                      << std::setw(12) << c->getSymbol()
                      << "node " << c->getNodeA() << " - node " << c->getNodeB() << std::endl;
        std::cout << std::endl;
    }
};

#endif // CIRCUIT_H
