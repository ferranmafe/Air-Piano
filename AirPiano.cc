#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <iterator>
#include <string>
#include <string.h>
#include <set>
#include <cstring>
#include <cmath>
#include "LeeMotion.hh"
#include "Reproduce_music.hh"
#include <unistd.h>

void f(int s) {}


float calcular_angle (Vector v1,Vector v2){
    float sup = abs(v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
    float inf1 = sqrt(pow(v1.x,2) + pow(v1.y,2) + pow(v1.z,2));
    float inf2 = sqrt(pow(v2.x,2) + pow(v2.y,2) + pow(v2.z,2));
    return 180*acos(sup/(inf1*inf2))/PI;
}

template<typename Out>
void split(const std::string &s, char delim, Out result) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

map<int, musical_note_data> read_data_from_txt(string path_to_read) {
    map<int, musical_note_data> data_map;
    std::ifstream input(path_to_read); //put your program together with thsi file in the same folder.
    int i = 0;
    if(input.is_open()){
        while(!input.eof()){
            string data;
            getline(input,data);
            vector<string> splitted_data = split(data, ';'); //convert to integer
            musical_note_data aux;
            aux.hand = atoi(splitted_data[1].c_str());
            aux.finger = atoi(splitted_data[2].c_str());
            aux.height = atoi(splitted_data[3].c_str());
            //aux.route = splitted_data[4].c_str();
            data_map[atoi(splitted_data[0].c_str())] = aux;
            cout << i++ << endl;
       }
    }

    return data_map;
}

int ObtainIDNoteActiveFinger(bool isRightHand, int fingerType, Vector& h_position, const map<int, musical_note_data>& infoFinger){
    map<int, musical_note_data>::const_iterator it = infoFinger.begin();
    bool idFound = false;
    int idValue = -1;
    int alçada = 0;

    if (h_position.y > 0 && h_position.y <= 130){
        alçada = 0;
    }
    else if (h_position.y > 130 && h_position.y <= 210){
        alçada = 130;
    }
    else {
        alçada = 210;
    }
    while(!idFound && it != infoFinger.end()){
        if (it->second.hand == isRightHand && it->second.finger == fingerType && it->second.height == alçada){
            idValue = it->first;
            idFound = true;
        }
        it++;
    }
    return idValue;
}

bool IsFingerPositionActive(Vector fingerDirection, Vector handNormalDirection){
    /*cout << "Direccio dit: " << fingerDirection.x << " " << fingerDirection.y << " " << fingerDirection.z << endl;
    cout << "Direccio dit: " << handNormalDirection.x << " " << handNormalDirection.y << " " << handNormalDirection.z << endl;
    */
    /*Vector v1 = handNormalDirection;
    Vector v2 = fingerDirection;
    v2.x = v1.x;
    v2.z = v1.z;
     float angle = calcular_angle(v1,v2);
    cout << "Angle:" << angle  << endl;
    bool test = (angle >= 30);
    cout<< test << endl;
    return test;*/
    cout << handNormalDirection.y << " " << fingerDirection.y << " " << (handNormalDirection.y -fingerDirection.y) << endl;
    return ((handNormalDirection.y -fingerDirection.y)  > 10);


}

set<int> ConvertDataToNote(const vector<DataToTreat>& leapMotionData, vector<bool>& activeNotes, const map<int, musical_note_data>& infoFinger){
    set<int> ActiveFingers;
    for (int i = 0; i < leapMotionData.size(); i++){
        DataToTreat handInformation = leapMotionData[i];
        if (handInformation.h_id != -1){
            cout << "Ma vàlida!" << endl;
            for (int j = 0; j < handInformation.ftype.size(); j++){
                //if (handInformation.ftype[j].first != 0 &&  && IsFingerPositionActive(handInformation.ftype[j].second, handInformation.h_normal)){
                bool isRightHand = handInformation.right;
                int fingerType = handInformation.ftype[j].first;
                Vector h_position = handInformation.h_position;
                bool toBeActivated = (handInformation.ftype[j].first != 0  && IsFingerPositionActive(handInformation.ftype[j].second, handInformation.h_normal));
                int id_note = ObtainIDNoteActiveFinger(isRightHand, fingerType, h_position, infoFinger);
                if (id_note != -1){
                    if (!activeNotes[id_note] && toBeActivated){
                        ActiveFingers.insert(id_note);
                        activeNotes[id_note] = true;
                    }
                    else if (activeNotes[id_note] && !toBeActivated){
                        activeNotes[id_note] = false;
                    }
                }
            }
        }
    }
    return ActiveFingers;
}

void GetNewStruct (LeeMotion &leapMotion, pair<DataToTreat, DataToTreat> &data) {
    HandList hl = leapMotion.getHands();
    data.first.h_id = data.second.h_id = -1; //In first case, does not exist or is not valid...
    if (hl.begin() != hl.end()) {
        HandList::const_iterator hdit = hl.begin();
        Hand lh, rh;
        while (hdit != hl.end()) {
            if ((*hdit).isLeft()) {
                lh = *hdit;
                data.first.h_id = lh.id();
                data.first.right = false;
            }
            else {
                rh = *hdit;
                data.second.h_id = rh.id();
                data.second.right = true;
            }
            ++hdit;
        }
        //cout << "valido? " << lh.isValid() << ' ' << "valido?" << rh.isValid() << endl;
        FingerList left_fl = leapMotion.getFingers(lh);
        FingerList right_fl = leapMotion.getFingers(rh);
        FingerList::const_iterator it1 = left_fl.begin();
        FingerList::const_iterator it2 = right_fl.begin();
        //cout << "id: " << lh.id() << ' ' << rh.id() << endl;
        int i = 0;
        while (it1 != left_fl.end() and i < 5) {
            Finger f = *it1;
            pair<int, Vector> to_push;
            to_push.first = f.type();
            to_push.second = leapMotion.getFingerDirection(f);
            data.first.ftype.push_back(to_push);
            ++i; ++it1;
        }

        i = 0;
        while (it2 != right_fl.end() and i < 5) {
            Finger f = *it2;
            pair<int, Vector> to_push;
            to_push.first = f.type();
            to_push.second = leapMotion.getFingerDirection(f);
            data.second.ftype.push_back(to_push);;
            ++i; ++it2;
        }

        data.first.h_position = lh.palmPosition();
        data.second.h_position = rh.palmPosition();

        data.first.h_normal = leapMotion.getHandNormal(lh);
        data.second.h_normal = leapMotion.getHandNormal(rh);

        Vector mi = data.first.h_position;
        Vector md = data.second.h_position;
        //cout << "Mano Izquierda: " << mi.x << ' ' << mi.y << ' ' << mi.z << endl;
        //cout << "Mano Derecha: " << md.x << ' ' << md.y << ' ' << md.z << endl;

    }
}


vector<DataToTreat> ConvertPairToVector (const pair<DataToTreat, DataToTreat> data) {
    vector <DataToTreat> dtt(2);
    dtt [0] = data.first;
    dtt [1] = data.second;

    return dtt;
}

int main(){

    /*map<int, musical_note_data>::iterator ita;
    for (ita = note_data.begin(); ita != note_data.end(); ++ita) {
        cout << ita->first << " " << st
        d::string(ita->second.route) << endl;
    }*/
    map<int, musical_note_data> note_data = read_data_from_txt("DB_sounds.txt");
    LeeMotion leapMotion;
    Reproduce_music music_player;

    pair<DataToTreat, DataToTreat> data; //Respectively, LEFT AND RIGHT.
    while(!leapMotion.isConnected()){};
    set<int> notesToReproduce;
    vector<bool> activeNotes(25, false);

    /*map<int, musical_note_data>::const_iterator it3 = note_data.begin();
    while(it3 != note_data.end()){
        cout << it3->first << " " << it3->second.hand << " " << it3->second.finger << " " << it3->second.height << endl;
        it3++;
    }*/

    //En el codigo
    signal(SIGALRM, f);
    while (1){
        leapMotion.updateFrame();
        HandList hl = leapMotion.getHands();
        HandList::const_iterator it = hl.begin();
        FingerList fl = leapMotion.getFingers(*it);
        FingerList::const_iterator cit = fl.begin();
        Vector v = leapMotion.getFingerDirection(*cit);
        //cout << "x: " << v.x << " y: " << v.y << " z: " << v.z << endl;
        Vector pos = (*it).palmPosition();
        cout << "x: " << pos.x << " y: " << pos.y << " z: " << pos.z << endl;

        float angle = (180 * leapMotion.getHandNormal(*it).angleTo(leapMotion.getHandDirection(*it)/PI));
        cout << "Angle:" << angle << " " << int(angle)  << endl;

        angle = calcular_angle(leapMotion.getHandNormal(*it), leapMotion.getHandDirection(*it));
        cout << "Angle:" << angle << " " << int(angle)  << endl;
        GetNewStruct(leapMotion, data);
        vector<DataToTreat> leapMotionData = ConvertPairToVector(data);
        notesToReproduce = ConvertDataToNote(leapMotionData, activeNotes, note_data);
        if (notesToReproduce.begin() != notesToReproduce.end()){
            cout << "Reproduce!" << endl;
        }
        music_player.update_musical_notes(notesToReproduce);
        music_player.play_musical_notes();
        //alarm(10);
        //pause();
    }
}

/*while(not leapMotion.isConnected()){};
    while (1){
        leapMotion.updateFrame();
        HandList hl = leapMotion.getHands();
        HandList::const_iterator it = hl.begin();
        FingerList fl = leapMotion.getFingers(*it);
        FingerList::const_iterator cit = fl.begin();
        Vector v = leapMotion.getFingerDirection(*cit);
        //cout << "x: " << v.x << " y: " << v.y << " z: " << v.z << endl;
        Vector pos = (*it).palmPosition();
        cout << "x: " << pos.x << " y: " << pos.y << " z: " << pos.z << endl;
    }*/
