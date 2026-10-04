#ifndef INDUCTOR_H
#define INDUCTOR_H

#include "Component.h"

class Inductor : public Component {
private:
    double inductance;

public:
    Inductor(std::string n, int a, int b, double l)
        : Component(std::move(n), a, b), inductance(l) {
        if (l <= 0) {
            throw std::invalid_argument("Inductance must be positive");
        }
    }

    std::complex<double> getImpedance(double frequency) const override {
        if (frequency == 0.0) {
            return std::complex<double>(0.0, 0.0);
        }
        double omega = 2.0 * M_PI * frequency;
        double imaginaryPart = omega * inductance;
        return std::complex<double>(0.0, imaginaryPart);
    }

    double getValue() const override { return inductance; }

    std::string getType() const override { return "L"; }

    bool isSource() const override { return false; }
};

#endif // INDUCTOR_H
