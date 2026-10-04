#ifndef RESISTOR_H
#define RESISTOR_H

#include "Component.h"

class Resistor : public Component {
private:
    double resistance;

public:
    Resistor(std::string n, int a, int b, double r)
        : Component(std::move(n), a, b), resistance(r) {
        if (r <= 0) {
            throw std::invalid_argument("Resistance must be positive");
        }
    }

    std::complex<double> getImpedance(double frequency) const override {
        (void)frequency;  // Resistor is frequency-independent
        return std::complex<double>(resistance, 0.0);
    }

    double getValue() const override { return resistance; }

    std::string getType() const override { return "R"; }

    bool isSource() const override { return false; }
};

#endif // RESISTOR_H
