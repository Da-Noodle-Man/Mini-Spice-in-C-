#include <iostream>
#include <iomanip>
#include <cmath>
#include <limits>
#include <functional>
#include <string>
#include "Circuit.h"
#include "Resistor.h"
#include "Inductor.h"
#include "Capacitor.h"
#include "ACVoltageSource.h"
#include "DCVoltageSource.h"
#include "Analysis.h"
#include "NodalAnalysis.h"

// ============================================
// Helper Functions
// ============================================

// Keep asking until user enters valid number
double askNumber(const std::string& prompt,
                 const std::function<bool(double)>& valid,
                 const std::string& errorMsg) {
    while (true) {
        std::cout << prompt;
        double v;
        if (std::cin >> v && std::isfinite(v) && valid(v)) return v;
        if (std::cin.eof()) throw std::runtime_error("Input closed");
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  " << errorMsg << std::endl;
    }
}

// Print impedance in engineering format
void printZ(std::complex<double> Z) {
    std::cout << std::fixed << std::setprecision(2);
    if (std::abs(Z) > 1e12) {
        std::cout << "infinite (open circuit)" << std::endl;
        return;
    }
    std::cout << Z.real() << (Z.imag() < 0 ? " - j" : " + j") 
              << std::abs(Z.imag()) << " ohm"
              << "   |Z| = " << std::abs(Z) << " ohm"
              << "   phase = " << std::arg(Z) * 180.0 / M_PI << " deg" << std::endl;
}

// Print voltage in engineering format
void printVoltage(std::complex<double> V) {
    std::cout << std::fixed << std::setprecision(4);
    std::cout << V.real() << (V.imag() < 0 ? " - j" : " + j") 
              << std::abs(V.imag()) << " V"
              << "   |V| = " << std::abs(V) << " V"
              << "   phase = " << std::arg(V) * 180.0 / M_PI << " deg" << std::endl;
}

// ============================================
// Main Program
// ============================================

int main() {
    try {
        //std::cout << "\n╔════════════════════════════════════════╗" << std::endl;
        std::cout << "      MiniSPICE Phase 3 - Complete      " << std::endl;
        std::cout << "  Impedance Analysis + Nodal Analysis   " << std::endl;
        //std::cout << "╚════════════════════════════════════════╝\n" << std::endl;

        // Get AC source settings from user
        std::cout << "Enter AC voltage source settings:" << std::endl;
        double amp = askNumber("  Amplitude, peak volts (>= 0): ",
                               [](double v) { return v >= 0; }, "Must be 0 or more.");
        double phase = askNumber("  Phase shift, degrees: ",
                                 [](double) { return true; }, "Enter a number.");
        double freq = askNumber("  Frequency, Hz (> 0): ",
                                [](double v) { return v > 0; }, "Must be greater than 0.");

        // ============================================
        // Build the circuit
        // ============================================
        std::cout << "\n--- Building Circuit ---\n" << std::endl;
        
        Circuit myCircuit(4);  // nodes: 0 (ground), 1, 2, 3
        
        // Add AC voltage source between node 1 and ground
        myCircuit.add<ACVoltageSource>("V1", 1, 0, amp, phase, freq);
        std::cout << " Added AC source V1: " << amp << "V ∠ " << phase
                  << "° at " << engFormat(freq) << "Hz" << std::endl;
        
        // Add passive components
        myCircuit.add<Resistor>("R1", 1, 2, 1000.0);  // 1 kOhm
        std::cout << " Added Resistor R1: 1kΩ between nodes 1-2" << std::endl;
        
        myCircuit.add<Inductor>("L1", 2, 3, 0.01);  // 10 mH
        std::cout << " Added Inductor L1: 10mH between nodes 2-3" << std::endl;
        
        myCircuit.add<Capacitor>("C1", 3, 0, 1e-6);  // 1 µF
        std::cout << " Added Capacitor C1: 1µF between nodes 3-0" << std::endl;

        // Validate circuit
        auto problems = myCircuit.validate();
        if (!problems.empty()) {
            for (const auto& p : problems) {
                std::cout << "Problem: " << p << std::endl;
            }
            return 1;
        }

        const VoltageSource* source = findSource(myCircuit);
        double f = source->getFrequency();

        // ============================================
        // PART 1: Components and Topology
        // ============================================
        //std::cout << "\n╔════════════════════════════════════════╗" << std::endl;
        std::cout << "  PART 1: Circuit Components            " << std::endl;
        //std::cout << "╚════════════════════════════════════════╝\n" << std::endl;
        
        myCircuit.listComponents();
        
        std::cout << "\nVoltage Source Details:" << std::endl;
        std::cout << "  Name: " << source->getName() << std::endl;
        std::cout << "  Type: " << source->getType() << std::endl;
        std::cout << "  Symbol: " << source->getSymbol() << std::endl;
        std::cout << "  Positive terminal: node " << source->getNodeA() << std::endl;
        std::cout << "  Negative terminal: node " << source->getNodeB() << " (ground)" << std::endl;
        std::cout << "  Frequency: " << engFormat(f) << "Hz\n" << std::endl;

        // ============================================
        // PART 2: Impedance Analysis
        // ============================================
        //std::cout << "╔════════════════════════════════════════╗" << std::endl;
        std::cout << "  PART 2: Impedance Analysis            " << std::endl;
        std::cout << "  (Impedance seen by voltage source)    " << std::endl;
        //std::cout << "╚════════════════════════════════════════╝\n" << std::endl;

        std::cout << "--- Individual Component Impedances at " << engFormat(f) << "Hz ---\n" << std::endl;
        
        for (const auto& c : myCircuit.getComponents()) {
            if (c->isSource()) continue;
            std::cout << "  " << std::left << std::setw(5) << c->getName()
                      << std::setw(10) << c->getSymbol();
            printZ(c->getImpedance(f));
        }

        // Calculate series impedance of passive components
        std::cout << "\nCalculating total impedance..." << std::endl;
        std::cout << "  Path: R1 (series) → L1 (series) → C1 (shunt to ground)" << std::endl;
        
        std::complex<double> Z_R1 = myCircuit.getComponents()[1]->getImpedance(f);
        std::complex<double> Z_L1 = myCircuit.getComponents()[2]->getImpedance(f);
        std::complex<double> Z_C1 = myCircuit.getComponents()[3]->getImpedance(f);
        
        // Series: R1 + L1
        std::complex<double> Z_series = Z_R1 + Z_L1;
        std::cout << "\n  Z_series (R1 + L1) = ";
        printZ(Z_series);
        
        // Total: (R1 + L1) in series with C1
        std::complex<double> Z_total_manual = Z_series + Z_C1;
        std::cout << "  Z_total (R1+L1 in series with C1) = ";
        printZ(Z_total_manual);

        std::cout << "\n--- Circuit Impedance (using nodal analysis) ---\n" << std::endl;
        std::complex<double> Z_total = circuitImpedance(myCircuit, f);
        std::cout << "  Z_total = ";
        printZ(Z_total);
        std::cout << std::endl;

        // ============================================
        // PART 3: Nodal Analysis - Node Voltages
        // ============================================
        //std::cout << "╔════════════════════════════════════════╗" << std::endl;
        std::cout << "  PART 3: Nodal Analysis                " << std::endl;
        std::cout << "  (Node Voltages at Frequency)          " << std::endl;
        //std::cout << "╚════════════════════════════════════════╝\n" << std::endl;

        std::cout << "Solving circuit for node voltages..." << std::endl;

        NodalAnalyzer analyzer(myCircuit);
        AnalysisResult result = analyzer.solve(f);
        const auto& nodeVoltages = result.nodeVoltages;

        std::cout << "\nNode Voltages:\n" << std::endl;
        for (size_t i = 0; i < nodeVoltages.size(); i++) {
            std::cout << "  V[" << i << "] = ";
            printVoltage(nodeVoltages[i]);
        }

        //std::cout << "╔════════════════════════════════════════╗" << std::endl;
        std::cout << "  PART 4: Circuit Behavior Analysis     " << std::endl;
        //std::cout << "╚════════════════════════════════════════╝\n" << std::endl;

        // Calculate source current
        std::complex<double> V_source = source->getSourceVoltage(f);
        std::complex<double> I_source = V_source / Z_total;
        
        std::cout << "Source Analysis:" << std::endl;
        std::cout << "  Source voltage: ";
        printVoltage(V_source);
        
        std::cout << "  Source current: ";
        std::cout << std::fixed << std::setprecision(6) 
                  << I_source.real() << (I_source.imag() < 0 ? " - j" : " + j") 
                  << std::abs(I_source.imag()) << " A"
                  << "   I = " << std::abs(I_source) << " A"
                  << "   phase = " << std::arg(I_source) * 180.0 / M_PI << " deg" << std::endl;
        
        std::cout << "  Power dissipated (real): " << std::fixed << std::setprecision(4)
                  << (V_source * std::conj(I_source)).real() << " W" << std::endl;

        // Voltage drops across components
        std::cout << "\nVoltage Drops Across Components:" << std::endl;
        
        std::complex<double> V_R1 = nodeVoltages[1] - nodeVoltages[2];
        std::cout << "  V_R1 (nodes 1-2): ";
        printVoltage(V_R1);
        
        std::complex<double> V_L1 = nodeVoltages[2] - nodeVoltages[3];
        std::cout << "  V_L1 (nodes 2-3): ";
        printVoltage(V_L1);
        
        std::complex<double> V_C1 = nodeVoltages[3] - nodeVoltages[0];
        std::cout << "  V_C1 (nodes 3-0): ";
        printVoltage(V_C1);

        // Calculate currents
        std::cout << "\nCurrents Through Components:" << std::endl;
        
        std::complex<double> I_R1 = V_R1 / Z_R1;
        std::cout << "  I_R1: " << std::fixed << std::setprecision(6)
                  << I_R1.real() << (I_R1.imag() < 0 ? " - j" : " + j")
                  << std::abs(I_R1.imag()) << " A   (magnitude: " 
                  << std::abs(I_R1) << " A)" << std::endl;
        
        std::complex<double> I_L1 = V_L1 / Z_L1;
        std::cout << "  I_L1: " << std::fixed << std::setprecision(6)
                  << I_L1.real() << (I_L1.imag() < 0 ? " - j" : " + j")
                  << std::abs(I_L1.imag()) << " A   (magnitude: "
                  << std::abs(I_L1) << " A)" << std::endl;
        
        std::complex<double> I_C1 = V_C1 / Z_C1;
        std::cout << "  I_C1: " << std::fixed << std::setprecision(6)
                  << I_C1.real() << (I_C1.imag() < 0 ? " - j" : " + j")
                  << std::abs(I_C1.imag()) << " A   (magnitude: "
                  << std::abs(I_C1) << " A)" << std::endl;


        //std::cout << "\n╔════════════════════════════════════════╗" << std::endl;
        std::cout << "   ANALYSIS COMPLETE      " << std::endl;
        //std::cout << "╚════════════════════════════════════════╝\n" << std::endl;

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "\n✗ ERROR: " << e.what() << std::endl;
        return 1;
    }
}
