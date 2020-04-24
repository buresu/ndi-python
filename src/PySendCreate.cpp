#include "PySendCreate.hpp"

PySendCreate::PySendCreate(const std::string &ndi_name_,
                           const std::string &groupds_, bool clock_video_,
                           bool clock_audio_)
    : _ndi_name(ndi_name_), _groups(groupds_) {
  p_ndi_name = _ndi_name.c_str();
  p_groups = _groups.c_str();
  clock_video = clock_video_;
  clock_audio = clock_audio_;
}

PySendCreate::~PySendCreate() {}

const std::string &PySendCreate::getNdiName() const { return _ndi_name; }

void PySendCreate::setNdiName(const std::string &name) {
  _ndi_name = name;
  p_ndi_name = _ndi_name.c_str();
}

const std::string &PySendCreate::getGroups() const { return _groups; }

void PySendCreate::setGroups(const std::string &groups) {
  _groups = groups;
  p_groups = _groups.c_str();
}
