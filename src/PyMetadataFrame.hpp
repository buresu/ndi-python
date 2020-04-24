#include <string>

#include <Processing.NDI.Lib.h>

class PyMetadataFrame : public NDIlib_metadata_frame_t {
public:
  PyMetadataFrame(int length_, int64_t timecode_, const std::string &data_);
  virtual ~PyMetadataFrame();

  const std::string &getData() const;
  void setData(const std::string &data);

private:
  std::string _data;
};
