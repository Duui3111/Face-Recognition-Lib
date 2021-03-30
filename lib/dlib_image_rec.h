#pragma once

#define DLIB_TEST_FOR_ODR_VIOLATIONS_H_
#include "3rdparty/include/dlib/dnn/loss.h"
#include "3rdparty/include/dlib/dnn/input.h"
#include "3rdparty/include/dlib/dnn/layers.h"
#include "3rdparty/include/dlib/image_loader/load_image.h"
#include "3rdparty/include/dlib/image_processing/frontal_face_detector.h"

#pragma comment (lib, "3rdparty/lib/dlib19.21.0_debug_64bit_msvc1928.lib")

using namespace dlib;
template <template <int, template<typename>class, int, typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual = add_prev1<block<N, BN, 1, tag1<SUBNET>>>;

template <template <int, template<typename>class, int, typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual_down = add_prev2<avg_pool<2, 2, 2, 2, skip1<tag2<block<N, BN, 2, tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET>
using block = BN<con<N, 3, 3, 1, 1, relu<BN<con<N, 3, 3, stride, stride, SUBNET>>>>>;

template <int N, typename SUBNET> using ares = relu<residual<block, N, affine, SUBNET>>;
template <int N, typename SUBNET> using ares_down = relu<residual_down<block, N, affine, SUBNET>>;
template <typename SUBNET> using alevel0 = ares_down<256, SUBNET>;
template <typename SUBNET> using alevel1 = ares<256, ares<256, ares_down<256, SUBNET>>>;
template <typename SUBNET> using alevel2 = ares<128, ares<128, ares_down<128, SUBNET>>>;
template <typename SUBNET> using alevel3 = ares<64, ares<64, ares<64, ares_down<64, SUBNET>>>>;
template <typename SUBNET> using alevel4 = ares<32, ares<32, ares<32, SUBNET>>>;

using anet_type = dlib::loss_metric<dlib::fc_no_bias<128, dlib::avg_pool_everything<alevel0<alevel1<alevel2<alevel3<alevel4<dlib::max_pool<3, 3, 2, 2, dlib::relu<dlib::affine<dlib::con<32, 7, 7, 2, 2, dlib::input_rgb_image_sized<150>>>>>>>>>>>>>;

bool __IsFaceRec = false;

int Face_Image_Recognition(std::string image1, std::string image2)
{
    dlib::frontal_face_detector detector;
    dlib::shape_predictor sp;
    anet_type net;

    detector = dlib::get_frontal_face_detector();
    dlib::deserialize("./dat/shape_predictor_5_face_landmarks.dat") >> sp;
    dlib::deserialize("./dat/dlib_face_recognition_resnet_model_v1.dat") >> net;

    dlib::matrix<dlib::rgb_pixel> img;
    dlib::matrix<dlib::rgb_pixel> img2;

    dlib::load_image(img, image1);
    dlib::load_image(img2, image2);
    double a1 = std::min(img.nc(), img.nr());
    double a2 = std::min(img2.nc(), img2.nr());
    resize_image(250 / a1 * 1, img);
    resize_image(250 / a2 * 1, img2);

    std::vector<dlib::matrix<dlib::rgb_pixel>> faces1;
    double s1 = clock();

    for (auto face : detector(img, 0))
    {
        auto shape = sp(img, face);
        dlib::matrix<dlib::rgb_pixel> face_chip;
        extract_image_chip(img, get_face_chip_details(shape, 150, 0.25), face_chip);
        faces1.push_back(std::move(face_chip));
    }

    std::vector<dlib::matrix<dlib::rgb_pixel>> faces2;
    for (auto face : detector(img2, 0))
    {
        auto shape = sp(img2, face);
        dlib::matrix<dlib::rgb_pixel> face_chip;
        extract_image_chip(img2, get_face_chip_details(shape, 150, 0.25), face_chip);
        faces2.push_back(std::move(face_chip));
    }

    double s2 = clock();
    std::cout << "# Time detect faces: " << (s2 - s1) / CLOCKS_PER_SEC << std::endl;
    std::cout << "The number of faces found in the first image: " << faces1.size() << std::endl;
    std::cout << "The number of faces found in the second image: " << faces2.size() << std::endl;

    std::vector<dlib::matrix<float, 0, 1>> face_descriptors1 = net(faces1);
    std::vector<dlib::matrix<float, 0, 1>> face_descriptors2 = net(faces2);
    int flag = 0;

    for (const auto& i : face_descriptors1)
    {
        for (const auto& j : face_descriptors2)
        {
            std::cout << "The distance of two faces: " << length(i - j) << std::endl;
            if (length(i - j) < (double)0.6)
            {
                flag = 1;
                __IsFaceRec = true;
                std::cout << "The two pictures have the same face" << std::endl;
                return 4;
            }
        }
    }

    double s3 = clock();
    std::cout << "# Time run Resnet: " << (s3 - s2) / CLOCKS_PER_SEC << std::endl;
    std::cout << "The two pictures don't have the same face" << std::endl;
}