//
// Created by nikita on 06.07.17.
//

#include "interface.h"
#include "io.h"
#include "filters.h"
#include "core.h"

namespace interface {
    filter::Compose build_filters() {
        filter::GrayScale *gs = new filter::GrayScale();
        filter::Blur *blur = new filter::Blur(/*sigma = */ 3);
        filter::Canny *canny = new filter::Canny(/*low_threshold = */ 30, /*ratio = */ 3, /*kernel_size = */ 3);
        filter::Sobel *sbl = new filter::Sobel(0, 0, 3, 1, 0, CV_16S, cv::BORDER_DEFAULT);
        std::vector<filter::Filter *> filters = {gs, blur, canny};
        filter::Compose compose(filters);
        return compose;
    }

    bool
    Resize::is_init() const {
        return vseams.size() != 0;
    }

    Resize::Resize() {
        filter = build_filters();
    }

    void
    Resize::init(cv::Mat&& in) {
        for (auto seam : vseams) delete seam;
        image = core::MatWrp(in);
        core::MatWrp energy(in);
        filter(in, energy);
        vseams = core::get_seams(energy);
        hseams = core::get_seams(energy);
    }

    Resize::~Resize() {}

    void
    Resize::process(cv::Mat& result, cv::Size new_size) {
        cv::Size in_size = image.mat.size();
        core::MatWrp in_wrp(image.clone());

        core::resize_with_seams(in_wrp, in_size.width - new_size.width, vseams);
        in_wrp.transpose();
//        core::resize_with_filter(in_wrp, in_size.height - new_size.height, filter);
        core::resize_with_seams(in_wrp, in_size.height - new_size.height, hseams);
        result = in_wrp.mat;
    }


    void process_image(io::Input in, io::Output out, cv::Size size, bool show_images) {
        cv::Mat input_matrix = in.read_image();
        cv::Mat output_matrix = in.read_image();

        auto filter = build_filters();

        core::resize_to_fit(input_matrix, output_matrix, size, filter);

        out.write_image(output_matrix);
    }

}