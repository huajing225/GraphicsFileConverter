#include <cassert>
#include <cstdio>
#include <numeric>
#include <functional>
#include <boost/bind.hpp>
#include <MyException.hpp>
#include <Smb.hpp>

using namespace std;
using boost::bind;

class PointInfoInFace {
public:
    void read(FILE *f);

public:
    long col;
    long index;
    unsigned short att;
};

void PointInfoInFace::read(FILE *f)
{
    if (fread(&col, sizeof(long), 1, f) != 1)
        throw SmbFormatError();
    if (fread(&index, sizeof(long), 1, f) != 1)
        throw SmbFormatError();
    if (fread(&att, sizeof(unsigned short), 1, f) != 1)
        throw SmbFormatError();
}

void SmbFace::read(FILE *f, long max_index)
{
    if (fread(&num_points, sizeof(long), 1, f) != 1)
        throw SmbFormatError();
    if (fread(&type, sizeof(unsigned short), 1, f) != 1)
        throw SmbFormatError();
    if (fread(&dim, sizeof(unsigned short), 1, f) != 1)
        throw SmbFormatError();
    point_index.reset(new long[num_points]);
    for (int i = 0; i < num_points; ++i) {
        PointInfoInFace tmp;
        tmp.read(f);
        if (tmp.index >= max_index)
            throw SmbFormatError();
        point_index[i] = tmp.index;
    }
}


AMapSmb::AMapSmb(const char* filename, const char* texture_filename)
: filename_(filename), texture_filename_(texture_filename)
{
    FILE *f = fopen((string("./smb/") + filename).c_str(), "rb");
    if (f == NULL)
        throw FileError(filename);
    try {
        if (fread(&num_faces_, sizeof(long), 1, f) != 1)
            throw SmbFormatError();
        if (fread(&num_points_, sizeof(long), 1, f) != 1)
            throw SmbFormatError();

        faces_.reset(new SmbFace[num_faces_]);
        for (int i = 0; i < num_faces_; ++i)
            faces_[i].read(f, num_points_);

        points_.reset(new float[num_points_ * 9]);
        if (fread(&points_[0], sizeof(float), num_points_ * 9, f) != num_points_ * 9)
            throw SmbFormatError();

        for (int i = 0; i < num_points_; ++i) {
            long beg = 9 * i;
            bbox_.surround(points_[beg + 3], points_[beg + 4], points_[beg + 5]);
        }
    } catch (...) {
        fclose(f);
        throw;
    }
    fclose(f);
}

void AMapSmb::printVertex() const
{
    printf("   vertex_vectors {\n");
    printf("      %d,", num_points_);
    for (int i = 0; i < num_points_; ++i) {
        if (i % 2 == 0)
            printf("\n      ");
        printf("<%f, %f, %f>", points_[9 * i + 3], points_[9 * i + 4], points_[9 * i + 5]);
        if (i != num_points_ - 1)
            printf(", ");
    }
    printf("\n   }\n");
}

void AMapSmb::printNormal() const
{
    printf("   normal_vectors {\n");
    printf("      %d,", num_points_);
    for (int i = 0; i < num_points_; ++i) {
        if (i % 2 == 0)
            printf("\n      ");
        printf("<%f, %f, %f>", points_[9 * i + 0], points_[9 * i + 1], points_[9 * i + 2]);
        if (i != num_points_ - 1)
            printf(", ");
    }
    printf("\n   }\n");
}

void AMapSmb::printTexture() const
{
    printf("   uv_vectors {\n");
    printf("      %d,", num_points_);
    for (int i = 0; i < num_points_; ++i) {
        if (i % 2 == 0)
            printf("\n      ");
        printf("<%f, %f>", points_[9 * i + 6], points_[9 * i + 7]);
        if (i != num_points_ - 1)
            printf(", ");
    }
    printf("\n   }\n");
}

void AMapSmb::printFace() const
{
    printf("   face_indices {\n");
    printf("      %d,", accumulate(&faces_[0], &faces_[0] + num_faces_, 0, 
                                   bind(plus<int>(), _1, bind(&SmbFace::meshNum, _2))));
    int count = 0;
    for (int i = 0; i < num_faces_; ++i) {
        long first = faces_[i].point_index[0];
        for (int j = 2; j < faces_[i].num_points; ++j) {
            if (count % 4 == 0)
                printf("\n      ");
            printf("<%ld, %ld, %ld>", first, faces_[i].point_index[j - 1], faces_[i].point_index[j]);
            if (i != num_faces_ - 1 || j != faces_[i].num_points - 1)
                printf(", ");
            ++count;
        }
    }
    printf("\n   }\n");
}

string AMapSmb::identifier() const
{
    string id(filename_);
    id.erase(id.rfind("."));
    return id;
}

void AMapSmb::print(bool visibility_out) const
{
    printf("#declare %s = \nmesh2 {\n", identifier().c_str());
    printVertex();
    printNormal();
    printTexture();
    printFace();

    printf("   uv_mapping\n");
    if (!visibility_out)
        printf("   pigment { png image_map {\"%s\"} }\n", texture_filename_.c_str());
    printf("   finish { ambient 0.4 diffuse 0.8 }\n");
    printf("}\n");
}
