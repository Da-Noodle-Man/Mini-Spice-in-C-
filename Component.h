#ifndef COMPONENT_H
#define COMPONENT_H

#include <string>
#include <complex>
#include <cmath>
#include <sstream>
#include <iomanip>

// Engineering notation helper
inline std::string engFormat(double value) {
    if (value == 0.0) return "0";
    if (!std::isfinite(value)) return "inf";
    
    double absVal = std::abs(value);
    const char* suffixes[] = {"", "k", "M", "G", "T", "m", "u", "n", "p"};
    const double scales[] = {1e0, 1e3, 1e6, 1e9, 1e12, 1e-3, 1e-6, 1e-9, 1e-12};
    
    int idx = 0;
    double scaled = absVal;
    
    // Find appropriate scale (positive exponents first)
    if (absVal >= 1.0) {
        while (idx < 4 && scaled >= 1000.0) {
            scaled /= 1000.0;
            idx++;
        }
    } else {
        idx = 4;  // Start with 'm'
        while (idx < 9 && scaled < 1.0) {
            scaled *= 1000.0;
            idx++;
        }
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << (value < 0 ? -scaled : scaled) << suffixes[idx];
    return oss.str();
}

// ============================================
// Component Base Class
// ============================================
// All electrical components inherit from this class
// Defines interface for impedance calculation and component properties

class Component {
protected:
    std::string name;
    int nodeA, nodeB;

public:
    Component(std::string n, int a, int b)
        : name(std::move(n)), nodeA(a), nodeB(b) {}

    virtual ~Component() = default;

    // Get the impedance at a given frequency
    virtual std::complex<double> getImpedance(double frequency) const = 0;

    // Get the component's base value
    virtual double getValue() const = 0;

    // Get the component type
    virtual std::string getType() const = 0;

    // Get component name
    virtual std::string getName() const { return name; }

    // Check if this is a source (voltage/current)
    virtual bool isSource() const { return false; }

    // Get symbol representation
    virtual std::string getSymbol(double frequency = -1.0) const {
        return getName() + "(" + engFormat(getValue()) + ")";
    }

    // Node accessors
    int getNodeA() const { return nodeA; }
    int getNodeB() const { return nodeB; }
};

#endif // COMPONENT_H
