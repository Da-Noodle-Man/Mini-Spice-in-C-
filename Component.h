#ifndef COMPONENT_H
#define COMPONENT_H

#include <string>
#include <complex>
#include <cmath>
#include <sstream>
#include <iomanip>
using namespace std;

inline string engFormat(double value) {
    if (value == 0.0) return "0";
    if (!isfinite(value)) return "inf";
    
    double absVal = abs(value);
    const char* suffixes[] = {"", "k", "M", "G", "T", "m", "u", "n", "p"};
    const double scales[] = {1e0, 1e3, 1e6, 1e9, 1e12, 1e-3, 1e-6, 1e-9, 1e-12};
    
    int idx = 0;
    double scaled = absVal;
    
    if (absVal >= 1.0) {
        while (idx < 4 && scaled >= 1000.0) {
            scaled /= 1000.0;
            idx++;
        }
    } else {
        idx = 4;
        while (idx < 9 && scaled < 1.0) {
            scaled *= 1000.0;
            idx++;
        }
    }
    
    ostringstream oss;
    oss << fixed << setprecision(2) << (value < 0 ? -scaled : scaled) << suffixes[idx];
    return oss.str();
}

class Component {
protected:
    string name;
    int nodeA, nodeB;

public:
    Component(string n, int a, int b): name(n), nodeA(a), nodeB(b) {}

    virtual ~Component() = default;
    virtual complex<double> getImpedance(double frequency) const = 0;
    virtual double getValue() const = 0;
    virtual string getType() const = 0;
    virtual string getName() const { return name; }
    virtual bool isSource() const { return false; }
    virtual string getSymbol(double frequency = -1.0) const {
        return getName() + "(" + engFormat(getValue()) + ")";
    }
    int getNodeA() const { return nodeA; }
    int getNodeB() const { return nodeB; }
};

#endif
