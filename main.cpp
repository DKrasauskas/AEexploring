#include <iostream>
#include <cmath>
#include <fstream>

//global variables
float  day  = 11.4f;
float night  = (24.6f - 11.4f);
float convFactor = (60*60);
float flux = 490.0f;
float g  =3.72f;
float rho = 0.013225739f;

namespace Parameters{
    float mass_empty = 1.25f;
    float S = 2.1f;
    float eta = (float)1/0.89*0.79f;
    float P_avionics = 11.5;
    float cl = 1.020f;
    float cd = 0.03347f;
}

struct Battery{
    float mass, E_density, Energy;

    Battery() = default;
    Battery(float mass, float E_density){
        this -> mass = mass;
        this -> E_density = E_density;
        Get_Energy();
    }
    void Get_Energy(){
        Energy =  mass * E_density;
    }
};
struct SolarPanels{
    // density => mass/area
    float mass, efficiency, density;
};

class Aircraft{
public:
    //fields
    float mass, cL, cD, density, S, eta;
    double P_R, P_avionics;
    Battery batmann;
    SolarPanels panels;

    //constructors
    Aircraft() = default;
    Aircraft(float mass, float cL, float cD, float density, float S, float eta, double P_avionics,  Battery batmann, SolarPanels panels){
        this -> mass = mass;
        this -> cL = cL;
        this -> cD = cD;
        this -> density = density;
        this -> S = S;
        this -> batmann = batmann;
        this -> panels = panels;
        this -> eta = eta;
        this -> P_avionics = P_avionics;
        P_R = Power();
    }

    //methods
     double Power() {
        return 1/eta*pow(pow((mass+batmann.mass+panels.mass)*g, 3) * 2 * 1 / (density * S) * pow(cD, 2)/ pow(cL, 3), 0.5f) + P_avionics;
    }
};
std::ofstream out("avionics.txt");
std::ofstream out1("battery.txt");
std::ofstream out2("panel.txt");
std::ofstream out3("efficiency.txt");
class Performance{
public:
    Performance() = default;
    Aircraft aircraft;
    void FlyNight(){
        float dE = aircraft.P_R * night * convFactor;
        aircraft.batmann.Energy -= dE;
    }
    void FlyDay(){
        float dE = aircraft.P_R * day * convFactor - aircraft.panels.efficiency * aircraft.panels.density * aircraft.panels.mass * flux * day * convFactor;
        if(-dE > aircraft.batmann.mass * aircraft.batmann.E_density) aircraft.batmann.Get_Energy();
        else{
            aircraft.batmann.Energy -= dE;
        }
    }
    int Iterate(){
        for(int i = 0; i < 10000; i ++){
            FlyDay();
            FlyNight();
           // std::cout << aircraft.batmann.Energy << " " <<  aircraft.batmann.mass*aircraft.batmann.E_density << "\n";
            if(aircraft.batmann.Energy <= 0) return -1;
           // if(aircraft.batmann.Energy == aircraft.batmann.mass * aircraft.batmann.E_density - night*convFactor*aircraft.P_R) return 1;
        }
        return 0;
    }
    void IncreasePerformance(){
        float mass = aircraft.mass + aircraft.batmann.mass + aircraft.panels.mass;
        float initial_mass = 0;
        float dm = 0.001;
        float ideal_mass = 0xFFFFFF;

        for(int j = 0; j < 10000; j ++){
            int current_index = -1;
            for(int i = 0; i < 10000; i ++) {
                aircraft.panels.mass += dm;
                aircraft.P_R = aircraft.Power();
                int status = Iterate();
                if (status == 0) {
                    current_index = i;
                    break;
                }
                if (status == 1) {
                    current_index = i;
                    break;
                }
            }
            if(current_index == -1) {
                aircraft.batmann.mass += dm;
                aircraft.panels.mass = 0;
                continue;
            }

            if(ideal_mass > aircraft.mass + dm*(current_index+1) + aircraft.batmann.mass){
                ideal_mass = aircraft.mass + dm*(current_index+1) + aircraft.batmann.mass;
                aircraft.P_R = aircraft.Power();
                std::cout << ideal_mass << " battery_mass " << aircraft.batmann.mass << " panel_mass " << dm*(current_index+1)  << " Power_required " << aircraft.P_R << "\n";
                out << ideal_mass  << "\n";
                out1 << aircraft.batmann.mass  << "\n";
                out2 << aircraft.P_R  << "\n";
                out3 << aircraft.batmann.E_density/convFactor  << "\n";

            }
            else return;
            aircraft.panels.mass = 0;
            aircraft.batmann.mass += dm*1000;

        }
    }
};

int main() {
    Battery battery = {0, 200*60*60};
    SolarPanels panels = {0, 0.05, 1/0.32};
    Performance test = Performance();
    test.aircraft = {Parameters::mass_empty, Parameters::cl, Parameters::cd, rho, Parameters::S, Parameters::eta, Parameters::P_avionics,battery, panels};
    for(int i = 0; i <= 20; i ++){
        test.aircraft.batmann.E_density = (float)(i+14)*10*convFactor;
        test.IncreasePerformance();
        test.aircraft.batmann.mass = 0;
        test.aircraft.panels.mass = 0;
    }
    return 0;
}
