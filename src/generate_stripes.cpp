#include <generate_stripes.h>

GenerateStripes::GenerateStripes(string img_path, int _stripes_n) {
    
    ori_img = cv::imread(img_path);
    ori_img_size = ori_img.size();
    stripes_n = _stripes_n;

    seg_stripes();

}

bool GenerateStripes::seg_stripes() {

    int seg_step = int((double)ori_img_size.width / stripes_n);

    int seg_st = 0;
    for (int i = 0; i < stripes_n; i++) {

        int seg_ed = seg_st + seg_step;
        if (i == stripes_n - 1) {
            seg_ed = ori_img_size.width;
        }

        cv::Mat stripe = ori_img(cv::Rect(seg_st, 0, seg_ed - seg_st, ori_img_size.height));
        stripes.push_back(stripe.clone());

        seg_st = seg_ed;
        
    }

    return true;

}