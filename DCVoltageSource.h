#ifndef DCVOLTAGESOURCE_H
#define DCVOLTAGESOURCE_H

#include "VoltageSource.h"

// DC voltage source. Only the voltage is needed. Diagram symbol: DC(amp)
// A negative value just flips the polarity.
class DCVoltageSource : public VoltageSource {
private:
    double voltage;  // Volts

public:
    DCVoltageSource(std::string n, int a, int b, double v)
        : VoltageSource(std::move(n), a, b), voltage(v) {
        if (!std::isfinite(v)) {
            throw std::invalid_argument("DC voltage must be a finite number");
        }
    }

    double getValue() const override { return voltage; }
    double getFrequency() const override { return 0.0; }

    std::string getType() const override { return "DC"; }

    std::string getSymbol(double /*frequency*/ = -1.0) const override {
        return "DC(" + engFormat(voltage) + ")";
    }

    std::complex<double> getSourceVoltage(double frequency) const override {
        return frequency == 0.0 ? std::complex<double>(voltage, 0.0)
                                : std::complex<double>(0.0, 0.0);
    }
};

#endif // DCVOLTAGESOURCE_H
