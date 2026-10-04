#ifndef ACVOLTAGESOURCE_H
#define ACVOLTAGESOURCE_H

#include "VoltageSource.h"

// Sinusoidal AC voltage source. Diagram symbol: AC(amp,phase)
//   amplitude = peak voltage in Volts (must be >= 0)
//   phase     = phase shift in DEGREES (any sign)
//   frequency = Hz (must be > 0), chosen by the user
//
// As a phasor the source is  amplitude at angle phase  (peak value).
class ACVoltageSource : public VoltageSource {
private:
    double amplitude;  // Peak volts
    double phaseDeg;   // Degrees
    double frequency;  // Hz

public:
    ACVoltageSource(std::string n, int a, int b,
                    double amp, double phase_deg, double freq_hz)
        : VoltageSource(std::move(n), a, b),
          amplitude(amp), phaseDeg(phase_deg), frequency(freq_hz) {
        if (!std::isfinite(amp) || amp < 0) {
            throw std::invalid_argument(
                "AC amplitude must be non-negative (use phase = 180 to invert)");
        }
        if (!std::isfinite(phase_deg)) {
            throw std::invalid_argument("AC phase must be a finite number");
        }
        if (!std::isfinite(freq_hz) || freq_hz <= 0) {
            throw std::invalid_argument("AC frequency must be positive (Hz)");
        }
    }

    double getValue() const override { return amplitude; }
    double getPhaseDeg() const { return phaseDeg; }
    double getFrequency() const override { return frequency; }

    std::string getType() const override { return "AC"; }

    std::string getSymbol(double /*frequency*/ = -1.0) const override {
        std::ostringstream os;
        os << "AC(" << engFormat(amplitude) << "," << phaseDeg << ")";
        return os.str();
    }

    // Phasor at the source's own frequency; zero at any other frequency
    std::complex<double> getSourceVoltage(double f) const override {
        if (f <= 0.0 || std::abs(f - frequency) > 1e-9 * frequency)
            return std::complex<double>(0.0, 0.0);
        return std::polar(amplitude, phaseDeg * M_PI / 180.0);
    }
};

#endif // ACVOLTAGESOURCE_H
