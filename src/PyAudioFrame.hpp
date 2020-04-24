#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <string>

#include <Processing.NDI.Lib.h>

namespace py = pybind11;

class PyAudioFrameV2 : public NDIlib_audio_frame_v2_t {
public:
  PyAudioFrameV2(int sample_rate_, int no_channels_, int no_samples_,
                 int64_t timecode_, float *p_data_,
                 int channel_stride_in_bytes_, const std::string &metadata_,
                 int64_t timestamp_);
  virtual ~PyAudioFrameV2();

  void setData(py::array_t<float> &array);

  const std::string &getMetadata() const;
  void setMetadata(const std::string &data);

private:
  std::string _metadata;
};
