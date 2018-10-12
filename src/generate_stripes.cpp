#include <generate_stripes.h>

GenerateStripes::GenerateStripes(string img_path, int _stripes_n) {
    
    ori_img = cv::imread(img_path);
    ori_img_size = ori_img.size();
    stripes_n = _stripes_n;

    seg_stripes();

}

bool GenerateStripes::seg_stripes() {

    double seg_step = (double)ori_img_size.width / stripes_n;

    for (int i = 0; i < stripes_n; i++) {

    }

    return true;

}