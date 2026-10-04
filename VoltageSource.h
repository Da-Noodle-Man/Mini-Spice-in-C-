#ifndef VOLTAGESOURCE_H
#define VOLTAGESOURCE_H

#include "Component.h"
using namespace std;

class VoltageSource : public Component {
public:
    VoltageSource(string n, int a, int b) : Component(move(n), a, b) {}
    virtual ~VoltageSource() = default;

    complex<double> getImpedance(double frequency) const override {
        return complex<double>(0.0, 0.0);
    }

    bool isSource() const override { return true; }

    virtual complex<double> getSourceVoltage(double frequency) const = 0;
    virtual double getFrequency() const = 0;
    virtual string getSymbol(double frequency = -1.0) const = 0;
};

#endif // VOLTAGESOURCE_H
