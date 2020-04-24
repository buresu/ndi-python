#include "PyVideoFrame.hpp"

PyVideoFrameV2::PyVideoFrameV2(int xres_, int yres_,
                               NDIlib_FourCC_video_type_e FourCC_,
                               int frame_rate_N_, int frame_rate_D_,
                               float picture_aspect_ratio_,
                               NDIlib_frame_format_type_e frame_format_type_,
                               int64_t timecode_, uint8_t *data_,
                               int line_stride_in_bytes_,
                               const std::string &metadata_, int64_t timestamp_)
    : _metadata(metadata_) {
  xres = xres_;
  yres = yres_;
  FourCC = FourCC_;
  frame_rate_N = frame_rate_N_;
  frame_rate_D = frame_rate_D_;
  picture_aspect_ratio = picture_aspect_ratio_;
  frame_format_type = frame_format_type_;
  timecode = timecode_;
  line_stride_in_bytes = line_stride_in_bytes_;
  p_data = data_;
  p_metadata = _metadata.c_str();
  timestamp = timestamp_;
}

PyVideoFrameV2::~PyVideoFrameV2() {}

void PyVideoFrameV2::setData(py::array_t<uint8_t> &array) {
  auto info = array.request();
  p_data = reinterpret_cast<uint8_t *>(info.ptr);
  picture_aspect_ratio = info.shape[1] / float(info.shape[0]);
  xres = info.shape[1];
  yres = info.shape[0];
  line_stride_in_bytes = info.strides[0];
}

const std::string &PyVideoFrameV2::getMetadata() const { return _metadata; }

void PyVideoFrameV2::setMetadata(const std::string &data) {
  _metadata = data;
  p_metadata = _metadata.c_str();
}
