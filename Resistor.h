#ifndef RESISTOR_H
#define RESISTOR_H

#include "Component.h"
using namespace std;

class Resistor:public Component {
private:
    double resistance;
public:
    Resistor(string n, int a, int b, double r): Component(n, a, b), resistance(r) {}
    complex<double> getImpedance(double freq) const override {
        (void)freq;
        return complex<double>(resistance, 0.0);
    }
    double getValue() const override { return resistance; }
    string getType() const override { return "R"; }
    bool isSource() const override { return false; }
};

#endif
