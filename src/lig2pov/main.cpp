#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <list>
#include <cmath>
#include <boost/bind.hpp>
#include "Smb.hpp"
#include "LigOrgan.hpp"
#include "MyException.hpp"
#include "BBox.hpp"

using namespace std;
using boost::shared_ptr;

void GetAllSmb(map<long, shared_ptr<AMapSmb> >& all_smb, const char* dsp_filename)
{
    ifstream f(dsp_filename);
    if (!f) {
        cout << "Can not open dsp file";
        exit(-1);
    }

    try {
        while (!f.eof()) {
            long id;
            string smb_filename;
            string texture_filename;
            f >> id >> smb_filename >> texture_filename;
            if (f)
                all_smb[id] = shared_ptr<AMapSmb>(new AMapSmb(smb_filename.c_str(), texture_filename.c_str()));
        }
    } catch(...) {
        f.close();
        throw;
    }
}

static double square(double x)
{
    return x * x;
}

void PrintCameraAndLight(const BBox& bbox)
{
    double r = sqrt(square(bbox.maxX() - bbox.minX()) +\
                    square(bbox.maxY() - bbox.minY()) +\
                    square(bbox.maxZ() - bbox.minZ())) / 2.0;
    double x = (bbox.minX() + bbox.maxX()) / 2.0;
    double y = (bbox.minY() + bbox.maxY()) / 2.0;
    double z = (bbox.minZ() + bbox.maxZ()) / 2.0;
    printf("#include \"colors.inc\"\n\n");
    printf("light_source {\n");
    printf("   <%f, %f, %f>\n", x - 2 * sqrt(2.0) * r, y - 2 * sqrt(2.0) * r, z + 2 * sqrt(2.0) * r);
    printf("   color White\n");
    printf("   spotlight\n");
    printf("   radius %f\n", atan(0.1) * 180.0 / atan(1.0) / 4);
    printf("   falloff 20\n");
    printf("   tightness 10\n");
    printf("   point_at <%f, %f, %f>\n", x, y, z);
    printf("}\n");
    printf("global_settings { ambient_light rgb<1, 1, 1> }\n\n");
    printf("camera {\n");
    printf("   location <%f, %f, %f>\n", (bbox.minX() + bbox.maxX()) / 2.0, 
                                         (bbox.minY() + bbox.maxY()) / 2.0 - 10 * r, 
                                         (bbox.minZ() + bbox.maxZ()) / 2.0);
    printf("   angle %f", 2.0 * atan(0.1) * 180.0 / atan(1.0) / 4 * 1.5);
    printf("   direction <0.0, 1.0, 0.0>\n");
    printf("   up z\n");
    printf("}\n");
}

int main(int argc, char* argv[])
{
    if (argc != 3 || (string(argv[1]) != "-visibility" && string(argv[1]) != "-texture")) {
        printf("Usage: lig2pov.exe -texture[-visibility] filename.lig");
        return -1;
    }

    bool visibility_out = false;
    if (string(argv[1]) == "-visibility")
        visibility_out = true;

    FILE *lig_file = fopen(argv[2], "rb");
    if (lig_file == NULL) {
        printf("Can not open lig file");
        return -1;
    }
    fseek(lig_file, 80, SEEK_SET);
    list<LigOrgan> organ_list;
    LigFileFormat one_lig;
    while (fread(&one_lig, sizeof(LigFileFormat), 1, lig_file) == 1) {
        organ_list.push_back(one_lig);
    }
    fclose(lig_file);

    BBox bbox;
    for (list<LigOrgan>::iterator it = organ_list.begin(); it != organ_list.end(); ++it)
        bbox.surround(it->boundingBox());

    // PrintCameraAndLight(bbox);
    // printf("\n");

    string dsp_filename(argv[2]);
    dsp_filename.replace(dsp_filename.rfind("."), 4, ".dsp");
    map<long, shared_ptr<AMapSmb> > smb_table;
    GetAllSmb(smb_table, dsp_filename.c_str());
    for (map<long, shared_ptr<AMapSmb> >::const_iterator 
        it = smb_table.begin(); it != smb_table.end(); ++it) {
        it->second->print();
        printf("\n");
    }

    for (list<LigOrgan>::iterator it = organ_list.begin(); it != organ_list.end(); ++it) {
        if (smb_table.find(it->id()) == smb_table.end()) {
            printf("Miss smb in dsp file: id %ld", it->id());
            exit(-1);
        }
        it->print(((smb_table[it->id()])->identifier()).c_str(), visibility_out);
        printf("\n");
    }
    return 0;
}
