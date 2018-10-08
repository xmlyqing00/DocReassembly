#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

int main()
{

    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    // Initialize tesseract-ocr with English, without specifying tessdata path
    if (api->Init(NULL, "eng+chi_sim")) {
        fprintf(stderr, "Could not initialize tesseract.\n");
        exit(1);
    }

    // Open input image with leptonica library
    Pix *image = pixRead("./data/test0.png");
    api->SetImage(image); 

    // Get OCR result
    // char * outText = api->GetUTF8Text();
    // int conf = api->MeanTextConf();
    // printf("OCR output:\n%s", outText);
    // printf("Confidence: %d\n", conf);
    // delete [] outText;

    api->Recognize(0);
    tesseract::ResultIterator* ri = api->GetIterator();
    tesseract::PageIteratorLevel level = tesseract::RIL_SYMBOL;
    if (ri != 0) {
        do {
        const char* word = ri->GetUTF8Text(level);
        float conf = ri->Confidence(level);
        int x1, y1, x2, y2;
        ri->BoundingBox(level, &x1, &y1, &x2, &y2);
        printf("word: '%s';  \tconf: %.2f; BoundingBox: %d,%d,%d,%d;\n",
                   word, conf, x1, y1, x2, y2);
        delete[] word;
        } while (ri->Next(level));
    }

    // Destroy used object and release memory
    api->End();
    pixDestroy(&image);

    return 0;
}
