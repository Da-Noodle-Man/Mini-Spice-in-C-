#ifndef NODALANALYSIS_H
#define NODALANALYSIS_H
#include "Circuit.h"
#include "VoltageSource.h"
#include <algorithm>
#include <cmath>
#include <complex>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
using cplx=std::complex<double>;

struct BranchResult{
    std::string name;
    std::string type;
    int nodeA=0,nodeB=0;
    cplx voltage;
    cplx current;
    double power=0.0;
};

class AnalysisResult{
public:
    double frequency=0.0;
    int activeSources=0;
    std::vector<cplx> nodeVoltages;
    std::vector<BranchResult> branches;
    bool isDC()const{return frequency==0.0;}
    cplx voltage(int node)const{
        if(node<0||node>=static_cast<int>(nodeVoltages.size()))throw std::out_of_range("Node "+std::to_string(node)+" does not exist");
        return nodeVoltages[node];
    }
    cplx voltageBetween(int a,int b)const{return voltage(a)-voltage(b);}
    const BranchResult&branch(const std::string&name)const{
        for(const auto&b:branches)
            if(b.name==name)return b;
        throw std::out_of_range("No component named '"+name+"' in the results");
    }
    cplx current(const std::string&name)const{return branch(name).current;}
    double totalPower()const{
        double p=0.0;
        for(const auto&b:branches)p+=b.power;
        return p;
    }
    void print()const;
};

class NodalAnalyzer{
public:
    explicit NodalAnalyzer(const Circuit&c):circuit(c){}
    AnalysisResult solve(double frequency)const;
private:
    const Circuit&circuit;
    static constexpr double OPEN_Z=1e12;
    static constexpr double SHORT_Z=1e-12;
    static constexpr double SINGULAR_TOL=1e-12;
    enum class Kind{Admittance,Open,Short,Source};
    struct Element{
        const Component*comp=nullptr;
        Kind kind=Kind::Admittance;
        cplx y;
        cplx vs;
        int branch=-1;
    };
    int rowOf(int node)const{
        int g=circuit.getGroundNode();
        if(node==g)return -1;
        return node<g?node:node-1;
    }
    static std::vector<cplx> gaussSolve(std::vector<std::vector<cplx>>A,std::vector<cplx>b,double frequency);
};

inline std::vector<cplx> NodalAnalyzer::gaussSolve(std::vector<std::vector<cplx>>A,std::vector<cplx>b,double frequency){
    const size_t n=b.size();
    double scale=0.0;
    for(const auto&row:A)
        for(const auto&v:row)scale=std::max(scale,std::abs(v));
    auto singular=[&](){
        return std::runtime_error("Circuit equations are singular at f = "+engFormat(frequency)+"Hz. Likely causes: a loop made only of voltage sources and/or inductors (at DC), or a part of the circuit with no path to ground.");
    };
    if(scale==0.0)throw singular();
    for(size_t col=0;col<n;++col){
        size_t pivot=col;
        for(size_t r=col+1;r<n;++r)
            if(std::abs(A[r][col])>std::abs(A[pivot][col]))pivot=r;
        if(std::abs(A[pivot][col])<SINGULAR_TOL*scale)throw singular();
        std::swap(A[pivot],A[col]);
        std::swap(b[pivot],b[col]);
        for(size_t r=col+1;r<n;++r){
            cplx factor=A[r][col]/A[col][col];
            if(factor==cplx(0.0,0.0))continue;
            for(size_t c=col;c<n;++c)A[r][c]-=factor*A[col][c];
            b[r]-=factor*b[col];
        }
    }
    std::vector<cplx> x(n);
    for(size_t i=n;i-->0;){
        cplx sum=b[i];
        for(size_t c=i+1;c<n;++c)sum-=A[i][c]*x[c];
        x[i]=sum/A[i][i];
    }
    return x;
}

inline AnalysisResult NodalAnalyzer::solve(double frequency)const{
    if(!std::isfinite(frequency)||frequency<0.0)throw std::invalid_argument("Frequency must be a finite number >= 0");
    auto problems=circuit.validate();
    if(!problems.empty()){
        std::string msg="Circuit is not valid:";
        for(const auto&p:problems)msg+="\n  - "+p;
        throw std::runtime_error(msg);
    }
    if(!circuit.hasSource())throw std::runtime_error("Circuit has no source - nothing drives it");

    std::vector<Element> elems;
    int numBranch=0;
    int active=0;
    for(const auto&up:circuit.getComponents()){
        const Component*c=up.get();
        Element e;
        e.comp=c;
        if(c->isSource()){
            const auto*vs=dynamic_cast<const VoltageSource*>(c);
            if(!vs)throw std::runtime_error("Component '"+c->getName()+"' is a source of an unsupported kind");
            e.kind=Kind::Source;
            e.vs=vs->getSourceVoltage(frequency);
            e.branch=numBranch++;
            if(std::abs(e.vs)>0.0)++active;
        }else{
            cplx Z=c->getImpedance(frequency);
            double mag=std::abs(Z);
            if(mag>=OPEN_Z){
                e.kind=Kind::Open;
            }else if(mag<=SHORT_Z){
                e.kind=Kind::Short;
                e.vs=cplx(0.0,0.0);
                e.branch=numBranch++;
            }else{
                e.kind=Kind::Admittance;
                e.y=cplx(1.0,0.0)/Z;
            }
        }
        elems.push_back(e);
    }

    {
        std::vector<bool> reached(circuit.getNumNodes(),false);
        reached[circuit.getGroundNode()]=true;
        bool grew=true;
        while(grew){
            grew=false;
            for(const auto&e:elems){
                if(e.kind==Kind::Open)continue;
                int a=e.comp->getNodeA(),b=e.comp->getNodeB();
                if(reached[a]!=reached[b]){
                    reached[a]=reached[b]=true;
                    grew=true;
                }
            }
        }
        std::string floating;
        for(int n=0;n<circuit.getNumNodes();++n)
            if(!reached[n])floating+=(floating.empty()?"":", ")+std::to_string(n);
        if(!floating.empty())throw std::runtime_error("At f = "+engFormat(frequency)+"Hz node(s) "+floating+" have no conducting path to ground (they are only connected through open circuits, e.g. capacitors at DC)");
    }

    const int N=circuit.getNumNodes()-1;
    const int M=numBranch;
    const int size=N+M;
    std::vector<std::vector<cplx>> A(size,std::vector<cplx>(size,cplx(0.0,0.0)));
    std::vector<cplx> z(size,cplx(0.0,0.0));
    auto add=[&](int r,int c,cplx v){
        if(r>=0&&c>=0)A[r][c]+=v;
    };
    for(const auto&e:elems){
        int ra=rowOf(e.comp->getNodeA());
        int rb=rowOf(e.comp->getNodeB());
        switch(e.kind){
            case Kind::Open:
                break;
            case Kind::Admittance:
                add(ra,ra,e.y);
                add(rb,rb,e.y);
                add(ra,rb,-e.y);
                add(rb,ra,-e.y);
                break;
            case Kind::Short:
            case Kind::Source:{
                int k=N+e.branch;
                add(ra,k,1.0);
                add(rb,k,-1.0);
                add(k,ra,1.0);
                add(k,rb,-1.0);
                z[k]=e.vs;
                break;
            }
        }
    }

    std::vector<cplx> x=gaussSolve(A,z,frequency);

    AnalysisResult res;
    res.frequency=frequency;
    res.activeSources=active;
    res.nodeVoltages.assign(circuit.getNumNodes(),cplx(0.0,0.0));
    for(int n=0;n<circuit.getNumNodes();++n){
        int r=rowOf(n);
        if(r>=0)res.nodeVoltages[n]=x[r];
    }
    const double powerFactor=(frequency==0.0)?1.0:0.5;
    for(const auto&e:elems){
        BranchResult br;
        br.name=e.comp->getName();
        br.type=e.comp->getType();
        br.nodeA=e.comp->getNodeA();
        br.nodeB=e.comp->getNodeB();
        br.voltage=res.nodeVoltages[br.nodeA]-res.nodeVoltages[br.nodeB];
        switch(e.kind){
            case Kind::Open:br.current=cplx(0.0,0.0);break;
            case Kind::Admittance:br.current=br.voltage*e.y;break;
            case Kind::Short:
            case Kind::Source:br.current=x[N+e.branch];break;
        }
        br.power=powerFactor*std::real(br.voltage*std::conj(br.current));
        res.branches.push_back(br);
    }
    return res;
}

namespace nodal_detail{
inline std::string fmtReal(double x,double ref,const std::string&unit){
    if(std::abs(x)<=1e-9*ref)return "0"+unit;
    return engFormat(x)+unit;
}

inline std::string fmtPhasor(cplx z,double ref,const std::string&unit,bool dc){
    if(dc)return fmtReal(z.real(),ref,unit);
    double mag=std::abs(z);
    if(mag<=1e-9*ref)return "0"+unit;
    double deg=std::round(std::arg(z)*180.0/M_PI*10.0)/10.0;
    std::string d=std::to_string(deg);
    d=d.substr(0,d.find('.')+2);
    return engFormat(mag)+unit+" @ "+d+" deg";
}
}

inline void AnalysisResult::print()const{
    using namespace std;
    using namespace nodal_detail;
    double vref=0.0,iref=0.0,pref=0.0;
    for(const auto&v:nodeVoltages)vref=max(vref,abs(v));
    for(const auto&b:branches){
        iref=max(iref,abs(b.current));
        pref=max(pref,abs(b.power));
    }

    cout<<"\n=== Nodal Analysis: ";
    if(isDC())cout<<"DC (0 Hz)";
    else cout<<"AC at "<<engFormat(frequency)<<"Hz";
    cout<<" ===\n";
    if(activeSources==0)cout<<"Note: no source is active at this frequency, so everything is 0.\n";
    if(!isDC())cout<<"(values are peak phasors: magnitude @ phase)\n";
    cout<<"\nNode voltages (node 0 = ground)\n";
    for(size_t n=0;n<nodeVoltages.size();++n)
        cout<<"  V("<<n<<") = "<<fmtPhasor(nodeVoltages[n],vref,"V",isDC())<<(n==0?"   (ground)":"")<<"\n";

    size_t wName=4,wV=7,wI=7;
    vector<string> vs,is,ps;
    for(const auto&b:branches){
        vs.push_back(fmtPhasor(b.voltage,vref,"V",isDC()));
        is.push_back(fmtPhasor(b.current,iref,"A",isDC()));
        ps.push_back(fmtReal(b.power,pref,"W"));
        wName=max(wName,b.name.size());
        wV=max(wV,vs.back().size());
        wI=max(wI,is.back().size());
    }

    cout<<"\nComponents (current flows node A -> node B; power > 0 = absorbed)\n";
    cout<<"  "<<left<<setw(wName+2)<<"Name"<<setw(7)<<"Nodes"<<setw(wV+2)<<"Voltage"<<setw(wI+2)<<"Current"<<"Power\n";
    for(size_t i=0;i<branches.size();++i){
        const auto&b=branches[i];
        string nodes=to_string(b.nodeA)+"->"+to_string(b.nodeB);
        cout<<"  "<<left<<setw(wName+2)<<b.name<<setw(7)<<nodes<<setw(wV+2)<<vs[i]<<setw(wI+2)<<is[i]<<ps[i]<<"\n";
    }
    cout<<endl;
}
#endif
