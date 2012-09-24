#include <cassert>
#include <LigOrgan.hpp>
#include <Smb.hpp>
#include <BBox.hpp>

using namespace std;

LigOrgan::LigOrgan(const LigFileFormat &organ)
{
    id_ = organ.symb;
    visibility_ = organ.numero_element;
    smb_ = 0;
    float scale[4] = {1.0, organ.var2, organ.var2, 1.0};
    for (int i = 0; i < 4; ++i) {
        transform_matrix_[i][0] = organ.x[i] * scale[i];
        transform_matrix_[i][1] = organ.y[i] * scale[i];
        transform_matrix_[i][2] = organ.z[i] * scale[i];
        transform_matrix_[i][3] = 0.0;
    }
    transform_matrix_[3][3] = 1.0;
}

BBox LigOrgan::boundingBox() const
{
    BBox ret(0.0, 0.0, 0.0, 1.0, 0.0, 0.0);
    if (smb_ != 0)
        ret.surround(smb_->boundingBox());
    return ret.transform(transform_matrix_);
}

void LigOrgan::print(const char *identifier, bool visibility_out) const
{
    printf("object {\n");
    printf("   %s\n", identifier);
    if (visibility_out && id_ == 10) 
        printf("   texture { pigment { color red %.1f green %.1f } }\n", visibility_ / 10.0, 1 - visibility_ / 10.0);
    printf("   matrix <%f, %f, %f,\n", transform_matrix_[0][0], transform_matrix_[0][1], transform_matrix_[0][2]);
    printf("           %f, %f, %f,\n",  transform_matrix_[1][0], transform_matrix_[1][1], transform_matrix_[1][2]);
    printf("           %f, %f, %f,\n",  transform_matrix_[2][0], transform_matrix_[2][1], transform_matrix_[2][2]);
    printf("           %f, %f, %f>\n",  transform_matrix_[3][0], transform_matrix_[3][1], transform_matrix_[3][2]);
    printf("}\n");
}
