#include "PySource.hpp"

PySource::PySource(const std::string &ndi_name_,
                   const std::string &url_address_)
    : _ndi_name(ndi_name_), _url_address(url_address_) {
  p_ndi_name = _ndi_name.c_str();
  p_url_address = _url_address.c_str();
}

PySource::~PySource() {}

const std::string &PySource::getNdiName() const { return _ndi_name; }

void PySource::setNdiName(const std::string &name) {
  _ndi_name = name;
  p_ndi_name = _ndi_name.c_str();
}

const std::string &PySource::getUrlAddress() const { return _url_address; }

void PySource::setUrlAddress(const std::string &address) {
  _url_address = address;
  p_url_address = _url_address.c_str();
}
