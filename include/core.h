#include <opencv2/opencv.hpp>
#include <vector>
#include <boost/thread.hpp>

#ifndef CONTENTAWARERESIZE_CORE_H
#define CONTENTAWARERESIZE_CORE_H

namespace core {
    class MatWrp;

    typedef long long WeightData;
    typedef uchar EnergyData;
    typedef std::vector<cv::Point2i> Seem; // Path to add/remove along

    // Shrinks width and height of the in image to сorrespond with desirable size
    // image 640x640, new size 500x700 -> new image 500x640
    // image 640x640, new size 500x500 -> new image 500x500 etc.
    template <typename TFilter>
    void shrink_to_fit(const cv::Mat& in, cv::Mat& out, const cv::Size& new_size, TFilter filter, double quality);

    class MatWrp {
    private:
        bool transposed;
    public:
        cv::Mat mat;

        MatWrp();

        MatWrp(const MatWrp &other);

        MatWrp(const cv::Mat &other);

        MatWrp(int h, int w, int type);

        MatWrp clone() const;

        const int width() const;

        const int height() const;

        template<typename T>
        T &at(int i, int j);

        template<typename T>
        const T &at(int i, int j) const;

        void transpose();
        void set_shape(const MatWrp& other);
        void set_orientation(const MatWrp& other);
        MatWrp  operator() (cv::Range rowRange, cv::Range colRange) const;
        MatWrp& operator= (const MatWrp& other);
    };

    template <typename TData>
    TData&
    MatWrp::at(int i, int j) {
        return ((this->transposed) ? (this->mat).at<TData>(j, i) :
                (this->mat).at<TData>(i, j)); // Hello, opencv index style
    }

    template<typename TData>
    const TData &
    MatWrp::at(int i, int j) const {
        return ((this->transposed) ? (this->mat).at<TData>(j, i) :
                (this->mat).at<TData>(i, j)); // Hello, opencv index style
    }

    std::vector<Seem> get_seems(MatWrp& energy, int k);
    void remove_seems(MatWrp& from, std::vector<Seem>& seems);

    template <typename TFilter>
    void shrink(MatWrp& in, int delta, const TFilter& filter) {
        if (delta <= 0) return;
        MatWrp energy;
        energy.set_orientation(in);
        filter(in.mat, energy.mat);
        auto seems = get_seems(energy, delta);
        remove_seems(in, seems);
    }

    template<typename TFilter>
    void shrink_to_fit(const cv::Mat &in, cv::Mat &out, const cv::Size &new_size, TFilter filter, double quality) {
        cv::Size in_size = in.size();
        MatWrp in_wrp(in.clone());

        shrink(in_wrp, in_size.width - new_size.width, filter);
        in_wrp.transpose();
        shrink(in_wrp, in_size.height - new_size.height, filter);

        out = in_wrp.mat;
    }

    template <typename TFilter>
    void shrink_to_fit(const cv::Mat& in, cv::Mat& out, const cv::Size& new_size, const TFilter& filter) {
        shrink_to_fit(in, out, new_size, filter, 1.f);
    }
}

#endif //CONTENTAWARERESIZE_CORE_H
