#include "PyMetadataFrame.hpp"

PyMetadataFrame::PyMetadataFrame(int length_, int64_t timecode_,
                                 const std::string &data_)
    : _data(data_) {
  length = length_;
  timecode = timecode_;
  p_data = &_data[0];
}

PyMetadataFrame::~PyMetadataFrame() {}

const std::string &PyMetadataFrame::getData() const { return _data; }

void PyMetadataFrame::setData(const std::string &data) {
  _data = data;
  p_data = &_data[0];
}
