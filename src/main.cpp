#include <main.h>

int main(int argc, char ** argv) {

    string img_path = "data/test0.png";
    int stripes_n = 4;

    switch (argc) {
        case 3:
            stripes_n = atoi(argv[2]);
        case 2:
            img_path = argv[1];
    }

    cv::Mat img = cv::imread(img_path);
    
    tesseract::TessBaseAPI *ocr = new tesseract::TessBaseAPI();
    if (ocr->Init(NULL, "eng")) {
        fprintf(stderr, "Could not initialize tesseract.\n");
        return 1;
    }
    ocr->SetImage(img.data, img.cols, img.rows, 3, img.step);

    std::string out_text(ocr->GetUTF8Text());

    std::cout<< out_text << std::endl;

    ocr->End();

    GenerateStripes generate_stripes = GenerateStripes(img_path, stripes_n);
    cv::imshow("whole", generate_stripes.ori_img);
    for (int i = 0; i < generate_stripes.stripes.size(); i++) {
        cv::imshow("stripe", generate_stripes.stripes[i]);
        cv::waitKey(0);
    }

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
