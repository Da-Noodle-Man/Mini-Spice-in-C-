#ifndef INDUCTOR_H
#define INDUCTOR_H

#include "Component.h"
using namespace std;

class Inductor:public Component {
private:
    double inductance;

public:
    Inductor(string n, int a, int b, double l): Component(n, a, b), inductance(l) {}
    complex<double> getImpedance(double freq) const override {
        if (freq == 0.0) {
            return complex<double>(0.0, 0.0);
        }
        double omega = 2.0 * M_PI * freq;
        double img = omega * inductance;
        return complex<double>(0.0, img);
    }
    double getValue() const override { return inductance; }
    string getType() const override { return "L"; }
    bool isSource() const override { return false; }
};

#endif
