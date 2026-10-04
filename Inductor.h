#ifndef INDUCTOR_H
#define INDUCTOR_H

#include "Component.h"
using namespace std;

class Inductor:public Component {
private:
    double inductance;

public:
    Inductor(string n, int a, int b, double l): Component(move(n), a, b), inductance(l) {
        if (l <= 0) {
            throw invalid_argument("Inductance must be positive");
        }
    }

    complex<double> getImpedance(double frequency) const override {
        if (frequency == 0.0) {
            return complex<double>(0.0, 0.0);
        }
        double omega = 2.0 * M_PI * frequency;
        double imaginaryPart = omega * inductance;
        return complex<double>(0.0, imaginaryPart);
    }

    double getValue() const override { return inductance; }
    string getType() const override { return "L"; }
    bool isSource() const override { return false; }
};

#endif
