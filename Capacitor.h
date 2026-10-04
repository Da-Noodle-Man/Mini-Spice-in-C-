#ifndef CAPACITOR_H
#define CAPACITOR_H

#include "Component.h"

class Capacitor : public Component {
private:
    double capacitance;

public:
    Capacitor(std::string n, int a, int b, double c)
        : Component(std::move(n), a, b), capacitance(c) {
        if (c <= 0) {
            throw std::invalid_argument("Capacitance must be positive");
        }
    }

    std::complex<double> getImpedance(double frequency) const override {
        if (frequency == 0.0) {
            return std::complex<double>(0.0, 1e15);
        }
        double omega = 2.0 * M_PI * frequency;
        double imaginaryPart = -1.0 / (omega * capacitance);
        return std::complex<double>(0.0, imaginaryPart);
    }

    double getValue() const override { return capacitance; }

    std::string getType() const override { return "C"; }

    bool isSource() const override { return false; }
};

#endif // CAPACITOR_H
