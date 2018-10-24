#include <main.h>

int main(int argc, char ** argv) {

    // Default parameters
    string case_name = "test0";
    int stripes_n = 4;
    bool generate_flag = false;
    Stripes::Composition comp_mod = Stripes::GREEDY;

    // Parse command line parameters
    const string opt_str = "t:T:n:N:gG";
    int opt = getopt(argc, argv, opt_str.c_str());

    while (opt != -1) {
        switch (opt) {
            case 't': case 'T':
                case_name = string(optarg);
                break;
            case 'n': case 'N':
                stripes_n = atoi(optarg);
                break;
            case 'g': case 'G':
                generate_flag = true;
                break;
        }
        
        opt = getopt(argc, argv, opt_str.c_str());
    }

    cout << "Case name:\t\t" << case_name << endl;
    cout << "Stripes num:\t\t" << stripes_n << endl;
    cout << "Generate stripes:\t" << boolalpha << generate_flag << endl;

    // Generate new stripes
    const string stripes_folder = "data/stripes/" + case_name + "_" + to_string(stripes_n) + "/";

    if (generate_flag || access(stripes_folder.c_str(), 0) == -1) {
        const string gt_folder = "data/gt/";
        const string gt_img_path = gt_folder + case_name + ".png";
        cv::Mat gt_img = cv::imread(gt_img_path);

        StripesGenerator stripes_generator(gt_img_path, stripes_n);
        stripes_generator.save_stripes(stripes_folder);
    }

    // Import stripes
    // const string model_path = "data/models/";
    const string model_path = "";
    Stripes stripes(model_path);

    for (int i = 0; i < stripes_n; i++) {
        const string stripe_img_path = stripes_folder + to_string(i) + ".png";
        cv::Mat stripe_img = cv::imread(stripe_img_path);
        stripes.push(stripe_img);
    }

    stripes.reassemble(comp_mod);
    for (const int idx: stripes.comp_idx) {
        cout << idx << endl;
    }
    cv::imshow("comp_img", stripes.comp_img);
    cv::waitKey();
    
    
    // ocr->SetImage(img.data, img.cols, img.rows, 3, img.step);

    // std::string out_text(ocr->GetUTF8Text());

    // std::cout<< out_text << std::endl;

    // ocr->End();

    
    // for (int i = 0; i < generate_stripes.stripes.size(); i++) {
        // cv::imshow("stripe", generate_stripes.stripes[i]);
        // cv::waitKey(0);
    // }

    // printf("start\n");

    // // Open input image with leptonica library
    // Pix *image = pixRead("./data/test0.png");
    // api->SetImage(image); 

    // // // Get OCR result
    // char * outText = api->GetUTF8Text();
    // int conf = api->MeanTextConf();
    // printf("OCR output:\n%s", outText);
    // printf("Confidence: %d\n", conf);
    // delete [] outText;

    // // // api->Recognize(0);
    // // // int b;
    // // // tesseract::ResultIterator* ri = api->GetIterator();
    // // // tesseract::PageIteratorLevel level = tesseract::RIL_WORD;
    // // // if (ri != 0) {
    // // //     do {
    // // //     const char* word = ri->GetUTF8Text(level);
    // // //     float conf = ri->Confidence(level);
    // // //     int x1, y1, x2, y2;
    // // //     ri->BoundingBox(level, &x1, &y1, &x2, &y2);
    // // //     printf("word: '%s';  \tconf: %.2f; BoundingBox: %d,%d,%d,%d;\n",
    // // //                word, conf, x1, y1, x2, y2);
    // // //     delete[] word;
    // // //     } while (ri->Next(level));
    // // // }

    // // Destroy used object and release memory
    // api->End();
    // pixDestroy(&image);

    return 0;
}
