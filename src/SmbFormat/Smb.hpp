#ifndef _SMB_H_
#define _SMB_H_

#include <boost/utility.hpp>
#include <boost/scoped_array.hpp>
#include <string>
#include "BBox.hpp"

class SmbFace {
public:
    void read(FILE *f, long max_index);
    int meshNum() const { return num_points - 2; }

public:
    long num_points;
    unsigned short type;
    unsigned short dim;
    boost::scoped_array<long> point_index;
};


class AMapSmb: public boost::noncopyable {
public:
    AMapSmb(const char* filename, const char* texture_filename);
    BBox boundingBox() const { return bbox_; }
    void print(bool visibility_out=false) const;
    std::string identifier() const;

private:
    void printVertex() const;
    void printNormal() const;
    void printTexture() const;
    void printFace() const;

private:
    std::string filename_;
    std::string texture_filename_;
    long num_faces_;
    long num_points_;
    boost::scoped_array<SmbFace> faces_;
    boost::scoped_array<float> points_;
    BBox bbox_;
};

#endif
