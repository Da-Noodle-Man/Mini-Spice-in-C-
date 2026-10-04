#ifndef VOLTAGESOURCE_H
#define VOLTAGESOURCE_H

#include "Component.h"

// ============================================
// VoltageSource Base Class
// ============================================
// Base class for all voltage sources (AC and DC)
// Subclasses must implement getSourceVoltage() and getFrequency()

class VoltageSource : public Component {
public:
    VoltageSource(std::string n, int a, int b)
        : Component(std::move(n), a, b) {}

    virtual ~VoltageSource() = default;

    // Impedance is always 0 for ideal voltage source
    std::complex<double> getImpedance(double frequency) const override {
        (void)frequency;
        return std::complex<double>(0.0, 0.0);
    }

    // Check if this is a source
    bool isSource() const override { return true; }

    // Get the source voltage as a phasor at given frequency
    virtual std::complex<double> getSourceVoltage(double frequency) const = 0;

    // Get the frequency for this source (0 for DC)
    virtual double getFrequency() const = 0;

    // Get symbol representation
    virtual std::string getSymbol(double frequency = -1.0) const = 0;
};

#endif // VOLTAGESOURCE_H
