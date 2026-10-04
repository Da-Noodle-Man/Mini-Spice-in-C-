#ifndef ACVOLTAGESOURCE_H
#define ACVOLTAGESOURCE_H

#include "VoltageSource.h"
#include <sstream>
using namespace std;

class ACVoltageSource : public VoltageSource {
private:
    double amplitude;  // Peak volts
    double phaseDeg;   // Degrees
    double frequency;  // Hz

public:
    ACVoltageSource(string n, int a, int b, double amp, double phase_deg, double freq_hz)
        : VoltageSource(move(n), a, b), amplitude(amp), phaseDeg(phase_deg), frequency(freq_hz) {
        if (!isfinite(amp) || amp < 0)
            throw invalid_argument("AC amplitude must be non-negative");
        if (!isfinite(phase_deg))
            throw invalid_argument("AC phase must be a finite number");
        if (!isfinite(freq_hz) || freq_hz <= 0)
            throw invalid_argument("AC frequency must be positive (Hz)");
    }

    double getValue() const override { return amplitude; }
    double getPhaseDeg() const { return phaseDeg; }
    double getFrequency() const override { return frequency; }
    string getType() const override { return "AC"; }

    string getSymbol(double = -1.0) const override {
        ostringstream os;
        os << "AC(" << engFormat(amplitude) << "," << phaseDeg << ")";
        return os.str();
    }

    // Returns phasor at source's own frequency, zero at all others
    complex<double> getSourceVoltage(double f) const override {
        if (f <= 0.0 || abs(f - frequency) > 1e-9 * frequency)
            return complex<double>(0.0, 0.0);
        return polar(amplitude, phaseDeg * M_PI / 180.0);
    }
};

#endif // ACVOLTAGESOURCE_H
