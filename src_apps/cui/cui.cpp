#include "console_handler.cpp"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

 #include "opencv2/objdetect/objdetect.hpp"
 #include "opencv2/highgui/highgui.hpp"
 #include "opencv2/imgproc/imgproc.hpp"
 #include <iostream>
 #include <stdio.h>


using namespace xinar_utils;
using namespace cv;

int main(int argc, char **argv) {
    auto config = Singleton<io::Config>::Instance();
    po::options_description desc = build_options();
    po::variables_map vm;
    try {
        update_vm(argc, argv, vm, desc);
    } catch (std::string error) {
        std::cerr << error << '\n';
        return 1;
    }
    po::notify(vm);

    if (argc == 2 && vm.count("DEBUG")) {
        run_debug_code();
        return 0;
    }

    if (argc == 1 || vm.count("help")) {
        std::cout << desc << '\n' << help_page() << std::endl;
        return 0;
    }

    if (vm.count("config")) {
        config.print(std::cout);
        return 0;
    }

    xinar_utils::io::Input in;
    xinar_utils::io::Input maskin;
    xinar_utils::io::Output out;

    if (vm.count("input-file")) {
        in = io::bind_input(vm["input-file"].as<std::string>());
    } else {
        std::cerr << "No input file providen!\n";
        return 1;
    }

    auto image = in.read_image();
    cv::Mat image_gray;
    
    //Generate Face Mask
    auto canvas = image.clone();


    if (vm.count("mask")) {
        maskin = io::bind_input(vm["mask"].as<std::string>());
        canvas = maskin.read_image();
    } else{
        canvas=cv::Scalar(255, 255, 255);
        CascadeClassifier face_cascade;
        face_cascade.load("haarcascade_frontalface_alt.xml" );
        std::vector<Rect> faces;
        cvtColor(image, image_gray, COLOR_BGR2GRAY);
        equalizeHist(image_gray, image_gray);
        face_cascade.detectMultiScale( image_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(80, 80) );
        for( int i = 0; i < faces.size(); i++ ){
            Point center( faces[i].x + faces[i].width/2, faces[i].y + faces[i].height/2 );
            ellipse(canvas, center, Size( faces[i].width/2, faces[i].height/2), 0, 0, 360, Scalar( 0, 0, 0 ),-1, CV_AA );
        }
    }

    if (vm.count("output-file")) {
        out = io::bind_output(vm["output-file"].as<std::string>());
    } else {
        out = io::bind_output(create_default_filepath(in.get_path()));
    }



    if (image.empty()) {
        std::cerr << "Error while opening file. Passed: \"" << in.get_path() << "\"\n";
        return 1;
    }

    cv::Size size(image.rows, image.cols);

    if (vm.count("preset")) {
        auto parameter = vm["preset"].as<std::string>();
        try {
            size = update_size(parameter);
        } catch (char *e) {
            std::cout << e;
            return 1;
        }

    } else if (vm.count("width") || vm.count("height")) {
        size = update_size(vm, size);
    } else {
        std::cerr << "No size providen. Use either --width/--height or --preset command. --help for more info\n";
        return 1;
    }
    if (!check_dims(image, size)) {
        std::cerr << "Image can't be resized to " << size
                  << ". Possible dimentions: " << possible_dims(image) << ". --help to more info\n";
        return 1;
    }

    cv::Mat output;

    // out.write_image(canvas);
    xinar::maskresize(image, canvas, output, size);
    out.write_image(output);
    return 0;
}