#ifndef CAPACITOR_H
#define CAPACITOR_H

#include "Component.h"
using namespace std;

class Capacitor : public Component {
private:
    double capacitance;

public:
    Capacitor(string n, int a, int b, double c): Component(n, a, b), capacitance(c) {}

    complex<double> getImpedance(double freq) const override {
        if (freq == 0.0) {
            return complex<double>(0.0, 1e15);
        }
        double omega = 2.0 * M_PI * freq;
        double img = -1.0 / (omega * capacitance);
        return complex<double>(0.0, img);
    }

    double getValue() const override { return capacitance; }
    string getType() const override { return "C"; }
    bool isSource() const override { return false; }
};

#endif