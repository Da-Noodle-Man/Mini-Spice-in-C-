#ifndef RESISTOR_H
#define RESISTOR_H

#include "Component.h"
using namespace std;

class Resistor:public Component {
private:
    double resistance;

public:
    Resistor(string n, int a, int b, double r): Component(move(n), a, b), resistance(r) {
        if (r <= 0) {
            throw invalid_argument("Resistance must be positive");
        }
    }

    complex<double> getImpedance(double frequency) const override {
        (void)frequency;
        return complex<double>(resistance, 0.0);
    }

    double getValue() const override { return resistance; }
    string getType() const override { return "R"; }
    bool isSource() const override { return false; }
};

#endif
