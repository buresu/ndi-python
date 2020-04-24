#include "PyAudioFrame.hpp"

PyAudioFrameV2::PyAudioFrameV2(int sample_rate_, int no_channels_,
                               int no_samples_, int64_t timecode_,
                               float *p_data_, int channel_stride_in_bytes_,
                               const std::string &metadata_, int64_t timestamp_)
    : _metadata(metadata_) {
  sample_rate = sample_rate_;
  no_channels = no_channels_;
  no_samples = no_samples_;
  timecode = timecode_;
  p_data = p_data_;
  channel_stride_in_bytes = channel_stride_in_bytes_;
  p_metadata = _metadata.c_str();
  timestamp = timestamp_;
}

PyAudioFrameV2::~PyAudioFrameV2() {}

void PyAudioFrameV2::setData(py::array_t<float> &array) {
  auto info = array.request();
  p_data = reinterpret_cast<float *>(info.ptr);
  channel_stride_in_bytes = info.strides[0];
}

const std::string &PyAudioFrameV2::getMetadata() const { return _metadata; }

void PyAudioFrameV2::setMetadata(const std::string &data) {
  _metadata = data;
  p_metadata = _metadata.c_str();
}
