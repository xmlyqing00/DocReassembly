#include <string>
#include <tesseract/baseapi.h>
#include <opencv2/opencv.hpp>

int main(int argc, char ** argv) {

    std::string img_path = argv[1];
    std::cout << argc << std::endl;
    std::cout << img_path << std::endl;

    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    
    printf("at\n");
    // Initialize tesseract-ocr with English, without specifying tessdata path
    if (api->Init(NULL, "eng")) {
        fprintf(stderr, "Could not initialize tesseract.\n");
        return 1;
    }

    cv::Mat img = cv::imread(img_path);
    api->SetImage(img.data, img.cols, img.rows, 3, img.step);

    std::string out_text(api->GetUTF8Text());

    std::cout<< out_text << std::endl;

    api->End();

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
