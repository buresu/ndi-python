#include <Processing.NDI.Lib.h>
#include <string>

class PySource : public NDIlib_source_t {
public:
  PySource(const std::string &ndi_name_, const std::string &url_address_);
  virtual ~PySource();

  const std::string &getNdiName() const;
  void setNdiName(const std::string &name);

  const std::string &getUrlAddress() const;
  void setUrlAddress(const std::string &address);

private:
  std::string _ndi_name;
  std::string _url_address;
};
