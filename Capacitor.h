#ifndef CAPACITOR_H
#define CAPACITOR_H

#include "Component.h"
using namespace std;

class Capacitor : public Component {
private:
    double capacitance;

public:
    Capacitor(string n, int a, int b, double c): Component(move(n), a, b), capacitance(c) {
        if (c <= 0) {
            throw invalid_argument("Capacitance must be positive");
        }
    }

    complex<double> getImpedance(double frequency) const override {
        if (frequency == 0.0) {
            return complex<double>(0.0, 1e15);
        }
        double omega = 2.0 * M_PI * frequency;
        double imaginaryPart = -1.0 / (omega * capacitance);
        return complex<double>(0.0, imaginaryPart);
    }

    double getValue() const override { return capacitance; }
    string getType() const override { return "C"; }
    bool isSource() const override { return false; }
};

#endif