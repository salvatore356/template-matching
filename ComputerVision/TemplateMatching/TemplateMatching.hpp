//
//  TemplateMatching.hpp
//  ComputerVision
//
//  Created by Salvatore on 9/8/23.
//

#ifndef TemplateMatching_hpp
#define TemplateMatching_hpp

#include <stdio.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <limits>


#include "../Utils/Utils.hpp"

namespace TemplateMatching {

    struct Match {
        int r = -1, c = -1;
        double correlation = numeric_limits<double>::min();
    };

    Pixel calculateCorrelation(
       Pixel &imgsProduct, Pixel &imgSum, Pixel &tempSum,
       int templateHeight, int templateWidth
    ) {
        Pixel imgSum2 = imgSum * imgSum;
        Pixel tempSum2 = tempSum * tempSum;
        double WxH = templateWidth * templateHeight;
        
        Pixel numerator = imgsProduct - (imgSum * tempSum) / (WxH);
        Pixel denominator = ((imgSum2 - (tempSum2 * WxH)) * (tempSum2 - (tempSum2 * WxH))).sqrt();
        
        return numerator / denominator;
    }

    double calculatePixelCorrelation(Image &img, Image &tempImg, int r, int c) {
        
        Pixel imgSum = { 0.0, 0.0, 0.0 };
        Pixel tempSum = { 0.0, 0.0, 0.0 };
        Pixel imgsProduct = { 0.0, 0.0, 0.0 };
        
        for (int rr = 0; rr < tempImg.height; ++rr) {
            for (int cc = 0; cc < tempImg.width; ++cc) {
                
                Pixel pixImg = img.at(r + rr, c + cc);
                Pixel pixTemp = tempImg.at(rr, cc);
                
                // Calculate correlation using RGB channels
                imgSum += (pixImg * pixImg);
                tempSum += (pixTemp * pixTemp);
                imgsProduct += (pixImg * pixTemp);
                
            }
        }
        
        auto cor = calculateCorrelation(imgsProduct, imgSum, tempSum, tempImg.height, tempImg.width);
        return cor.avg();
    }

    Match findInImage(Image &img, Image &tempImg, string resultPath) {
        
        Match bestMatch;
        
        double bestMatchScore = 0.0;
        Image correlationImg(img.height, img.width, 3);
        
        for (int r = 0; r <= img.height - tempImg.height; ++r) {
            for (int c = 0; c <= img.width - tempImg.width; ++c) {
                
                double correlation = calculatePixelCorrelation(img, tempImg, r, c);
                
                //printf("correlation %.20f\n", correlation);
                correlationImg.at(r, c) = { (float)correlation, (float)correlation, (float)correlation };
                
                if (correlation > bestMatch.correlation) {
                    bestMatch.c = c;
                    bestMatch.r = r;
                    bestMatch.correlation = correlation;
                }
            }
        }
        
        if(bestMatch.r != -1 && bestMatch.c != -1) {
            normalizeImage(correlationImg, bestMatch.correlation);
            writeJPG(correlationImg, resultPath + "_normalized.jpg");
            drawRectangle(img, bestMatch.r, bestMatch.c, tempImg.height, tempImg.width);
            writeJPG(img, resultPath + "_solution.jpg");
        }
        
        printf("Best match found at (%d, %d) correlation %.20f\n", bestMatch.r, bestMatch.c, bestMatchScore);
        return bestMatch;
    }



    void test(string scene, string temp, string result){
        auto img = readJPG(scene);
        auto waldo = readJPG(temp);
        
        findInImage(*img, *waldo, result);
    }

    void test() {
        printf("Test case 1....\n");
        test("waldo-scene.jpg", "waldo.jpg", "waldo");
        
        printf("Test case 2....\n");
        test("eyes.jpg", "eye-1.jpg", "eye-1");
        
        printf("Test case 3....\n");
        test("eyes.jpg", "eye-2.jpg", "eye-2");
    }

}
#endif /* TemplateMatching_hpp */
