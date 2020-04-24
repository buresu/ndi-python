#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <string>

#include <Processing.NDI.Lib.h>

namespace py = pybind11;

class PyVideoFrameV2 : public NDIlib_video_frame_v2_t {
public:
  PyVideoFrameV2(int xres_, int yres_, NDIlib_FourCC_video_type_e FourCC_,
                 int frame_rate_N_, int frame_rate_D_,
                 float picture_aspect_ratio_,
                 NDIlib_frame_format_type_e frame_format_type_,
                 int64_t timecode_, uint8_t *data_, int line_stride_in_bytes_,
                 const std::string &metadata_, int64_t timestamp_);
  virtual ~PyVideoFrameV2();

  void setData(py::array_t<uint8_t> &array);

  const std::string &getMetadata() const;
  void setMetadata(const std::string &data);

private:
  std::string _metadata;
};
