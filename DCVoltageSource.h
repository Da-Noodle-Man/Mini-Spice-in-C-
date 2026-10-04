#ifndef DCVOLTAGESOURCE_H
#define DCVOLTAGESOURCE_H

#include "VoltageSource.h"

class DCVoltageSource : public VoltageSource {
private:
    double voltage;  // Volts

public:
    DCVoltageSource(string n, int a, int b, double v)
        : VoltageSource(move(n), a, b), voltage(v) {
        if (!isfinite(v))
            throw invalid_argument("DC voltage must be a finite number");
    }

    double getValue() const override { return voltage; }
    double getFrequency() const override { return 0.0; }
    string getType() const override { return "DC"; }

    string getSymbol(double = -1.0) const override {
        return "DC(" + engFormat(voltage) + ")";
    }

    complex<double> getSourceVoltage(double frequency) const override {
        return frequency == 0.0 ? complex<double>(voltage, 0.0)   // Returns voltage at DC f=0
                                : complex<double>(0.0, 0.0);    // 0 at any other freq 
    }
};

#endif // DCVOLTAGESOURCE_H
