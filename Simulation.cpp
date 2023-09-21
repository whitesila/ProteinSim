#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include "Env.h"
#include "parm.h"
#include "Sim.h"
#include "Vector_Math.h"
#include <math.h>
#include <thread>
#include <iomanip>

simulation::simulation(Environment& temp1, parm& temp2, float step, std::string export_name){

    coord = std::make_unique<Environment>(temp1);
    top = std::make_unique<parm>(temp2);


    velocities = std::vector<float>(coord->Acoords.size(), 0);
    forces = std::vector<float>(coord->Acoords.size(), 0);
    

    temp_file.open("/home/nich/Documents/GitHub/coord_vis/" + export_name +".crd", std::ios::out);
    temp_file << std::endl;
    }


void simulation::update_coord(float step_size, int frames, int export_step){
    //initialize bond force constants 
    std::unordered_map<std::string, std::vector<T> >::iterator bwh = top->values.find("BONDS_WITHOUT_HYDROGEN");
        std::vector<T>& BWoutH = bwh->second;
    
    std::unordered_map<std::string, std::vector<T> >::iterator bih = top->values.find("BONDS_INC_HYDROGEN");
        std::vector<T>& BIH = bih->second;

    std::unordered_map<std::string, std::vector<T> >::iterator bfc = top->values.find("BOND_FORCE_CONSTANT");
        std::vector<T>& BForceC = bfc->second;

    std::unordered_map<std::string, std::vector<T> >::iterator bev = top->values.find("BOND_EQUIL_VALUE");
        std::vector<T>& BEQV = bev->second;


    //initialize angle force constants
    std::unordered_map<std::string, std::vector<T> >::iterator awh = top->values.find("ANGLES_WITHOUT_HYDROGEN");
        std::vector<T>& AWoutH = awh->second;

    std::unordered_map<std::string, std::vector<T> >::iterator aih = top->values.find("ANGLES_INC_HYDROGEN");
        std::vector<T>& AIH = aih->second;

    std::unordered_map<std::string, std::vector<T> >::iterator afc = top->values.find("ANGLE_FORCE_CONSTANT");
        std::vector<T>& AForceC = afc->second;

    std::unordered_map<std::string, std::vector<T> >::iterator aev = top->values.find("ANGLE_EQUIL_VALUE");
        std::vector<T>& AEQV = aev->second;


    //initialize DIH force constants
    std::unordered_map<std::string, std::vector<T> >::iterator dih = top->values.find("DIHEDRALS_INC_HYDROGEN");
        std::vector<T>& DincH = dih->second;

    std::unordered_map<std::string, std::vector<T> >::iterator dwh = top->values.find("DIHEDRALS_WITHOUT_HYDROGEN");
        std::vector<T>& DWoutH = dwh->second;

    std::unordered_map<std::string, std::vector<T> >::iterator dfc = top->values.find("DIHEDRAL_FORCE_CONSTANT");
        std::vector<T>& DForceC = dfc->second;

    std::unordered_map<std::string, std::vector<T> >::iterator dp = top->values.find("DIHEDRAL_PERIODICITY");
        std::vector<T>& DPeriod = dp->second;

    std::unordered_map<std::string, std::vector<T> >::iterator dph = top->values.find("DIHEDRAL_PHASE");
        std::vector<T>& DPhase = dph->second;

    std::unordered_map<std::string, std::vector<T> >::iterator scee = top->values.find("SCEE_SCALE_FACTOR");
        std::vector<T>& SCEE_SF = scee->second;      

    std::unordered_map<std::string, std::vector<T> >::iterator scnb = top->values.find("SCNB_SCALE_FACTOR");
        std::vector<T>& SCNB_SF = scnb->second;  

    std::unordered_map<std::string, std::vector<T> >::iterator lja = top->values.find("LENNARD_JONES_ACOEF");
        std::vector<T>& LJAC = lja->second;  

    std::unordered_map<std::string, std::vector<T> >::iterator ljb = top->values.find("LENNARD_JONES_BCOEF");
        std::vector<T>& LJBC = ljb->second;  

    std::unordered_map<std::string, std::vector<T> >::iterator ati = top->values.find("ATOM_TYPE_INDEX");
        std::vector<T>& ATI = ati->second;  

    std::unordered_map<std::string, std::vector<T> >::iterator nbpi = top->values.find("NONBONDED_PARM_INDEX");
        std::vector<T>& NBPIndex = nbpi->second;
  
    //import excluded atoms 
    std::unordered_map<std::string, std::vector<T> >::iterator nea = top->values.find("NUMBER_EXCLUDED_ATOMS");
        std::vector<T>& NEA = nea->second;
    std::unordered_map<std::string, std::vector<T> >::iterator eal = top->values.find("EXCLUDED_ATOMS_LIST");
        std::vector<T>& EAL = eal->second;

    std::vector<std::vector<int> > excluded;
    int count = 0;
    for(int i = 0; i < NEA.size(); i++){
        std::vector<int> temp;

        for(int y = count; y < count + std::get<int> (NEA[i]); y++){
            temp.push_back(std::get<int>(EAL[y]));

    }
        count += std::get<int> (NEA[i]);
        excluded.push_back(temp);
    }



for(int i = 0; i <= frames; i++){
        VerletAlg(step_size,  BWoutH,  BIH,  BForceC, BEQV,  AWoutH,
  AIH, AForceC, AEQV,  DincH,  DWoutH,  DForceC, 
 DPeriod ,  DPhase,  SCEE_SF,  SCNB_SF, LJAC,  LJBC, ATI,
 NBPIndex, excluded);
        
        if(i % export_step == 0) {
            std::cout<< (float) i / frames * 100 << "% complete!" << std::endl;
            float total_f = 0;
            exports();
        }
    }




    
}

void simulation::force_additions(std::vector<T>& BWoutH, std::vector<T>& BIH, std::vector<T>& BForceC, std::vector<T>& BEQV, std::vector<T>& AWoutH,
 std::vector<T>& AIH,std::vector<T>& AForceC, std::vector<T>& AEQV, std::vector<T>& DincH, std::vector<T>& DWoutH, std::vector<T>& DForceC, 
 std::vector<T>& DPeriod , std::vector<T>& DPhase, std::vector<T>& SCEE_SF, std::vector<T>& SCNB_SF, std::vector<T>& LJAC, std::vector<T>& LJBC,std::vector<T>& ATI,
  std::vector<T>& NBPIndex, std::vector<std::vector<int> >& excluded){ //all forces on protien for bond/FF interactions



    //bond forces
    for(int i = 0; i < BWoutH.size() - 1; i+=3){ //BWoutH.size()
        spring_force( std::get<int>(BWoutH[i]) / 3,std::get<int>(BWoutH[i + 1]) / 3, std::get<float>(BForceC[std::get<int>(BWoutH[i + 2]) - 1]), std::get<float>(BEQV[std::get<int>(BWoutH[i + 2]) - 1]) );
    
    }
    for(int i = 0; i < BIH.size(); i+=3){ // this is a bond which include hydrogen. I want to stiffen interaction so as to reduce computational complexity. I can set up a method for selecting a model and integrate it into there
        spring_force( std::get<int>(BIH[i]) / 3,std::get<int>(BIH[i + 1]) / 3, std::get<float>(BForceC[std::get<int>(BIH[i + 2]) - 1]), std::get<float>(BEQV[std::get<int>(BIH[i + 2]) - 1]) );
    }


    //angle forces
    
    for(int i = 0; i < AWoutH.size(); i+=4){
        angle_force( std::get<int>(AWoutH[i]) / 3,std::get<int>(AWoutH[i + 1]) / 3, std::get<int>(AWoutH[i + 2]) / 3, std::get<float>(AForceC[std::get<int>(AWoutH[i + 3]) - 1]), std::get<float>(AEQV[std::get<int>(AWoutH[i + 3]) - 1]));

    }

    for(int i = 0; i < AIH.size() - 1; i+=4){ 
        angle_force( std::get<int>(AIH[i]) / 3,std::get<int>(AIH[i + 1]) / 3, std::get<int>(AIH[i + 2]) / 3, std::get<float>(AForceC[std::get<int>(AIH[i + 3]) - 1]), std::get<float>(AEQV[std::get<int>(AIH[i + 3]) - 1]));

    }

    //Dihedral forces
    


    for(int i = 0; i < DWoutH.size(); i+=5){
        dihedral_force( std::get<int>(DWoutH[i]) / 3, std::get<int>(DWoutH[i + 1]) / 3, (std::get<int>(DWoutH[i + 2]) / 3), (std::get<int>(DWoutH[i + 3]) / 3), std::get<float>(DForceC[std::get<int>(DWoutH[i + 4]) - 1]), 
        std::get<float>(DPeriod[std::get<int>(DWoutH[i + 4]) - 1]), std::get<float>(SCEE_SF[std::get<int>(DWoutH[i + 4]) - 1]), std::get<float>(SCNB_SF[std::get<int>(DWoutH[i + 4]) - 1]), std::get<float>(DPhase[std::get<int>(DWoutH[i + 4]) - 1]));
        
        if((std::get<int>(DWoutH[i + 2]) / 3) >= 0){
            float LJA, LJB;
            int atom1, atom4;
            atom1 = std::get<int>(DWoutH[i]) / 3;
            atom4 = std::get<int>(DWoutH[i + 3]) / 3;
            int temp;
            LJA = std::get<int>(ATI[atom1]);
            LJB = std::get<int>(ATI[abs(atom4)]);

            temp = std::get<int>(NBPIndex[sqrt(NBPIndex.size()) * (LJA - 1) + LJB]);
            //std::cout << temp << std::endl;
            LJB = std::get<float>(LJBC[temp - 1]);
            LJA = std::get<float>(LJAC[temp - 1]);


            DH_LJF(atom1, abs(atom4), std::get<float>(SCNB_SF[std::get<int>(DWoutH[i + 4]) - 1]), LJA, LJB);
        }
        
        

    }
    for(int i = 0; i < DincH.size(); i+=5){

        dihedral_force( std::get<int>(DincH[i]) / 3,std::get<int>(DincH[i + 1]) / 3, (std::get<int>(DincH[i + 2]) / 3), (std::get<int>(DincH[i + 3]) / 3), std::get<float>(DForceC[std::get<int>(DincH[i + 4]) - 1]), 
        std::get<float>(DPeriod[std::get<int>(DincH[i + 4]) - 1]), std::get<float>(SCEE_SF[std::get<int>(DincH[i + 4]) - 1]), std::get<float>(SCNB_SF[std::get<int>(DincH[i + 4]) - 1]), std::get<float>(DPhase[std::get<int>(DincH[i + 4]) - 1]));
        
        if((std::get<int>(DincH[i + 2]) / 3) >= 0){
            float LJA, LJB;
            int atom1, atom4;
            atom1 = abs(std::get<int>(DincH[i]) / 3);
            atom4 = abs(std::get<int>(DincH[i + 3]) / 3);


            int temp;
            LJA = std::get<int>(ATI[atom1]);
            LJB = std::get<int>(ATI[abs(atom4)]);

            temp = std::get<int>(NBPIndex[sqrt(NBPIndex.size()) * (LJA - 1) + LJB]);
            
            LJA = std::get<float>(LJAC[temp - 1]);
            LJB = std::get<float>(LJBC[temp - 1]);


            

            DH_LJF(atom1, abs(atom4), std::get<float>(SCNB_SF[std::get<int>(DincH[i + 4]) - 1]), LJA, LJB);
        }
    }

    //LJ/hbond forces
    for(int i = 0; i < ATI.size(); i++){
        for(int y = i + 1; y < ATI.size(); y++){
            int check = 0;
            for(int exc = 0; exc < excluded[i].size(); exc++){
                if(excluded[i][exc] == y + 1){
                    check = 1;
                }
                }
            if(check == 1){
                ;
            } else {
            float A, B; 
            int a1 = i; 
            int a2 = y;
            A = std::get<int>(ATI[a1]);
            B = std::get<int>(ATI[a2]);
            int temp = std::get<int>(NBPIndex[sqrt(NBPIndex.size()) * (A - 1) + B]);
            if(temp >= 0){
            A = std::get<float>(LJAC[temp - 1]); 
            B = std::get<float>(LJBC[temp - 1]); 
            LennardJ_force(i, y, A, B);
            }

             
            
            
            
            
            //1. import excluded_atoms_list
            //2. import number_excluded_atoms
            //3. atom excluded_atoms_list = i * 2; 
            //4. 
            
        }
    }
    //for atom1 and atom2 given, if the found non-bonded_parm_index is positive -> assign LJA and LJB and find LJ force
    // else index is negative, assign HBA and HBB, find the Hydrogen bond for

    }
}

void simulation::spring_force(int atom1, int atom2, float k, float eq){
    std::vector<float> disp;
    float dmag;
    std::vector<float> dunit_vect;

    displacement_vect(disp,coord->Acoords, atom1, atom2);
    magnitude(disp, dmag);
    unit_vector(dmag, disp, dunit_vect);

    for(int i = 0; i < disp.size(); i ++){
        float force = -0.5 * (-k * (disp[i] - eq * dunit_vect[i]));
        forces[atom2 * 3 + i] += force;
        forces[atom1 * 3 + i] -= force;

    }

}


void simulation::angle_force(int atom1, int atom2, int atom3, float k, float eq){ //this needs to be optimized so as not to make disp and mags two times
    //first find the change of potential with respect the change of angle : 2k(thetaEQ - current_theta)
    //then find the change of potential with respect to the change of bond length
    // we must then make a nomalized vector which is in plane to atoms 1,2,3 and points in a direction orth to vector 1-2, and then 2-3
    // from there we can apply forces via F12 = - du/dr dot orth_vect12 which: -2k(thetaEQ - current_theta)/mag(vector1-2) dot orth_vect12.
    //F23 is the same 
    //then take the total of both as a negative ( - f12 - f23) and then we obtain force on atom 2. since -FA + -FC = FB;
    
    std::vector<float> disp1, disp2, dispac;
    displacement_vect(disp1,coord->Acoords, atom1, atom2);
    displacement_vect(disp2,coord->Acoords, atom3, atom2);
    displacement_vect(dispac,coord->Acoords, atom1, atom3);

    std::vector<float> orth_abc, inp_ba, inp_bc;
    cross(disp1, disp2, orth_abc);
    cross(disp1, orth_abc, inp_ba);
    cross(disp2, orth_abc, inp_bc);

    float magba, magbc, mag_inp_ba, mag_inp_bc;
    magnitude(disp1, magba);
    magnitude(disp2, magbc);
    magnitude(inp_ba, mag_inp_ba);
    magnitude(inp_bc, mag_inp_bc);

    std::vector<float> unit_inp_ba, unit_inp_bc;
    unit_vector(mag_inp_ba, inp_ba, unit_inp_ba);
    unit_vector(mag_inp_bc, inp_bc, unit_inp_bc);

    float theta; 
    theta_from_dot(atom1, atom2, atom3, theta);

    float force_ba, force_bc;
    for(int i = 0; i < unit_inp_ba.size(); i++){
        force_ba =  k * (eq - theta)/magba * unit_inp_ba[i];
        force_bc =  -k * (eq - theta)/magbc * unit_inp_bc[i];
        forces[atom1 * 3 + i] += force_ba;
        forces[atom3 * 3 + i] += force_bc;
        forces[atom2 * 3 + i] += -force_ba - force_bc;
        
    }    

}   

void simulation::dihedral_force(int atom1, int atom2, int atom3, int atom4, float k, float period, float sceef, float scnbf, float phase){
//torsion potential is defined as U= 0.5[A1(1 + cos(theta)) + A2(1 - cos2theta)) + A3(1 + cost(3theta)) + A4]
//the partial derivative of tors w/ respect to pos of atom a (pa) is: dU/dpa = du/dtheta * dtheta/dpa
//dU/dpa = +/- 0.5(-A1sin(theta) + 2A2sing(2theta) - 3A3sin(3theta))
//dtheta/dpa = 1/(mag(end_vector) * sin(theta))
//steps to find forces on end atoms: 1.create unit vectors whose direction is orthogonal abc and bcd 2. find the angle between the planes 
//3. calculate the forces for atoms a and d usign (0.5/mag(ab)sin(theta)) * (1A1sin(theta) - 2A2sin(2theta) + 3A3(sin(theta)) * norm vector orth to plane. 

//1.
std::vector<float> dispba, dispbc, dispcb, dispcd;
displacement_vect(dispba,coord->Acoords, atom1, atom2);
displacement_vect(dispbc,coord->Acoords, abs(atom3), atom2);
displacement_vect(dispcb,coord->Acoords, atom2, abs(atom3));
displacement_vect(dispcd,coord->Acoords, abs(atom4), abs(atom3));

std::vector<float> orthaabc, orthabcd;
cross(dispba, dispbc, orthaabc);
cross(dispcd, dispcb, orthabcd);

float magba, magbc, magcb, magcd, magorthabc, magorthbcd;
magnitude(dispba, magba);
magnitude(dispbc, magbc);
magnitude(dispcb, magcb);
magnitude(dispcd, magcd);
magnitude(orthaabc, magorthabc);
magnitude(orthabcd, magorthbcd);

std::vector<float> northabc, northbcd, Fa, Fd;
unit_vector(magorthabc, orthaabc, northabc);
unit_vector(magorthbcd, orthabcd, northbcd);

float ab_theta, cd_theta, DH_theta;
DHrotTheta_from_dot(dispba, dispcb, magba, magbc, ab_theta);
DHrotTheta_from_dot(dispbc, dispcd, magbc, magcd, cd_theta);
DHtheta_from_dot(northabc, northbcd, magorthabc, magorthbcd, DH_theta);

for(int i = 0; i < 3; i++){

    Fa.push_back(0.5 * period * k * sin(period*DH_theta + phase)/(magba * sin(ab_theta)));
    Fd.push_back(0.5 * period * k * sin(period*DH_theta + phase)/(magcd * sin(cd_theta)));
    Fa[i] *= northabc[i];
    Fd[i] *= northbcd[i];

}



std::vector<float> dispoc, tc, tb, ocXFd, cdXFd, baXFa, Fc;
displacement_vect(dispoc, coord->Acoords, abs(atom3), atom2);
resize(dispoc, 0.5);
resize(dispcd, 0.5);
resize(dispba, 0.5);

cross(dispoc, Fd, ocXFd);
cross(dispcd, Fd, cdXFd);
cross(dispba, Fa, baXFa);

for(int i = 0; i < 3; i++){
    tc.push_back(0);
    tb.push_back(0);
}

vect_add(ocXFd, cdXFd, tc);
vect_add(tc, baXFa, tc);

resize(tc, -1);




float ocmag;
magnitude(dispoc, ocmag);
ocmag = 1/(ocmag * ocmag);
resize(tc, ocmag);

cross(tc, dispoc, Fc);

vect_add(Fa, Fd, tb);
vect_add(tb, Fc, tb);

resize(tb, -1);



//SUM OF TORQUES
resize(dispba, 2);
resize(dispcd, 2);


std::vector<float> dispob, dispoa, dispod, dispoc2;     
for(int i = 0; i < 3; i++){
    dispoa.push_back(0);
    dispod.push_back(0);
}
displacement_vect(dispoc2,coord->Acoords, abs(atom3), atom2);
displacement_vect(dispob, coord->Acoords, atom2, abs(atom3));
resize(dispoc2, 0.5);
resize(dispob, 0.5);
vect_add(dispba, dispob, dispoa);
vect_add(dispoc2, dispcd, dispod);

std::vector<float> torqoa, torqob, torqoc, torqod;

cross(dispoa, Fa, torqoa);
cross(dispob, tb, torqob);
cross(dispoc2, Fc, torqoc);
cross(dispod, Fd, torqod);

float total = 0;
for(int i = 0; i < 3; i++){
total += torqoa[i] + torqob[i] + torqoc[i] + torqod[i];
}
//std::cout << total << std::endl;
//SUM OF TROQUES


if(ab_theta){
    for(int i = 0; i < 3; i++){
        forces[atom1 * 3 + i] += Fa[i];
        forces[atom2 * 3 + i] += tb[i];
        forces[abs(atom3) * 3 + i] += Fc[i];
        forces[abs(atom4) * 3 + i] += Fd[i];
    }
}
}

void simulation::LennardJ_force(int atom1,int atom2, float LJA, float LJB){
std::vector<float> dispad, normad;
    displacement_vect(dispad, coord->Acoords, atom2, atom1);
    float magad;
    magnitude(dispad, magad);
    unit_vector(magad, dispad, normad);

    float Fa = 6/magad * (2 * LJA/(pow(magad, 12)) - LJB/pow(magad, 6));
    if(magad < 1){
        printf("%d to %d, %f\n", atom1, atom2, magad);
        printf("LJA %f\n", LJA);
        printf("LJB %f\n", LJB);
        printf("%f\n", Fa);
    }
for(int i = 0; i < 3; i++){
        forces[atom1 * 3 + i] -= Fa * normad[i];
        forces[atom2 * 3 + i] += Fa * normad[i];
    }

}

void simulation::DH_LJF(int atom1, int atom4, float SCNBF, float LJA, float LJB){

    std::vector<float> dispad, normad;
    displacement_vect(dispad, coord->Acoords, atom4, atom1);
    float magad;
    magnitude(dispad, magad);
    unit_vector(magad, dispad, normad);

    float Fa = 6/magad * (2 * LJA/(pow(magad, 12)) - LJB/pow(magad, 6))/SCNBF;

    


    if(SCNBF != 0){
        for(int i = 0; i < 3; i++){
        forces[atom1 * 3 + i] -= Fa * normad[i];
        forces[atom4 * 3 + i] += Fa * normad[i];
    }
    }


}







void simulation::DHrotTheta_from_dot(std::vector<float>& disp1, std::vector<float>& disp2, float& mag_disp1, float& mag_disp2, float& theta){
    float dotted = 0;
    dot(disp1, disp2, dotted);
    float cosine_value = dotted / (mag_disp1 * mag_disp2);
    cosine_value = std::max(-1.0f, std::min(1.0f, cosine_value));
    theta = acos(cosine_value);


}


void simulation::DHtheta_from_dot(std::vector<float>& nplane1, std::vector<float>& nplane2, float np1mag, float np2mag, float& theta){
    float dot12 = 0;
    dot(nplane1, nplane2, dot12);
    float cosine_value = dot12 / (np1mag * np2mag);
    cosine_value = std::max(-1.0f, std::min(1.0f, cosine_value));
    theta = acos(cosine_value);
    //std::cout << " theta then dot: " << dot12 << " " << np1mag << " " << np2mag << " " << theta << std::endl;
}



void simulation::theta_from_dot(int& atom1, int& atom2, int& atom3, float& theta){
    std::vector<float> disp1, disp2;
    displacement_vect(disp1, coord->Acoords,atom1, atom2);
    displacement_vect(disp2, coord->Acoords, atom3, atom2);
    float magba;
    magnitude(disp1, magba);
    float magbc;
    magnitude(disp2, magbc);
    float dotac = 0;
    dot(disp1, disp2, dotac);
    std::vector<float> ba_unitvect, bc_unitvect;

    float cosine_value = dotac/(magba*magbc);
    cosine_value = std::max(-1.0f, std::min(1.0f, cosine_value));
    theta = acos(cosine_value);
}



void simulation::VerletAlg(float& step_size, std::vector<T>& BWoutH, std::vector<T>& BIH, std::vector<T>& BForceC, std::vector<T>& BEQV, std::vector<T>& AWoutH,
 std::vector<T>& AIH,std::vector<T>& AForceC, std::vector<T>& AEQV, std::vector<T>& DincH, std::vector<T>& DWoutH, std::vector<T>& DForceC, 
 std::vector<T>& DPeriod , std::vector<T>& DPhase, std::vector<T>& SCEE_SF, std::vector<T>& SCNB_SF, std::vector<T>& LJAC, std::vector<T>& LJBC,std::vector<T>& ATI,
  std::vector<T>& NBPIndex, std::vector<std::vector<int> >& excluded){
    std::unordered_map<std::string, std::vector<T> >::iterator ms = top->values.find("MASS");
        std::vector<T>& Mass = ms->second;
for(int atom = 0; atom < velocities.size(); atom++){
    velocities[atom] = velocities[atom] + (forces[atom] * step_size/(2 * std::get<float>(Mass[atom/(int)3])));
}
for(int atom = 0; atom < coord->Acoords.size(); atom++){
    coord->Acoords[atom] = coord->Acoords[atom] + velocities[atom]*step_size;
}
forces.assign(forces.size(), 0);
force_additions(BWoutH,  BIH,  BForceC, BEQV,  AWoutH,
  AIH, AForceC, AEQV,  DincH,  DWoutH,  DForceC, 
 DPeriod ,  DPhase,  SCEE_SF,  SCNB_SF, LJAC,  LJBC, ATI,
 NBPIndex, excluded);
for(int atom = 0; atom < velocities.size(); atom++){
   velocities[atom] = velocities[atom] + forces[atom]*step_size / (2 * std::get<float>(Mass[atom/(int)3]));
   
}

}

void simulation::exports(){
    if(!temp_file.is_open()){
        std::cout << "failed open external file" << std::endl;
    }
    for(int i = 0; i < coord->Acoords.size(); i++){
        if(i % 10 == 0 && i != 0){
            temp_file << std::endl;
        }
        temp_file << std::fixed << std::right << std::setw(8) << std::setprecision(3) << (coord->Acoords[i]);
        if(i == coord->Acoords.size() - 1){
            temp_file << std::endl;
            for(int z = 1; z < 4; z++){
                temp_file << std::fixed << std::right << std::setw(8) << std::setprecision(3) << (1.000);
            }
            
        }
        
    }
    temp_file << std::endl;

}



