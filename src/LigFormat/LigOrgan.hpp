#ifndef _LIG_ORGAN_H_
#define _LIG_ORGAN_H_

#include <map>
#include <boost/shared_ptr.hpp>

struct LigFileFormat {
    long   symb;
    long   nument;
    long   numbra;
    long   portee;
                    
    float  x[4];
    float  y[4];
    float  z[4];
    float  var1;
    float  var2;
    long   numero_element;
};

class AMapSmb;
class BBox;

class LigOrgan {
public:
    LigOrgan(const LigFileFormat& organ);
    long id() const { return id_; }
    BBox boundingBox() const;
    void setSmb(AMapSmb *smb) { smb_ = smb; }
    void print(const char* identifier, bool visibility_out=false) const;

private:
    long id_;
    long visibility_;
    AMapSmb *smb_;
    float transform_matrix_[4][4];
};

#endif
