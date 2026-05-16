#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <Processing.NDI.Lib.h>

#include <string>

namespace py = pybind11;

// Wrapper structs

struct SourceWrapper {
  std::string ndi_name;
  std::string url_address;
  NDIlib_source_t inner{};

  SourceWrapper(const char *n, const char *u) {
    if (n) {
      ndi_name = n;
      inner.p_ndi_name = ndi_name.c_str();
    }
    if (u) {
      url_address = u;
      inner.p_url_address = url_address.c_str();
    }
  }
  SourceWrapper(const SourceWrapper &o)
      : ndi_name(o.ndi_name), url_address(o.url_address), inner(o.inner) {
    inner.p_ndi_name = ndi_name.empty() ? nullptr : ndi_name.c_str();
    inner.p_url_address = url_address.empty() ? nullptr : url_address.c_str();
  }
  SourceWrapper(SourceWrapper &&o) noexcept
      : ndi_name(std::move(o.ndi_name)), url_address(std::move(o.url_address)),
        inner(o.inner) {
    inner.p_ndi_name = ndi_name.empty() ? nullptr : ndi_name.c_str();
    inner.p_url_address = url_address.empty() ? nullptr : url_address.c_str();
  }
  SourceWrapper &operator=(const SourceWrapper &o) {
    ndi_name = o.ndi_name;
    url_address = o.url_address;
    inner = o.inner;
    inner.p_ndi_name = ndi_name.empty() ? nullptr : ndi_name.c_str();
    inner.p_url_address = url_address.empty() ? nullptr : url_address.c_str();
    return *this;
  }
  SourceWrapper &operator=(SourceWrapper &&o) noexcept {
    ndi_name = std::move(o.ndi_name);
    url_address = std::move(o.url_address);
    inner = o.inner;
    inner.p_ndi_name = ndi_name.empty() ? nullptr : ndi_name.c_str();
    inner.p_url_address = url_address.empty() ? nullptr : url_address.c_str();
    return *this;
  }
  void set_ndi_name(const std::string &s) {
    ndi_name = s;
    inner.p_ndi_name = ndi_name.c_str();
  }
  void set_url_address(const std::string &s) {
    url_address = s;
    inner.p_url_address = url_address.c_str();
  }
};

struct VideoFrameV2Wrapper {
  std::string metadata_str;
  py::array_t<uint8_t> data_array;
  NDIlib_video_frame_v2_t inner{};

  VideoFrameV2Wrapper(int xres, int yres, NDIlib_FourCC_video_type_e FourCC,
                      int frame_rate_N, int frame_rate_D,
                      float picture_aspect_ratio,
                      NDIlib_frame_format_type_e frame_format_type,
                      int64_t timecode, int line_stride_in_bytes,
                      const char *p_metadata, int64_t timestamp) {
    inner.xres = xres;
    inner.yres = yres;
    inner.FourCC = FourCC;
    inner.frame_rate_N = frame_rate_N;
    inner.frame_rate_D = frame_rate_D;
    inner.picture_aspect_ratio = picture_aspect_ratio;
    inner.frame_format_type = frame_format_type;
    inner.timecode = timecode;
    inner.p_data = nullptr;
    inner.line_stride_in_bytes = line_stride_in_bytes;
    inner.timestamp = timestamp;
    if (p_metadata) {
      metadata_str = p_metadata;
      inner.p_metadata = metadata_str.c_str();
    }
  }
  VideoFrameV2Wrapper(const VideoFrameV2Wrapper &o)
      : metadata_str(o.metadata_str), data_array(o.data_array), inner(o.inner) {
    inner.p_metadata = metadata_str.empty() ? nullptr : metadata_str.c_str();
  }
  VideoFrameV2Wrapper(VideoFrameV2Wrapper &&o) noexcept
      : metadata_str(std::move(o.metadata_str)),
        data_array(std::move(o.data_array)), inner(o.inner) {
    inner.p_metadata = metadata_str.empty() ? nullptr : metadata_str.c_str();
  }
  VideoFrameV2Wrapper &operator=(const VideoFrameV2Wrapper &o) {
    metadata_str = o.metadata_str;
    data_array = o.data_array;
    inner = o.inner;
    inner.p_metadata = metadata_str.empty() ? nullptr : metadata_str.c_str();
    return *this;
  }
  VideoFrameV2Wrapper &operator=(VideoFrameV2Wrapper &&o) noexcept {
    metadata_str = std::move(o.metadata_str);
    data_array = std::move(o.data_array);
    inner = o.inner;
    inner.p_metadata = metadata_str.empty() ? nullptr : metadata_str.c_str();
    return *this;
  }
  void set_metadata(const std::string &s) {
    metadata_str = s;
    inner.p_metadata = metadata_str.c_str();
  }
  void set_data(py::array_t<uint8_t> arr) {
    data_array = std::move(arr);
    auto info = data_array.request();
    inner.p_data = static_cast<uint8_t *>(info.ptr);
    inner.yres = info.shape[0];
    inner.xres = info.shape[1];
    inner.picture_aspect_ratio = info.shape[1] / float(info.shape[0]);
    inner.line_stride_in_bytes = info.strides[0];
  }
  py::array get_data() const {
    if (!inner.p_data)
      return py::array_t<uint8_t>();
    int r = inner.yres, c = inner.xres;
    size_t b1 = inner.line_stride_in_bytes;
    size_t b2 = c > 0 ? b1 / c : 0;
    size_t b3 = sizeof(uint8_t);
    return py::array(py::buffer_info(inner.p_data, b3,
                                     py::format_descriptor<uint8_t>::format(),
                                     3, {r, c, int(b2)}, {b1, b2, b3}));
  }
};

struct AudioFrameV2Wrapper {
  std::string metadata_str;
  py::array_t<float> data_array;
  NDIlib_audio_frame_v2_t inner{};

  AudioFrameV2Wrapper(int sample_rate, int no_channels, int no_samples,
                      int64_t timecode, int channel_stride_in_bytes,
                      const char *p_metadata, int64_t timestamp) {
    inner.sample_rate = sample_rate;
    inner.no_channels = no_channels;
    inner.no_samples = no_samples;
    inner.timecode = timecode;
    inner.p_data = nullptr;
    inner.channel_stride_in_bytes = channel_stride_in_bytes;
    inner.timestamp = timestamp;
    if (p_metadata) {
      metadata_str = p_metadata;
      inner.p_metadata = metadata_str.c_str();
    }
  }
  AudioFrameV2Wrapper(const AudioFrameV2Wrapper &o)
      : metadata_str(o.metadata_str), data_array(o.data_array), inner(o.inner) {
    inner.p_metadata = metadata_str.empty() ? nullptr : metadata_str.c_str();
  }
  AudioFrameV2Wrapper(AudioFrameV2Wrapper &&o) noexcept
      : metadata_str(std::move(o.metadata_str)),
        data_array(std::move(o.data_array)), inner(o.inner) {
    inner.p_metadata = metadata_str.empty() ? nullptr : metadata_str.c_str();
  }
  AudioFrameV2Wrapper &operator=(const AudioFrameV2Wrapper &o) {
    metadata_str = o.metadata_str;
    data_array = o.data_array;
    inner = o.inner;
    inner.p_metadata = metadata_str.empty() ? nullptr : metadata_str.c_str();
    return *this;
  }
  AudioFrameV2Wrapper &operator=(AudioFrameV2Wrapper &&o) noexcept {
    metadata_str = std::move(o.metadata_str);
    data_array = std::move(o.data_array);
    inner = o.inner;
    inner.p_metadata = metadata_str.empty() ? nullptr : metadata_str.c_str();
    return *this;
  }
  void set_metadata(const std::string &s) {
    metadata_str = s;
    inner.p_metadata = metadata_str.c_str();
  }
  void set_data(py::array_t<float> arr) {
    data_array = std::move(arr);
    auto info = data_array.request();
    inner.p_data = static_cast<float *>(info.ptr);
    inner.no_channels = info.shape[0];
    inner.no_samples = info.shape[1];
    inner.channel_stride_in_bytes = info.strides[0];
  }
  py::array get_data() const {
    if (!inner.p_data)
      return py::array_t<float>();
    size_t col = inner.no_samples, row = inner.no_channels,
           size = sizeof(float);
    return py::array(py::buffer_info(inner.p_data, size,
                                     py::format_descriptor<float>::format(), 2,
                                     {row, col}, {col * size, size}));
  }
};

struct AudioFrameV3Wrapper {
  std::string metadata_str;
  py::array_t<uint8_t> data_array;
  NDIlib_audio_frame_v3_t inner{};

  AudioFrameV3Wrapper(int sample_rate, int no_channels, int no_samples,
                      int64_t timecode, NDIlib_FourCC_audio_type_e FourCC,
                      int channel_stride_in_bytes, const char *p_metadata,
                      int64_t timestamp) {
    inner.sample_rate = sample_rate;
    inner.no_channels = no_channels;
    inner.no_samples = no_samples;
    inner.timecode = timecode;
    inner.FourCC = FourCC;
    inner.p_data = nullptr;
    inner.channel_stride_in_bytes = channel_stride_in_bytes;
    inner.timestamp = timestamp;
    if (p_metadata) {
      metadata_str = p_metadata;
      inner.p_metadata = metadata_str.c_str();
    }
  }
  AudioFrameV3Wrapper(const AudioFrameV3Wrapper &o)
      : metadata_str(o.metadata_str), data_array(o.data_array), inner(o.inner) {
    inner.p_metadata = metadata_str.empty() ? nullptr : metadata_str.c_str();
  }
  AudioFrameV3Wrapper(AudioFrameV3Wrapper &&o) noexcept
      : metadata_str(std::move(o.metadata_str)),
        data_array(std::move(o.data_array)), inner(o.inner) {
    inner.p_metadata = metadata_str.empty() ? nullptr : metadata_str.c_str();
  }
  AudioFrameV3Wrapper &operator=(const AudioFrameV3Wrapper &o) {
    metadata_str = o.metadata_str;
    data_array = o.data_array;
    inner = o.inner;
    inner.p_metadata = metadata_str.empty() ? nullptr : metadata_str.c_str();
    return *this;
  }
  AudioFrameV3Wrapper &operator=(AudioFrameV3Wrapper &&o) noexcept {
    metadata_str = std::move(o.metadata_str);
    data_array = std::move(o.data_array);
    inner = o.inner;
    inner.p_metadata = metadata_str.empty() ? nullptr : metadata_str.c_str();
    return *this;
  }
  void set_metadata(const std::string &s) {
    metadata_str = s;
    inner.p_metadata = metadata_str.c_str();
  }
  void set_data(py::array_t<uint8_t> arr) {
    data_array = std::move(arr);
    auto info = data_array.request();
    inner.p_data = static_cast<uint8_t *>(info.ptr);
    inner.no_channels = info.shape[0];
    inner.no_samples = info.shape[1];
    inner.channel_stride_in_bytes = info.strides[0];
  }
  py::array get_data() const {
    if (!inner.p_data)
      return py::array_t<uint8_t>();
    size_t col = inner.no_samples, row = inner.no_channels,
           size = sizeof(uint8_t);
    return py::array(py::buffer_info(inner.p_data, size,
                                     py::format_descriptor<uint8_t>::format(),
                                     2, {row, col}, {col * size * 4, size}));
  }
};

struct MetadataFrameWrapper {
  std::string data_str;
  NDIlib_metadata_frame_t inner{};

  MetadataFrameWrapper(int length, int64_t timecode, const char *p_data) {
    inner.length = length;
    inner.timecode = timecode;
    if (p_data) {
      data_str = p_data;
      inner.p_data = data_str.empty() ? nullptr : data_str.data();
    }
  }
  void set_data(const std::string &s) {
    data_str = s;
    inner.length = static_cast<int>(data_str.size());
    inner.p_data = data_str.empty() ? nullptr : data_str.data();
  }
  std::string get_data() const {
    return inner.p_data ? std::string(inner.p_data) : std::string();
  }
};

struct FindCreateWrapper {
  std::string groups_str;
  std::string extra_ips_str;
  NDIlib_find_create_t inner{};

  FindCreateWrapper(bool show_local_sources, const char *p_groups,
                    const char *p_extra_ips) {
    inner.show_local_sources = show_local_sources;
    if (p_groups) {
      groups_str = p_groups;
      inner.p_groups = groups_str.c_str();
    }
    if (p_extra_ips) {
      extra_ips_str = p_extra_ips;
      inner.p_extra_ips = extra_ips_str.c_str();
    }
  }
  void set_groups(const std::string &s) {
    groups_str = s;
    inner.p_groups = groups_str.c_str();
  }
  void set_extra_ips(const std::string &s) {
    extra_ips_str = s;
    inner.p_extra_ips = extra_ips_str.c_str();
  }
};

struct RecvCreateV3Wrapper {
  std::string ndi_recv_name_str;
  std::string source_ndi_name_str;
  std::string source_url_address_str;
  NDIlib_recv_create_v3_t inner{};

  RecvCreateV3Wrapper(const NDIlib_source_t src,
                      NDIlib_recv_color_format_e color_format,
                      NDIlib_recv_bandwidth_e bandwidth,
                      bool allow_video_fields, const char *p_ndi_recv_name) {
    set_source_ptrs(src.p_ndi_name, src.p_url_address);
    inner.color_format = color_format;
    inner.bandwidth = bandwidth;
    inner.allow_video_fields = allow_video_fields;
    if (p_ndi_recv_name) {
      ndi_recv_name_str = p_ndi_recv_name;
      inner.p_ndi_recv_name = ndi_recv_name_str.c_str();
    }
  }
  void set_ndi_recv_name(const std::string &s) {
    ndi_recv_name_str = s;
    inner.p_ndi_recv_name = ndi_recv_name_str.c_str();
  }
  void set_source_ptrs(const char *ndi_name, const char *url_address) {
    if (ndi_name) {
      source_ndi_name_str = ndi_name;
    } else {
      source_ndi_name_str.clear();
    }
    if (url_address) {
      source_url_address_str = url_address;
    } else {
      source_url_address_str.clear();
    }
    inner.source_to_connect_to.p_ndi_name =
        source_ndi_name_str.empty() ? nullptr : source_ndi_name_str.c_str();
    inner.source_to_connect_to.p_url_address =
        source_url_address_str.empty() ? nullptr
                                       : source_url_address_str.c_str();
  }
};

struct SendCreateWrapper {
  std::string ndi_name_str;
  std::string groups_str;
  NDIlib_send_create_t inner{};

  SendCreateWrapper(const char *p_ndi_name, const char *p_groups,
                    bool clock_video, bool clock_audio) {
    if (p_ndi_name) {
      ndi_name_str = p_ndi_name;
      inner.p_ndi_name = ndi_name_str.c_str();
    }
    if (p_groups) {
      groups_str = p_groups;
      inner.p_groups = groups_str.c_str();
    }
    inner.clock_video = clock_video;
    inner.clock_audio = clock_audio;
  }
  void set_ndi_name(const std::string &s) {
    ndi_name_str = s;
    inner.p_ndi_name = ndi_name_str.c_str();
  }
  void set_groups(const std::string &s) {
    groups_str = s;
    inner.p_groups = groups_str.c_str();
  }
};

struct RoutingCreateWrapper {
  std::string ndi_name_str;
  std::string groups_str;
  NDIlib_routing_create_t inner{};

  RoutingCreateWrapper(const char *p_ndi_name, const char *p_groups) {
    if (p_ndi_name) {
      ndi_name_str = p_ndi_name;
      inner.p_ndi_name = ndi_name_str.c_str();
    }
    if (p_groups) {
      groups_str = p_groups;
      inner.p_groups = groups_str.c_str();
    }
  }
  void set_ndi_name(const std::string &s) {
    ndi_name_str = s;
    inner.p_ndi_name = ndi_name_str.c_str();
  }
  void set_groups(const std::string &s) {
    groups_str = s;
    inner.p_groups = groups_str.c_str();
  }
};

struct AudioFrameInterleaved16sWrapper {
  py::array_t<int16_t> data_array;
  NDIlib_audio_frame_interleaved_16s_t inner{};

  AudioFrameInterleaved16sWrapper(int sample_rate, int no_channels,
                                  int no_samples, int64_t timecode,
                                  int reference_level) {
    inner.sample_rate = sample_rate;
    inner.no_channels = no_channels;
    inner.no_samples = no_samples;
    inner.timecode = timecode;
    inner.reference_level = reference_level;
    inner.p_data = nullptr;
  }
  void set_data(py::array_t<int16_t> arr) {
    data_array = std::move(arr);
    auto info = data_array.request();
    inner.p_data = static_cast<int16_t *>(info.ptr);
    inner.no_channels = info.shape[0];
    inner.no_samples = info.shape[1];
  }
  py::array get_data() const {
    if (!inner.p_data)
      return py::array_t<int16_t>();
    size_t col = inner.no_samples, row = inner.no_channels,
           size = sizeof(int16_t);
    return py::array(py::buffer_info(inner.p_data, size,
                                     py::format_descriptor<int16_t>::format(),
                                     2, {row, col}, {col * size, size}));
  }
};

struct AudioFrameInterleaved32sWrapper {
  py::array_t<int32_t> data_array;
  NDIlib_audio_frame_interleaved_32s_t inner{};

  AudioFrameInterleaved32sWrapper(int sample_rate, int no_channels,
                                  int no_samples, int64_t timecode,
                                  int reference_level) {
    inner.sample_rate = sample_rate;
    inner.no_channels = no_channels;
    inner.no_samples = no_samples;
    inner.timecode = timecode;
    inner.reference_level = reference_level;
    inner.p_data = nullptr;
  }
  void set_data(py::array_t<int32_t> arr) {
    data_array = std::move(arr);
    auto info = data_array.request();
    inner.p_data = static_cast<int32_t *>(info.ptr);
    inner.no_channels = info.shape[0];
    inner.no_samples = info.shape[1];
  }
  py::array get_data() const {
    if (!inner.p_data)
      return py::array_t<int32_t>();
    size_t col = inner.no_samples, row = inner.no_channels,
           size = sizeof(int32_t);
    return py::array(py::buffer_info(inner.p_data, size,
                                     py::format_descriptor<int32_t>::format(),
                                     2, {row, col}, {col * size, size}));
  }
};

struct AudioFrameInterleaved32fWrapper {
  py::array_t<float> data_array;
  NDIlib_audio_frame_interleaved_32f_t inner{};

  AudioFrameInterleaved32fWrapper(int sample_rate, int no_channels,
                                  int no_samples, int64_t timecode) {
    inner.sample_rate = sample_rate;
    inner.no_channels = no_channels;
    inner.no_samples = no_samples;
    inner.timecode = timecode;
    inner.p_data = nullptr;
  }
  void set_data(py::array_t<float> arr) {
    data_array = std::move(arr);
    auto info = data_array.request();
    inner.p_data = static_cast<float *>(info.ptr);
    inner.no_channels = info.shape[0];
    inner.no_samples = info.shape[1];
  }
  py::array get_data() const {
    if (!inner.p_data)
      return py::array_t<float>();
    size_t col = inner.no_samples, row = inner.no_channels,
           size = sizeof(float);
    return py::array(py::buffer_info(inner.p_data, size,
                                     py::format_descriptor<float>::format(), 2,
                                     {row, col}, {col * size, size}));
  }
};

struct RecvListenerCreateWrapper {
  std::string url_address_str;
  NDIlib_recv_listener_create_t inner{};
  explicit RecvListenerCreateWrapper(const std::string &url = {}) {
    if (!url.empty()) {
      url_address_str = url;
      inner.p_url_address = url_address_str.c_str();
    }
  }
  void set_url_address(const std::string &s) {
    url_address_str = s;
    inner.p_url_address = url_address_str.c_str();
  }
};

struct SendListenerCreateWrapper {
  std::string url_address_str;
  NDIlib_send_listener_create_t inner{};
  explicit SendListenerCreateWrapper(const std::string &url = {}) {
    if (!url.empty()) {
      url_address_str = url;
      inner.p_url_address = url_address_str.c_str();
    }
  }
  void set_url_address(const std::string &s) {
    url_address_str = s;
    inner.p_url_address = url_address_str.c_str();
  }
};

struct RecvAdvertiserCreateWrapper {
  std::string url_address_str;
  NDIlib_recv_advertiser_create_t inner{};
  explicit RecvAdvertiserCreateWrapper(const std::string &url = {}) {
    if (!url.empty()) {
      url_address_str = url;
      inner.p_url_address = url_address_str.c_str();
    }
  }
  void set_url_address(const std::string &s) {
    url_address_str = s;
    inner.p_url_address = url_address_str.c_str();
  }
};

struct SendAdvertiserCreateWrapper {
  std::string url_address_str;
  NDIlib_send_advertiser_create_t inner{};
  explicit SendAdvertiserCreateWrapper(const std::string &url = {}) {
    if (!url.empty()) {
      url_address_str = url;
      inner.p_url_address = url_address_str.c_str();
    }
  }
  void set_url_address(const std::string &s) {
    url_address_str = s;
    inner.p_url_address = url_address_str.c_str();
  }
};

struct ReceiverWrapper {
  std::string uuid, name, input_uuid, input_name, address;
  std::vector<NDIlib_receiver_type_e> streams;
  std::vector<NDIlib_receiver_command_e> commands;
  bool events_subscribed{};
  explicit ReceiverWrapper(const NDIlib_receiver_t &r)
      : uuid(r.p_uuid ? r.p_uuid : ""), name(r.p_name ? r.p_name : ""),
        input_uuid(r.p_input_uuid ? r.p_input_uuid : ""),
        input_name(r.p_input_name ? r.p_input_name : ""),
        address(r.p_address ? r.p_address : ""),
        events_subscribed(r.events_subscribed) {
    if (r.p_streams)
      streams.assign(r.p_streams, r.p_streams + r.num_streams);
    if (r.p_commands)
      commands.assign(r.p_commands, r.p_commands + r.num_commands);
  }
};

struct SenderWrapper {
  std::string uuid, name, metadata, address;
  int port{};
  std::vector<std::string> groups;
  bool events_subscribed{};
  explicit SenderWrapper(const NDIlib_sender_t &s)
      : uuid(s.p_uuid ? s.p_uuid : ""), name(s.p_name ? s.p_name : ""),
        metadata(s.p_metadata ? s.p_metadata : ""),
        address(s.p_address ? s.p_address : ""), port(s.port),
        events_subscribed(s.events_subscribed) {
    for (uint32_t i = 0; i < s.num_groups; ++i)
      if (s.p_groups[i])
        groups.emplace_back(s.p_groups[i]);
  }
};

struct ListenerEventWrapper {
  std::string uuid;
  std::string name;
  std::string value;
  explicit ListenerEventWrapper(const NDIlib_listener_event &e)
      : uuid(e.p_uuid ? e.p_uuid : ""), name(e.p_name ? e.p_name : ""),
        value(e.p_value ? e.p_value : "") {}
};

PYBIND11_MODULE(NDIlib, m) {

  m.doc() = "NDI SDK for Python";

  // Processing.NDI.structs
  py::enum_<NDIlib_frame_type_e>(m, "FrameType", py::arithmetic())
      .value("FRAME_TYPE_NONE", NDIlib_frame_type_none)
      .value("FRAME_TYPE_VIDEO", NDIlib_frame_type_video)
      .value("FRAME_TYPE_AUDIO", NDIlib_frame_type_audio)
      .value("FRAME_TYPE_METADATA", NDIlib_frame_type_metadata)
      .value("FRAME_TYPE_ERROR", NDIlib_frame_type_error)
      .value("FRAME_TYPE_STATUS_CHANGE", NDIlib_frame_type_status_change)
      .value("FRAME_TYPE_SOURCE_CHANGE", NDIlib_frame_type_source_change)
      .value("FRAME_TYPE_MAX", NDIlib_frame_type_max)
      .export_values();

  py::enum_<NDIlib_FourCC_video_type_e>(m, "FourCCVideoType", py::arithmetic())
      .value("FOURCC_VIDEO_TYPE_UYVY", NDIlib_FourCC_video_type_UYVY)
      .value("FOURCC_VIDEO_TYPE_UYVA", NDIlib_FourCC_video_type_UYVA)
      .value("FOURCC_VIDEO_TYPE_P216", NDIlib_FourCC_video_type_P216)
      .value("FOURCC_VIDEO_TYPE_PA16", NDIlib_FourCC_video_type_PA16)
      .value("FOURCC_VIDEO_TYPE_YV12", NDIlib_FourCC_video_type_YV12)
      .value("FOURCC_VIDEO_TYPE_I420", NDIlib_FourCC_video_type_I420)
      .value("FOURCC_VIDEO_TYPE_NV12", NDIlib_FourCC_video_type_NV12)
      .value("FOURCC_VIDEO_TYPE_BGRA", NDIlib_FourCC_video_type_BGRA)
      .value("FOURCC_VIDEO_TYPE_BGRX", NDIlib_FourCC_video_type_BGRX)
      .value("FOURCC_VIDEO_TYPE_RGBA", NDIlib_FourCC_video_type_RGBA)
      .value("FOURCC_VIDEO_TYPE_RGBX", NDIlib_FourCC_video_type_RGBX)
      .value("FOURCC_VIDEO_TYPE_MAX", NDIlib_FourCC_video_type_max)
      .export_values();

  py::enum_<NDIlib_FourCC_audio_type_e>(m, "FourCCAudioType", py::arithmetic())
      .value("FOURCC_AUDIO_TYPE_FLTP", NDIlib_FourCC_audio_type_FLTP)
      .value("FOURCC_AUDIO_TYPE_MAX", NDIlib_FourCC_audio_type_max)
      .export_values();

  py::enum_<NDIlib_frame_format_type_e>(m, "FrameFormatType", py::arithmetic())
      .value("FRAME_FORMAT_TYPE_PROGRESSIVE",
             NDIlib_frame_format_type_progressive)
      .value("FRAME_FORMAT_TYPE_INTERLEAVED",
             NDIlib_frame_format_type_interleaved)
      .value("FRAME_FORMAT_TYPE_FIELD_0", NDIlib_frame_format_type_field_0)
      .value("FRAME_FORMAT_TYPE_FIELD_1", NDIlib_frame_format_type_field_1)
      .value("FRAME_FORMAT_TYPE_MAX", NDIlib_frame_format_type_max)
      .export_values();

  m.attr("SEND_TIMECODE_SYNTHESIZE") = py::int_(INT64_MAX);

  m.attr("RECV_TIMESTAMP_UNDEFINED") = py::int_(INT64_MAX);

  py::class_<SourceWrapper>(m, "Source")
      .def(py::init<const char *, const char *>(),
           py::arg("p_ndi_name") = nullptr, py::arg("p_url_address") = nullptr)
      .def_property(
          "ndi_name", [](const SourceWrapper &self) { return self.ndi_name; },
          [](SourceWrapper &self, const std::string &name) {
            self.set_ndi_name(name);
          })
      .def_property(
          "url_address",
          [](const SourceWrapper &self) { return self.url_address; },
          [](SourceWrapper &self, const std::string &address) {
            self.set_url_address(address);
          });

  py::class_<VideoFrameV2Wrapper>(m, "VideoFrameV2")
      .def(py::init<int, int, NDIlib_FourCC_video_type_e, int, int, float,
                    NDIlib_frame_format_type_e, int64_t, int, const char *,
                    int64_t>(),
           py::arg("xres") = 0, py::arg("yres") = 0,
           py::arg("FourCC") = NDIlib_FourCC_video_type_UYVY,
           py::arg("frame_rate_N") = 30000, py::arg("frame_rate_D") = 1001,
           py::arg("picture_aspect_ratio") = 0.0f,
           py::arg("frame_format_type") = NDIlib_frame_format_type_progressive,
           py::arg("timecode") = 0, py::arg("line_stride_in_bytes") = 0,
           py::arg("p_metadata") = nullptr, py::arg("timestamp") = 0)
      .def_property(
          "xres", [](const VideoFrameV2Wrapper &s) { return s.inner.xres; },
          [](VideoFrameV2Wrapper &s, int v) { s.inner.xres = v; })
      .def_property(
          "yres", [](const VideoFrameV2Wrapper &s) { return s.inner.yres; },
          [](VideoFrameV2Wrapper &s, int v) { s.inner.yres = v; })
      .def_property(
          "FourCC", [](const VideoFrameV2Wrapper &s) { return s.inner.FourCC; },
          [](VideoFrameV2Wrapper &s, NDIlib_FourCC_video_type_e v) {
            s.inner.FourCC = v;
          })
      .def_property(
          "frame_rate_N",
          [](const VideoFrameV2Wrapper &s) { return s.inner.frame_rate_N; },
          [](VideoFrameV2Wrapper &s, int v) { s.inner.frame_rate_N = v; })
      .def_property(
          "frame_rate_D",
          [](const VideoFrameV2Wrapper &s) { return s.inner.frame_rate_D; },
          [](VideoFrameV2Wrapper &s, int v) { s.inner.frame_rate_D = v; })
      .def_property(
          "picture_aspect_ratio",
          [](const VideoFrameV2Wrapper &s) {
            return s.inner.picture_aspect_ratio;
          },
          [](VideoFrameV2Wrapper &s, float v) {
            s.inner.picture_aspect_ratio = v;
          })
      .def_property(
          "frame_format_type",
          [](const VideoFrameV2Wrapper &s) {
            return s.inner.frame_format_type;
          },
          [](VideoFrameV2Wrapper &s, NDIlib_frame_format_type_e v) {
            s.inner.frame_format_type = v;
          })
      .def_property(
          "timecode",
          [](const VideoFrameV2Wrapper &s) { return s.inner.timecode; },
          [](VideoFrameV2Wrapper &s, int64_t v) { s.inner.timecode = v; })
      .def_property(
          "data", [](const VideoFrameV2Wrapper &s) { return s.get_data(); },
          [](VideoFrameV2Wrapper &s, py::array_t<uint8_t> arr) {
            s.set_data(std::move(arr));
          })
      .def_property(
          "line_stride_in_bytes",
          [](const VideoFrameV2Wrapper &s) {
            return s.inner.line_stride_in_bytes;
          },
          [](VideoFrameV2Wrapper &s, int v) {
            s.inner.line_stride_in_bytes = v;
          })
      .def_property(
          "metadata",
          [](const VideoFrameV2Wrapper &s) { return s.metadata_str; },
          [](VideoFrameV2Wrapper &s, const std::string &v) {
            s.set_metadata(v);
          })
      .def_property(
          "timestamp",
          [](const VideoFrameV2Wrapper &s) { return s.inner.timestamp; },
          [](VideoFrameV2Wrapper &s, int64_t v) { s.inner.timestamp = v; });

  py::class_<AudioFrameV2Wrapper>(m, "AudioFrameV2")
      .def(py::init<int, int, int, int64_t, int, const char *, int64_t>(),
           py::arg("sample_rate") = 48000, py::arg("no_channels") = 2,
           py::arg("no_samples") = 0,
           py::arg("timecode") = NDIlib_send_timecode_synthesize,
           py::arg("channel_stride_in_bytes") = 0,
           py::arg("p_metadata") = nullptr, py::arg("timestamp") = 0)
      .def_property(
          "sample_rate",
          [](const AudioFrameV2Wrapper &s) { return s.inner.sample_rate; },
          [](AudioFrameV2Wrapper &s, int v) { s.inner.sample_rate = v; })
      .def_property(
          "no_channels",
          [](const AudioFrameV2Wrapper &s) { return s.inner.no_channels; },
          [](AudioFrameV2Wrapper &s, int v) { s.inner.no_channels = v; })
      .def_property(
          "no_samples",
          [](const AudioFrameV2Wrapper &s) { return s.inner.no_samples; },
          [](AudioFrameV2Wrapper &s, int v) { s.inner.no_samples = v; })
      .def_property(
          "timecode",
          [](const AudioFrameV2Wrapper &s) { return s.inner.timecode; },
          [](AudioFrameV2Wrapper &s, int64_t v) { s.inner.timecode = v; })
      .def_property(
          "data", [](const AudioFrameV2Wrapper &s) { return s.get_data(); },
          [](AudioFrameV2Wrapper &s, py::array_t<float> arr) {
            s.set_data(std::move(arr));
          })
      .def_property(
          "channel_stride_in_bytes",
          [](const AudioFrameV2Wrapper &s) {
            return s.inner.channel_stride_in_bytes;
          },
          [](AudioFrameV2Wrapper &s, int v) {
            s.inner.channel_stride_in_bytes = v;
          })
      .def_property(
          "metadata",
          [](const AudioFrameV2Wrapper &s) { return s.metadata_str; },
          [](AudioFrameV2Wrapper &s, const std::string &v) {
            s.set_metadata(v);
          })
      .def_property(
          "timestamp",
          [](const AudioFrameV2Wrapper &s) { return s.inner.timestamp; },
          [](AudioFrameV2Wrapper &s, int64_t v) { s.inner.timestamp = v; });

  py::class_<AudioFrameV3Wrapper>(m, "AudioFrameV3")
      .def(py::init<int, int, int, int64_t, NDIlib_FourCC_audio_type_e, int,
                    const char *, int64_t>(),
           py::arg("sample_rate") = 48000, py::arg("no_channels") = 2,
           py::arg("no_samples") = 0,
           py::arg("timecode") = NDIlib_send_timecode_synthesize,
           py::arg("FourCC") = NDIlib_FourCC_audio_type_FLTP,
           py::arg("channel_stride_in_bytes") = 0,
           py::arg("p_metadata") = nullptr, py::arg("timestamp") = 0)
      .def_property(
          "sample_rate",
          [](const AudioFrameV3Wrapper &s) { return s.inner.sample_rate; },
          [](AudioFrameV3Wrapper &s, int v) { s.inner.sample_rate = v; })
      .def_property(
          "no_channels",
          [](const AudioFrameV3Wrapper &s) { return s.inner.no_channels; },
          [](AudioFrameV3Wrapper &s, int v) { s.inner.no_channels = v; })
      .def_property(
          "no_samples",
          [](const AudioFrameV3Wrapper &s) { return s.inner.no_samples; },
          [](AudioFrameV3Wrapper &s, int v) { s.inner.no_samples = v; })
      .def_property(
          "timecode",
          [](const AudioFrameV3Wrapper &s) { return s.inner.timecode; },
          [](AudioFrameV3Wrapper &s, int64_t v) { s.inner.timecode = v; })
      .def_property(
          "FourCC", [](const AudioFrameV3Wrapper &s) { return s.inner.FourCC; },
          [](AudioFrameV3Wrapper &s, NDIlib_FourCC_audio_type_e v) {
            s.inner.FourCC = v;
          })
      .def_property(
          "data", [](const AudioFrameV3Wrapper &s) { return s.get_data(); },
          [](AudioFrameV3Wrapper &s, py::array_t<uint8_t> arr) {
            s.set_data(std::move(arr));
          })
      .def_property(
          "channel_stride_in_bytes",
          [](const AudioFrameV3Wrapper &s) {
            return s.inner.channel_stride_in_bytes;
          },
          [](AudioFrameV3Wrapper &s, int v) {
            s.inner.channel_stride_in_bytes = v;
          })
      .def_property(
          "metadata",
          [](const AudioFrameV3Wrapper &s) { return s.metadata_str; },
          [](AudioFrameV3Wrapper &s, const std::string &v) {
            s.set_metadata(v);
          })
      .def_property(
          "timestamp",
          [](const AudioFrameV3Wrapper &s) { return s.inner.timestamp; },
          [](AudioFrameV3Wrapper &s, int64_t v) { s.inner.timestamp = v; });

  py::class_<MetadataFrameWrapper>(m, "MetadataFrame")
      .def(py::init<int, int64_t, const char *>(), py::arg("length") = 0,
           py::arg("timecode") = NDIlib_send_timecode_synthesize,
           py::arg("p_data") = nullptr)
      .def_property(
          "length",
          [](const MetadataFrameWrapper &s) { return s.inner.length; },
          [](MetadataFrameWrapper &s, int v) { s.inner.length = v; })
      .def_property(
          "timecode",
          [](const MetadataFrameWrapper &s) { return s.inner.timecode; },
          [](MetadataFrameWrapper &s, int64_t v) { s.inner.timecode = v; })
      .def_property(
          "data", [](const MetadataFrameWrapper &s) { return s.get_data(); },
          [](MetadataFrameWrapper &s, const std::string &v) { s.set_data(v); });

  py::class_<NDIlib_tally_t>(m, "Tally")
      .def(py::init<bool, bool>(), py::arg("on_program") = false,
           py::arg("on_preview") = false)
      .def_readwrite("on_program", &NDIlib_tally_t::on_program)
      .def_readwrite("on_preview", &NDIlib_tally_t::on_preview);

  py::enum_<NDIlib_receiver_type_e>(m, "ReceiverType", py::arithmetic())
      .value("RECEIVER_TYPE_NONE", NDIlib_receiver_type_none)
      .value("RECEIVER_TYPE_METADATA", NDIlib_receiver_type_metadata)
      .value("RECEIVER_TYPE_VIDEO", NDIlib_receiver_type_video)
      .value("RECEIVER_TYPE_AUDIO", NDIlib_receiver_type_audio)
      .value("RECEIVER_TYPE_MAX", NDIlib_receiver_type_max)
      .export_values();

  py::enum_<NDIlib_receiver_command_e>(m, "ReceiverCommand", py::arithmetic())
      .value("RECEIVER_COMMAND_NONE", NDIlib_receiver_command_none)
      .value("RECEIVER_COMMAND_CONNECT", NDIlib_receiver_command_connect)
      .value("RECEIVER_COMMAND_MAX", NDIlib_receiver_command_max)
      .export_values();

  py::class_<ListenerEventWrapper>(m, "ListenerEvent")
      .def_property_readonly(
          "uuid", [](const ListenerEventWrapper &s) { return s.uuid; })
      .def_property_readonly(
          "name", [](const ListenerEventWrapper &s) { return s.name; })
      .def_property_readonly(
          "value", [](const ListenerEventWrapper &s) { return s.value; });

  py::class_<ReceiverWrapper>(m, "Receiver")
      .def_property_readonly("uuid",
                             [](const ReceiverWrapper &s) { return s.uuid; })
      .def_property_readonly("name",
                             [](const ReceiverWrapper &s) { return s.name; })
      .def_property_readonly(
          "input_uuid", [](const ReceiverWrapper &s) { return s.input_uuid; })
      .def_property_readonly(
          "input_name", [](const ReceiverWrapper &s) { return s.input_name; })
      .def_property_readonly("address",
                             [](const ReceiverWrapper &s) { return s.address; })
      .def_property_readonly("streams",
                             [](const ReceiverWrapper &s) { return s.streams; })
      .def_property_readonly(
          "commands", [](const ReceiverWrapper &s) { return s.commands; })
      .def_readonly("events_subscribed", &ReceiverWrapper::events_subscribed);

  py::class_<SenderWrapper>(m, "Sender")
      .def_property_readonly("uuid",
                             [](const SenderWrapper &s) { return s.uuid; })
      .def_property_readonly("name",
                             [](const SenderWrapper &s) { return s.name; })
      .def_property_readonly("metadata",
                             [](const SenderWrapper &s) { return s.metadata; })
      .def_property_readonly("address",
                             [](const SenderWrapper &s) { return s.address; })
      .def_readonly("port", &SenderWrapper::port)
      .def_property_readonly("groups",
                             [](const SenderWrapper &s) { return s.groups; })
      .def_readonly("events_subscribed", &SenderWrapper::events_subscribed);

  // Processing.NDI.Lib
  m.def("initialize", &NDIlib_initialize);

  m.def("destroy", &NDIlib_destroy);

  m.def("version", &NDIlib_version);

  m.def("is_supported_CPU", &NDIlib_is_supported_CPU);

  // Processing.NDI.Find
  py::class_<FindCreateWrapper>(m, "FindCreate")
      .def(py::init<bool, const char *, const char *>(),
           py::arg("show_local_sources") = true, py::arg("p_groups") = nullptr,
           py::arg("p_extra_ips") = nullptr)
      .def_property(
          "show_local_sources",
          [](const FindCreateWrapper &s) { return s.inner.show_local_sources; },
          [](FindCreateWrapper &s, bool v) { s.inner.show_local_sources = v; })
      .def_property(
          "groups", [](const FindCreateWrapper &s) { return s.groups_str; },
          [](FindCreateWrapper &s, const std::string &v) { s.set_groups(v); })
      .def_property(
          "extra_ips",
          [](const FindCreateWrapper &s) { return s.extra_ips_str; },
          [](FindCreateWrapper &s, const std::string &v) {
            s.set_extra_ips(v);
          });

  m.def(
      "find_create_v2",
      [](const FindCreateWrapper *p_create_settings) {
        auto p_instance = NDIlib_find_create_v2(
            p_create_settings ? &p_create_settings->inner : nullptr);
        if (!p_instance)
          return py::object(py::none());
        return py::object(py::capsule(p_instance, "FindInstance"));
      },
      py::arg("create_settings") = nullptr);

  m.def(
      "find_destroy",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_find_instance_type *>(instance.get_pointer());
        NDIlib_find_destroy(p_instance);
      },
      py::arg("instance"));

  m.def(
      "find_get_current_sources",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_find_instance_type *>(instance.get_pointer());
        uint32_t count = 0;
        auto sources = NDIlib_find_get_current_sources(p_instance, &count);
        py::list out;
        for (uint32_t i = 0; i < count; ++i) {
          SourceWrapper w(sources[i].p_ndi_name, sources[i].p_url_address);
          out.append(std::move(w));
        }
        return out;
      },
      py::arg("instance"));

  m.def(
      "find_wait_for_sources",
      [](py::capsule instance, uint32_t timeout_in_ms) {
        auto p_instance =
            static_cast<NDIlib_find_instance_type *>(instance.get_pointer());
        py::gil_scoped_release release;
        return NDIlib_find_wait_for_sources(p_instance, timeout_in_ms);
      },
      py::arg("instance"), py::arg("timeout_in_ms"));

  // Processing.NDI.Recv
  py::enum_<NDIlib_recv_bandwidth_e>(m, "RecvBandwidth", py::arithmetic())
      .value("RECV_BANDWIDTH_METADATA_ONLY",
             NDIlib_recv_bandwidth_metadata_only)
      .value("RECV_BANDWIDTH_AUDIO_ONLY", NDIlib_recv_bandwidth_audio_only)
      .value("RECV_BANDWIDTH_LOWEST", NDIlib_recv_bandwidth_lowest)
      .value("RECV_BANDWIDTH_HIGHEST", NDIlib_recv_bandwidth_highest)
      .value("RECV_BANDWIDTH_MAX", NDIlib_recv_bandwidth_max)
      .export_values();

  py::enum_<NDIlib_recv_color_format_e>(m, "RecvColorFormat", py::arithmetic())
      .value("RECV_COLOR_FORMAT_BGRX_BGRA", NDIlib_recv_color_format_BGRX_BGRA)
      .value("RECV_COLOR_FORMAT_UYVY_BGRA", NDIlib_recv_color_format_UYVY_BGRA)
      .value("RECV_COLOR_FORMAT_RGBX_RGBA", NDIlib_recv_color_format_RGBX_RGBA)
      .value("RECV_COLOR_FORMAT_UYVY_RGBA", NDIlib_recv_color_format_UYVY_RGBA)
      .value("RECV_COLOR_FORMAT_FASTEST", NDIlib_recv_color_format_fastest)
      .value("RECV_COLOR_FORMAT_BEST", NDIlib_recv_color_format_best)
      .value("RECV_COLOR_FORMAT_E_BGRX_BGRA",
             NDIlib_recv_color_format_e_BGRX_BGRA)
      .value("RECV_COLOR_FORMAT_E_UYVY_BGRA",
             NDIlib_recv_color_format_e_UYVY_BGRA)
      .value("RECV_COLOR_FORMAT_E_RGBX_RGBA",
             NDIlib_recv_color_format_e_RGBX_RGBA)
      .value("RECV_COLOR_FORMAT_E_UYVY_RGBA",
             NDIlib_recv_color_format_e_UYVY_RGBA)
#ifdef _WIN32
      .value("RECV_COLOR_FORMAT_BGRX_BGRA_FLIPPED",
             NDIlib_recv_color_format_BGRX_BGRA_flipped)
#endif
      .value("RECV_COLOR_FORMAT_MAX", NDIlib_recv_color_format_max)
      .export_values();

  py::class_<RecvCreateV3Wrapper>(m, "RecvCreateV3")
      .def(py::init([](py::object source_to_connect_to,
                       NDIlib_recv_color_format_e color_format,
                       NDIlib_recv_bandwidth_e bandwidth,
                       bool allow_video_fields, const char *p_ndi_recv_name) {
             NDIlib_source_t src{};
             if (!source_to_connect_to.is_none()) {
               auto &w = source_to_connect_to.cast<SourceWrapper &>();
               src = w.inner;
             }
             return new RecvCreateV3Wrapper(src, color_format, bandwidth,
                                            allow_video_fields,
                                            p_ndi_recv_name);
           }),
           py::arg("source_to_connect_to") = py::none(),
           py::arg("color_format") = NDIlib_recv_color_format_UYVY_BGRA,
           py::arg("bandwidth") = NDIlib_recv_bandwidth_highest,
           py::arg("allow_video_fields") = true,
           py::arg("p_ndi_recv_name") = nullptr)
      .def_property(
          "source_to_connect_to",
          [](const RecvCreateV3Wrapper &s) {
            auto w = std::make_unique<SourceWrapper>(
                s.source_ndi_name_str.empty() ? nullptr
                                              : s.source_ndi_name_str.c_str(),
                s.source_url_address_str.empty()
                    ? nullptr
                    : s.source_url_address_str.c_str());
            return w.release();
          },
          [](RecvCreateV3Wrapper &s, const SourceWrapper &src) {
            s.set_source_ptrs(src.inner.p_ndi_name, src.inner.p_url_address);
          })
      .def_property(
          "color_format",
          [](const RecvCreateV3Wrapper &s) { return s.inner.color_format; },
          [](RecvCreateV3Wrapper &s, NDIlib_recv_color_format_e v) {
            s.inner.color_format = v;
          })
      .def_property(
          "bandwidth",
          [](const RecvCreateV3Wrapper &s) { return s.inner.bandwidth; },
          [](RecvCreateV3Wrapper &s, NDIlib_recv_bandwidth_e v) {
            s.inner.bandwidth = v;
          })
      .def_property(
          "allow_video_fields",
          [](const RecvCreateV3Wrapper &s) {
            return s.inner.allow_video_fields;
          },
          [](RecvCreateV3Wrapper &s, bool v) {
            s.inner.allow_video_fields = v;
          })
      .def_property(
          "ndi_recv_name",
          [](const RecvCreateV3Wrapper &s) { return s.ndi_recv_name_str; },
          [](RecvCreateV3Wrapper &s, const std::string &v) {
            s.set_ndi_recv_name(v);
          });

  py::class_<NDIlib_recv_performance_t>(m, "RecvPerformance")
      .def(py::init<>())
      .def_readwrite("video_frames", &NDIlib_recv_performance_t::video_frames)
      .def_readwrite("audio_frames", &NDIlib_recv_performance_t::audio_frames)
      .def_readwrite("metadata_frames",
                     &NDIlib_recv_performance_t::metadata_frames);

  py::class_<NDIlib_recv_queue_t>(m, "RecvQueue")
      .def(py::init<>())
      .def_readwrite("video_frames", &NDIlib_recv_queue_t::video_frames)
      .def_readwrite("audio_frames", &NDIlib_recv_queue_t::audio_frames)
      .def_readwrite("metadata_frames", &NDIlib_recv_queue_t::metadata_frames);

  m.def(
      "recv_create_v3",
      [](const RecvCreateV3Wrapper *p_create_settings) -> py::object {
        auto p_instance = NDIlib_recv_create_v3(
            p_create_settings ? &p_create_settings->inner : nullptr);
        if (!p_instance)
          return py::none();
        return py::capsule(p_instance, "RecvInstance");
      },
      py::arg("create_settings") = nullptr);

  m.def(
      "recv_destroy",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        NDIlib_recv_destroy(p_instance);
      },
      py::arg("instance"));

  m.def(
      "recv_connect",
      [](py::capsule instance, const SourceWrapper *p_src) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        NDIlib_recv_connect(p_instance, p_src ? &p_src->inner : nullptr);
      },
      py::arg("instance"), py::arg("source") = nullptr);

  m.def(
      "recv_get_source_name",
      [](py::capsule instance, uint32_t timeout_in_ms) -> py::object {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        const char *p_source_name = nullptr;
        py::gil_scoped_release release;
        bool result = NDIlib_recv_get_source_name(p_instance, &p_source_name,
                                                  timeout_in_ms);
        py::gil_scoped_acquire acquire;
        if (result && p_source_name)
          return py::str(p_source_name);
        return py::none();
      },
      py::arg("instance"), py::arg("timeout_in_ms") = 0);

  m.def(
      "recv_capture_v2",
      [](py::capsule instance, uint32_t timeout_in_ms) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        NDIlib_video_frame_v2_t video_frame{};
        NDIlib_audio_frame_v2_t audio_frame{};
        NDIlib_metadata_frame_t metadata_frame{};
        NDIlib_frame_type_e type;
        {
          py::gil_scoped_release release;
          type = NDIlib_recv_capture_v2(p_instance, &video_frame, &audio_frame,
                                        &metadata_frame, timeout_in_ms);
        }
        // Build wrapper objects that hold the NDI-allocated pointers.
        // Callers must pass them back to recv_free_* before destroying.
        VideoFrameV2Wrapper vw(
            video_frame.xres, video_frame.yres, video_frame.FourCC,
            video_frame.frame_rate_N, video_frame.frame_rate_D,
            video_frame.picture_aspect_ratio, video_frame.frame_format_type,
            video_frame.timecode, video_frame.line_stride_in_bytes,
            video_frame.p_metadata, video_frame.timestamp);
        vw.inner.p_data = video_frame.p_data;

        AudioFrameV2Wrapper aw(audio_frame.sample_rate, audio_frame.no_channels,
                               audio_frame.no_samples, audio_frame.timecode,
                               audio_frame.channel_stride_in_bytes,
                               audio_frame.p_metadata, audio_frame.timestamp);
        aw.inner.p_data = audio_frame.p_data;

        MetadataFrameWrapper mw(metadata_frame.length, metadata_frame.timecode,
                                nullptr);
        mw.inner.p_data = metadata_frame.p_data;

        return py::make_tuple(type, std::move(vw), std::move(aw),
                              std::move(mw));
      },
      py::arg("instance"), py::arg("timeout_in_ms"));

  m.def(
      "recv_capture_v3",
      [](py::capsule instance, uint32_t timeout_in_ms) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        NDIlib_video_frame_v2_t video_frame{};
        NDIlib_audio_frame_v3_t audio_frame{};
        NDIlib_metadata_frame_t metadata_frame{};
        NDIlib_frame_type_e type;
        {
          py::gil_scoped_release release;
          type = NDIlib_recv_capture_v3(p_instance, &video_frame, &audio_frame,
                                        &metadata_frame, timeout_in_ms);
        }
        VideoFrameV2Wrapper vw(
            video_frame.xres, video_frame.yres, video_frame.FourCC,
            video_frame.frame_rate_N, video_frame.frame_rate_D,
            video_frame.picture_aspect_ratio, video_frame.frame_format_type,
            video_frame.timecode, video_frame.line_stride_in_bytes,
            video_frame.p_metadata, video_frame.timestamp);
        vw.inner.p_data = video_frame.p_data;

        AudioFrameV3Wrapper aw(audio_frame.sample_rate, audio_frame.no_channels,
                               audio_frame.no_samples, audio_frame.timecode,
                               audio_frame.FourCC,
                               audio_frame.channel_stride_in_bytes,
                               audio_frame.p_metadata, audio_frame.timestamp);
        aw.inner.p_data = audio_frame.p_data;

        MetadataFrameWrapper mw(metadata_frame.length, metadata_frame.timecode,
                                nullptr);
        mw.inner.p_data = metadata_frame.p_data;

        return py::make_tuple(type, std::move(vw), std::move(aw),
                              std::move(mw));
      },
      py::arg("instance"), py::arg("timeout_in_ms"));

  m.def(
      "recv_free_video_v2",
      [](py::capsule instance, const VideoFrameV2Wrapper *p_video_data) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        if (p_video_data)
          NDIlib_recv_free_video_v2(p_instance, &p_video_data->inner);
      },
      py::arg("instance"), py::arg("video_data") = nullptr);

  m.def(
      "recv_free_audio_v2",
      [](py::capsule instance, const AudioFrameV2Wrapper *p_audio_data) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        if (p_audio_data)
          NDIlib_recv_free_audio_v2(p_instance, &p_audio_data->inner);
      },
      py::arg("instance"), py::arg("audio_data") = nullptr);

  m.def(
      "recv_free_audio_v3",
      [](py::capsule instance, const AudioFrameV3Wrapper *p_audio_data) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        if (p_audio_data)
          NDIlib_recv_free_audio_v3(p_instance, &p_audio_data->inner);
      },
      py::arg("instance"), py::arg("audio_data") = nullptr);

  m.def(
      "recv_free_metadata",
      [](py::capsule instance, const MetadataFrameWrapper *p_metadata) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        if (p_metadata)
          NDIlib_recv_free_metadata(p_instance, &p_metadata->inner);
      },
      py::arg("instance"), py::arg("metadata") = nullptr);

  m.def(
      "recv_free_string",
      [](py::capsule instance, const char *p_string) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        NDIlib_recv_free_string(p_instance, p_string);
      },
      py::arg("instance"), py::arg("string") = nullptr);

  m.def(
      "recv_send_metadata",
      [](py::capsule instance, const MetadataFrameWrapper *p_metadata) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_send_metadata(
            p_instance, p_metadata ? &p_metadata->inner : nullptr);
      },
      py::arg("instance"), py::arg("metadata_frame"));

  m.def(
      "recv_set_tally",
      [](py::capsule instance, const NDIlib_tally_t *p_tally) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_set_tally(p_instance, p_tally);
      },
      py::arg("instance"), py::arg("tally"));

  m.def(
      "recv_get_performance",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        NDIlib_recv_performance_t total;
        NDIlib_recv_performance_t dropped;
        NDIlib_recv_get_performance(p_instance, &total, &dropped);
        return std::tuple<NDIlib_recv_performance_t, NDIlib_recv_performance_t>(
            total, dropped);
      },
      py::arg("instance"));

  m.def(
      "recv_get_queue",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        NDIlib_recv_queue_t total;
        NDIlib_recv_get_queue(p_instance, &total);
        return total;
      },
      py::arg("instance"));

  m.def(
      "recv_clear_connection_metadata",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        NDIlib_recv_clear_connection_metadata(p_instance);
      },
      py::arg("instance"));

  m.def(
      "recv_add_connection_metadata",
      [](py::capsule instance, const MetadataFrameWrapper *p_metadata) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        NDIlib_recv_add_connection_metadata(
            p_instance, p_metadata ? &p_metadata->inner : nullptr);
      },
      py::arg("instance"), py::arg("metadata"));

  m.def(
      "recv_get_no_connections",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_get_no_connections(p_instance);
      },
      py::arg("instance"));

  m.def(
      "recv_get_web_control",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        auto str = NDIlib_recv_get_web_control(p_instance);
        if (!str)
          return py::str("");
        py::str result(str);
        NDIlib_recv_free_string(p_instance, str);
        return result;
      },
      py::arg("instance"));

  // Processing.NDI.Recv.ex
  m.def(
      "recv_ptz_is_supported",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_ptz_is_supported(p_instance);
      },
      py::arg("instance"));

  m.def(
      "recv_recording_is_supported",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_recording_is_supported(p_instance);
      },
      py::arg("instance"));

  m.def(
      "recv_ptz_zoom",
      [](py::capsule instance, const float zoom_value) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_ptz_zoom(p_instance, zoom_value);
      },
      py::arg("instance"), py::arg("zoom_value"));

  m.def(
      "recv_ptz_zoom_speed",
      [](py::capsule instance, const float zoom_speed) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_ptz_zoom_speed(p_instance, zoom_speed);
      },
      py::arg("instance"), py::arg("zoom_speed"));

  m.def(
      "recv_ptz_pan_tilt",
      [](py::capsule instance, const float pan_value, const float tilt_value) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_ptz_pan_tilt(p_instance, pan_value, tilt_value);
      },
      py::arg("instance"), py::arg("pan_value"), py::arg("tilt_value"));

  m.def(
      "recv_ptz_pan_tilt_speed",
      [](py::capsule instance, const float pan_speed, const float tilt_speed) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_ptz_pan_tilt_speed(p_instance, pan_speed,
                                              tilt_speed);
      },
      py::arg("instance"), py::arg("pan_speed"), py::arg("tilt_speed"));

  m.def(
      "recv_ptz_store_preset",
      [](py::capsule instance, const int preset_no) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_ptz_store_preset(p_instance, preset_no);
      },
      py::arg("instance"), py::arg("preset_no"));

  m.def(
      "recv_ptz_recall_preset",
      [](py::capsule instance, const int preset_no, const float speed) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_ptz_recall_preset(p_instance, preset_no, speed);
      },
      py::arg("instance"), py::arg("preset_no"), py::arg("speed"));

  m.def(
      "recv_ptz_auto_focus",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_ptz_auto_focus(p_instance);
      },
      py::arg("instance"));

  m.def(
      "recv_ptz_focus",
      [](py::capsule instance, const float focus_value) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_ptz_focus(p_instance, focus_value);
      },
      py::arg("instance"), py::arg("focus_value"));

  m.def(
      "recv_ptz_focus_speed",
      [](py::capsule instance, const float focus_speed) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_ptz_focus_speed(p_instance, focus_speed);
      },
      py::arg("instance"), py::arg("focus_speed"));

  m.def(
      "recv_ptz_white_balance_auto",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_ptz_white_balance_auto(p_instance);
      },
      py::arg("instance"));

  m.def(
      "recv_ptz_white_balance_indoor",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_ptz_white_balance_indoor(p_instance);
      },
      py::arg("instance"));

  m.def(
      "recv_ptz_white_balance_outdoor",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_ptz_white_balance_outdoor(p_instance);
      },
      py::arg("instance"));

  m.def(
      "recv_ptz_white_balance_oneshot",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_ptz_white_balance_oneshot(p_instance);
      },
      py::arg("instance"));

  m.def(
      "recv_ptz_white_balance_manual",
      [](py::capsule instance, const float red, const float blue) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_ptz_white_balance_manual(p_instance, red, blue);
      },
      py::arg("instance"), py::arg("red"), py::arg("blue"));

  m.def(
      "recv_ptz_exposure_auto",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_ptz_exposure_auto(p_instance);
      },
      py::arg("instance"));

  m.def(
      "recv_ptz_exposure_manual",
      [](py::capsule instance, const float exposure_level) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_ptz_exposure_manual(p_instance, exposure_level);
      },
      py::arg("instance"), py::arg("exposure_level"));

  m.def(
      "recv_ptz_exposure_manual_v2",
      [](py::capsule instance, const float iris, const float gain,
         const float shutter_speed) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_ptz_exposure_manual_v2(p_instance, iris, gain,
                                                  shutter_speed);
      },
      py::arg("instance"), py::arg("iris"), py::arg("gain"),
      py::arg("shutter_speed"));

  m.def(
      "recv_recording_start",
      [](py::capsule instance, const char *p_filename_hint) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_recording_start(p_instance, p_filename_hint);
      },
      py::arg("instance"), py::arg("filename_hint"));

  m.def(
      "recv_recording_stop",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_recording_stop(p_instance);
      },
      py::arg("instance"));

  m.def(
      "recv_recording_set_audio_level",
      [](py::capsule instance, const float level_dB) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_recording_set_audio_level(p_instance, level_dB);
      },
      py::arg("instance"), py::arg("level_dB"));

  m.def(
      "recv_recording_is_recording",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_recording_is_recording(p_instance);
      },
      py::arg("instance"));

  m.def(
      "recv_recording_get_filename",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        auto str = NDIlib_recv_recording_get_filename(p_instance);
        if (!str)
          return py::str("");
        py::str result(str);
        NDIlib_recv_free_string(p_instance, str);
        return result;
      },
      py::arg("instance"));

  m.def(
      "recv_recording_get_error",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        auto str = NDIlib_recv_recording_get_error(p_instance);
        if (!str)
          return py::str("");
        py::str result(str);
        NDIlib_recv_free_string(p_instance, str);
        return result;
      },
      py::arg("instance"));

  py::class_<NDIlib_recv_recording_time_t>(m, "RecvRecordingTime")
      .def(py::init<>())
      .def_readwrite("no_frames", &NDIlib_recv_recording_time_t::no_frames)
      .def_readwrite("start_time", &NDIlib_recv_recording_time_t::start_time)
      .def_readwrite("last_time", &NDIlib_recv_recording_time_t::last_time);

  m.def(
      "recv_recording_get_times",
      [](py::capsule instance, NDIlib_recv_recording_time_t *p_times) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_recording_get_times(p_instance, p_times);
      },
      py::arg("instance"), py::arg("times"));

  // Processing.NDI.Send
  py::class_<SendCreateWrapper>(m, "SendCreate")
      .def(py::init<const char *, const char *, bool, bool>(),
           py::arg("p_ndi_name") = nullptr, py::arg("p_groups") = nullptr,
           py::arg("clock_video") = true, py::arg("clock_audio") = true)
      .def_property(
          "ndi_name", [](const SendCreateWrapper &s) { return s.ndi_name_str; },
          [](SendCreateWrapper &s, const std::string &v) { s.set_ndi_name(v); })
      .def_property(
          "groups", [](const SendCreateWrapper &s) { return s.groups_str; },
          [](SendCreateWrapper &s, const std::string &v) { s.set_groups(v); })
      .def_property(
          "clock_video",
          [](const SendCreateWrapper &s) { return s.inner.clock_video; },
          [](SendCreateWrapper &s, bool v) { s.inner.clock_video = v; })
      .def_property(
          "clock_audio",
          [](const SendCreateWrapper &s) { return s.inner.clock_audio; },
          [](SendCreateWrapper &s, bool v) { s.inner.clock_audio = v; });

  m.def(
      "send_create",
      [](const SendCreateWrapper *p_create_settings) -> py::object {
        auto p_instance = NDIlib_send_create(
            p_create_settings ? &p_create_settings->inner : nullptr);
        if (!p_instance)
          return py::none();
        return py::capsule(p_instance, "SendInstance");
      },
      py::arg("create_settings") = nullptr);

  m.def(
      "send_destroy",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        NDIlib_send_destroy(p_instance);
      },
      py::arg("instance"));

  m.def(
      "send_send_video_v2",
      [](py::capsule instance, const VideoFrameV2Wrapper *p_video_data) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        NDIlib_send_send_video_v2(
            p_instance, p_video_data ? &p_video_data->inner : nullptr);
      },
      py::arg("instance"), py::arg("video_data"));

  m.def(
      "send_send_video_async_v2",
      [](py::capsule instance, const VideoFrameV2Wrapper *p_video_data) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        NDIlib_send_send_video_async_v2(
            p_instance, p_video_data ? &p_video_data->inner : nullptr);
      },
      py::arg("instance"), py::arg("video_data"));

  m.def(
      "send_send_audio_v2",
      [](py::capsule instance, const AudioFrameV2Wrapper *p_audio_data) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        NDIlib_send_send_audio_v2(
            p_instance, p_audio_data ? &p_audio_data->inner : nullptr);
      },
      py::arg("instance"), py::arg("audio_data"));

  m.def(
      "send_send_audio_v3",
      [](py::capsule instance, const AudioFrameV3Wrapper *p_audio_data) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        NDIlib_send_send_audio_v3(
            p_instance, p_audio_data ? &p_audio_data->inner : nullptr);
      },
      py::arg("instance"), py::arg("audio_data"));

  m.def(
      "send_send_metadata",
      [](py::capsule instance, const MetadataFrameWrapper *p_metadata) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        NDIlib_send_send_metadata(p_instance,
                                  p_metadata ? &p_metadata->inner : nullptr);
      },
      py::arg("instance"), py::arg("metadata"));

  m.def(
      "send_capture",
      [](py::capsule instance, MetadataFrameWrapper *p_metadata,
         uint32_t timeout_in_ms) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        py::gil_scoped_release release;
        NDIlib_send_capture(p_instance,
                            p_metadata ? &p_metadata->inner : nullptr,
                            timeout_in_ms);
      },
      py::arg("instance"), py::arg("metadata"), py::arg("timeout_in_ms"));

  m.def(
      "send_free_metadata",
      [](py::capsule instance, const MetadataFrameWrapper *p_metadata) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        NDIlib_send_free_metadata(p_instance,
                                  p_metadata ? &p_metadata->inner : nullptr);
      },
      py::arg("instance"), py::arg("metadata"));

  m.def(
      "send_get_tally",
      [](py::capsule instance, NDIlib_tally_t *p_tally,
         uint32_t timeout_in_ms) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        py::gil_scoped_release release;
        return NDIlib_send_get_tally(p_instance, p_tally, timeout_in_ms);
      },
      py::arg("instance"), py::arg("tally"), py::arg("timeout_in_ms"));

  m.def(
      "send_get_no_connections",
      [](py::capsule instance, uint32_t timeout_in_ms) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        py::gil_scoped_release release;
        return NDIlib_send_get_no_connections(p_instance, timeout_in_ms);
      },
      py::arg("instance"), py::arg("timeout_in_ms"));

  m.def(
      "send_clear_connection_metadata",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        NDIlib_send_clear_connection_metadata(p_instance);
      },
      py::arg("instance"));

  m.def(
      "send_add_connection_metadata",
      [](py::capsule instance, const MetadataFrameWrapper *p_metadata) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        NDIlib_send_add_connection_metadata(
            p_instance, p_metadata ? &p_metadata->inner : nullptr);
      },
      py::arg("instance"), py::arg("metadata"));

  m.def(
      "send_set_failover",
      [](py::capsule instance, const SourceWrapper *p_failover_source) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        NDIlib_send_set_failover(p_instance, p_failover_source
                                                 ? &p_failover_source->inner
                                                 : nullptr);
      },
      py::arg("instance"), py::arg("failover_source"));

  m.def(
      "send_get_source_name",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        return NDIlib_send_get_source_name(p_instance);
      },
      py::arg("instance"));

  // Processing.NDI.Routing
  py::class_<RoutingCreateWrapper>(m, "RoutingCreate")
      .def(py::init<const char *, const char *>(),
           py::arg("p_ndi_name") = nullptr, py::arg("p_groups") = nullptr)
      .def_property(
          "ndi_name",
          [](const RoutingCreateWrapper &s) { return s.ndi_name_str; },
          [](RoutingCreateWrapper &s, const std::string &v) {
            s.set_ndi_name(v);
          })
      .def_property(
          "groups", [](const RoutingCreateWrapper &s) { return s.groups_str; },
          [](RoutingCreateWrapper &s, const std::string &v) {
            s.set_groups(v);
          });

  m.def(
      "routing_create",
      [](const RoutingCreateWrapper *p_create_settings) -> py::object {
        auto p_instance = NDIlib_routing_create(
            p_create_settings ? &p_create_settings->inner : nullptr);
        if (!p_instance)
          return py::none();
        return py::capsule(p_instance, "RoutingInstance");
      },
      py::arg("create_settings"));

  m.def(
      "routing_destroy",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_routing_instance_type *>(instance.get_pointer());
        NDIlib_routing_destroy(p_instance);
      },
      py::arg("instance"));

  m.def(
      "routing_change",
      [](py::capsule instance, const SourceWrapper *p_source) {
        auto p_instance =
            static_cast<NDIlib_routing_instance_type *>(instance.get_pointer());
        return NDIlib_routing_change(p_instance,
                                     p_source ? &p_source->inner : nullptr);
      },
      py::arg("instance"), py::arg("source"));

  m.def(
      "routing_clear",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_routing_instance_type *>(instance.get_pointer());
        NDIlib_routing_clear(p_instance);
      },
      py::arg("instance"));

  m.def(
      "routing_get_no_connections",
      [](py::capsule instance, uint32_t timeout_in_ms) {
        auto p_instance =
            static_cast<NDIlib_routing_instance_type *>(instance.get_pointer());
        py::gil_scoped_release release;
        return NDIlib_routing_get_no_connections(p_instance, timeout_in_ms);
      },
      py::arg("instance"), py::arg("timeout_in_ms"));

  m.def(
      "routing_get_source_name",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_routing_instance_type *>(instance.get_pointer());
        return NDIlib_routing_get_source_name(p_instance);
      },
      py::arg("instance"));

  // Processing.NDI.utilities
  py::class_<AudioFrameInterleaved16sWrapper>(m, "AudioFrameInterleaved16s")
      .def(py::init<int, int, int, int64_t, int>(),
           py::arg("sample_rate") = 48000, py::arg("no_channels") = 2,
           py::arg("no_samples") = 0,
           py::arg("timecode") = NDIlib_send_timecode_synthesize,
           py::arg("reference_level") = 0)
      .def_property(
          "sample_rate",
          [](const AudioFrameInterleaved16sWrapper &s) {
            return s.inner.sample_rate;
          },
          [](AudioFrameInterleaved16sWrapper &s, int v) {
            s.inner.sample_rate = v;
          })
      .def_property(
          "no_channels",
          [](const AudioFrameInterleaved16sWrapper &s) {
            return s.inner.no_channels;
          },
          [](AudioFrameInterleaved16sWrapper &s, int v) {
            s.inner.no_channels = v;
          })
      .def_property(
          "no_samples",
          [](const AudioFrameInterleaved16sWrapper &s) {
            return s.inner.no_samples;
          },
          [](AudioFrameInterleaved16sWrapper &s, int v) {
            s.inner.no_samples = v;
          })
      .def_property(
          "timecode",
          [](const AudioFrameInterleaved16sWrapper &s) {
            return s.inner.timecode;
          },
          [](AudioFrameInterleaved16sWrapper &s, int64_t v) {
            s.inner.timecode = v;
          })
      .def_property(
          "reference_level",
          [](const AudioFrameInterleaved16sWrapper &s) {
            return s.inner.reference_level;
          },
          [](AudioFrameInterleaved16sWrapper &s, int v) {
            s.inner.reference_level = v;
          })
      .def_property(
          "data",
          [](const AudioFrameInterleaved16sWrapper &s) { return s.get_data(); },
          [](AudioFrameInterleaved16sWrapper &s, py::array_t<int16_t> arr) {
            s.set_data(std::move(arr));
          });

  py::class_<AudioFrameInterleaved32sWrapper>(m, "AudioFrameInterleaved32s")
      .def(py::init<int, int, int, int64_t, int>(),
           py::arg("sample_rate") = 48000, py::arg("no_channels") = 2,
           py::arg("no_samples") = 0,
           py::arg("timecode") = NDIlib_send_timecode_synthesize,
           py::arg("reference_level") = 0)
      .def_property(
          "sample_rate",
          [](const AudioFrameInterleaved32sWrapper &s) {
            return s.inner.sample_rate;
          },
          [](AudioFrameInterleaved32sWrapper &s, int v) {
            s.inner.sample_rate = v;
          })
      .def_property(
          "no_channels",
          [](const AudioFrameInterleaved32sWrapper &s) {
            return s.inner.no_channels;
          },
          [](AudioFrameInterleaved32sWrapper &s, int v) {
            s.inner.no_channels = v;
          })
      .def_property(
          "no_samples",
          [](const AudioFrameInterleaved32sWrapper &s) {
            return s.inner.no_samples;
          },
          [](AudioFrameInterleaved32sWrapper &s, int v) {
            s.inner.no_samples = v;
          })
      .def_property(
          "timecode",
          [](const AudioFrameInterleaved32sWrapper &s) {
            return s.inner.timecode;
          },
          [](AudioFrameInterleaved32sWrapper &s, int64_t v) {
            s.inner.timecode = v;
          })
      .def_property(
          "reference_level",
          [](const AudioFrameInterleaved32sWrapper &s) {
            return s.inner.reference_level;
          },
          [](AudioFrameInterleaved32sWrapper &s, int v) {
            s.inner.reference_level = v;
          })
      .def_property(
          "data",
          [](const AudioFrameInterleaved32sWrapper &s) { return s.get_data(); },
          [](AudioFrameInterleaved32sWrapper &s, py::array_t<int32_t> arr) {
            s.set_data(std::move(arr));
          });

  py::class_<AudioFrameInterleaved32fWrapper>(m, "AudioFrameInterleaved32f")
      .def(py::init<int, int, int, int64_t>(), py::arg("sample_rate") = 48000,
           py::arg("no_channels") = 2, py::arg("no_samples") = 0,
           py::arg("timecode") = NDIlib_send_timecode_synthesize)
      .def_property(
          "sample_rate",
          [](const AudioFrameInterleaved32fWrapper &s) {
            return s.inner.sample_rate;
          },
          [](AudioFrameInterleaved32fWrapper &s, int v) {
            s.inner.sample_rate = v;
          })
      .def_property(
          "no_channels",
          [](const AudioFrameInterleaved32fWrapper &s) {
            return s.inner.no_channels;
          },
          [](AudioFrameInterleaved32fWrapper &s, int v) {
            s.inner.no_channels = v;
          })
      .def_property(
          "no_samples",
          [](const AudioFrameInterleaved32fWrapper &s) {
            return s.inner.no_samples;
          },
          [](AudioFrameInterleaved32fWrapper &s, int v) {
            s.inner.no_samples = v;
          })
      .def_property(
          "timecode",
          [](const AudioFrameInterleaved32fWrapper &s) {
            return s.inner.timecode;
          },
          [](AudioFrameInterleaved32fWrapper &s, int64_t v) {
            s.inner.timecode = v;
          })
      .def_property(
          "data",
          [](const AudioFrameInterleaved32fWrapper &s) { return s.get_data(); },
          [](AudioFrameInterleaved32fWrapper &s, py::array_t<float> arr) {
            s.set_data(std::move(arr));
          });

  m.def(
      "util_send_send_audio_interleaved_16s",
      [](py::capsule instance,
         const AudioFrameInterleaved16sWrapper *p_audio_data) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        NDIlib_util_send_send_audio_interleaved_16s(p_instance,
                                                    &p_audio_data->inner);
      },
      py::arg("instance"), py::arg("audio_data"));

  m.def(
      "util_send_send_audio_interleaved_32s",
      [](py::capsule instance,
         const AudioFrameInterleaved32sWrapper *p_audio_data) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        NDIlib_util_send_send_audio_interleaved_32s(p_instance,
                                                    &p_audio_data->inner);
      },
      py::arg("instance"), py::arg("audio_data"));

  m.def(
      "util_send_send_audio_interleaved_32f",
      [](py::capsule instance,
         const AudioFrameInterleaved32fWrapper *p_audio_data) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        NDIlib_util_send_send_audio_interleaved_32f(p_instance,
                                                    &p_audio_data->inner);
      },
      py::arg("instance"), py::arg("audio_data"));

  m.def(
      "util_audio_to_interleaved_16s_v2",
      [](const AudioFrameV2Wrapper *src, AudioFrameInterleaved16sWrapper *dst) {
        NDIlib_util_audio_to_interleaved_16s_v2(&src->inner, &dst->inner);
      },
      py::arg("src"), py::arg("dst"));

  m.def(
      "util_audio_to_interleaved_16s_v3",
      [](const AudioFrameV3Wrapper *src, AudioFrameInterleaved16sWrapper *dst) {
        NDIlib_util_audio_to_interleaved_16s_v3(&src->inner, &dst->inner);
      },
      py::arg("src"), py::arg("dst"));

  m.def(
      "util_audio_from_interleaved_16s_v2",
      [](const AudioFrameInterleaved16sWrapper *src, AudioFrameV2Wrapper *dst) {
        NDIlib_util_audio_from_interleaved_16s_v2(&src->inner, &dst->inner);
      },
      py::arg("src"), py::arg("dst"));

  m.def(
      "util_audio_from_interleaved_16s_v3",
      [](const AudioFrameInterleaved16sWrapper *src, AudioFrameV3Wrapper *dst) {
        NDIlib_util_audio_from_interleaved_16s_v3(&src->inner, &dst->inner);
      },
      py::arg("src"), py::arg("dst"));

  m.def(
      "util_audio_to_interleaved_32s_v2",
      [](const AudioFrameV2Wrapper *src, AudioFrameInterleaved32sWrapper *dst) {
        NDIlib_util_audio_to_interleaved_32s_v2(&src->inner, &dst->inner);
      },
      py::arg("src"), py::arg("dst"));

  m.def(
      "util_audio_to_interleaved_32s_v3",
      [](const AudioFrameV3Wrapper *src, AudioFrameInterleaved32sWrapper *dst) {
        NDIlib_util_audio_to_interleaved_32s_v3(&src->inner, &dst->inner);
      },
      py::arg("src"), py::arg("dst"));

  m.def(
      "util_audio_from_interleaved_32s_v2",
      [](const AudioFrameInterleaved32sWrapper *src, AudioFrameV2Wrapper *dst) {
        NDIlib_util_audio_from_interleaved_32s_v2(&src->inner, &dst->inner);
      },
      py::arg("src"), py::arg("dst"));

  m.def(
      "util_audio_from_interleaved_32s_v3",
      [](const AudioFrameInterleaved32sWrapper *src, AudioFrameV3Wrapper *dst) {
        NDIlib_util_audio_from_interleaved_32s_v3(&src->inner, &dst->inner);
      },
      py::arg("src"), py::arg("dst"));

  m.def(
      "util_audio_to_interleaved_32f_v2",
      [](const AudioFrameV2Wrapper *src, AudioFrameInterleaved32fWrapper *dst) {
        NDIlib_util_audio_to_interleaved_32f_v2(&src->inner, &dst->inner);
      },
      py::arg("src"), py::arg("dst"));

  m.def(
      "util_audio_to_interleaved_32f_v3",
      [](const AudioFrameV3Wrapper *src, AudioFrameInterleaved32fWrapper *dst) {
        NDIlib_util_audio_to_interleaved_32f_v3(&src->inner, &dst->inner);
      },
      py::arg("src"), py::arg("dst"));

  m.def(
      "util_audio_from_interleaved_32f_v2",
      [](const AudioFrameInterleaved32fWrapper *src, AudioFrameV2Wrapper *dst) {
        NDIlib_util_audio_from_interleaved_32f_v2(&src->inner, &dst->inner);
      },
      py::arg("src"), py::arg("dst"));

  m.def(
      "util_audio_from_interleaved_32f_v3",
      [](const AudioFrameInterleaved32fWrapper *src, AudioFrameV3Wrapper *dst) {
        NDIlib_util_audio_from_interleaved_32f_v3(&src->inner, &dst->inner);
      },
      py::arg("src"), py::arg("dst"));

  m.def(
      "util_V210_to_P216",
      [](const VideoFrameV2Wrapper *src_v210, VideoFrameV2Wrapper *dst_p216) {
        NDIlib_util_V210_to_P216(&src_v210->inner, &dst_p216->inner);
      },
      py::arg("src_v210"), py::arg("dst_p216"));

  m.def(
      "util_P216_to_V210",
      [](const VideoFrameV2Wrapper *src_p216, VideoFrameV2Wrapper *dst_v210) {
        NDIlib_util_P216_to_V210(&src_p216->inner, &dst_v210->inner);
      },
      py::arg("src_p216"), py::arg("dst_v210"));

  // Processing.NDI.deprecated
  // NOT IMPLEMENTED

  // Processing.NDI.FrameSync
  m.def(
      "framesync_create",
      [](py::capsule instance) -> py::object {
        auto p_recv_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        auto p_instance = NDIlib_framesync_create(p_recv_instance);
        if (!p_instance)
          return py::none();
        return py::capsule(p_instance, "FrameSyncInstance");
      },
      py::arg("receiver"));

  m.def(
      "framesync_destroy",
      [](py::capsule instance) {
        auto p_instance = static_cast<NDIlib_framesync_instance_type *>(
            instance.get_pointer());
        NDIlib_framesync_destroy(p_instance);
      },
      py::arg("instance"));

  m.def(
      "framesync_capture_audio",
      [](py::capsule instance, int sample_rate, int no_channels,
         int no_samples) {
        auto p_instance = static_cast<NDIlib_framesync_instance_type *>(
            instance.get_pointer());
        AudioFrameV2Wrapper w(sample_rate, no_channels, no_samples,
                              NDIlib_send_timecode_synthesize, 0, nullptr, 0);
        {
          py::gil_scoped_release release;
          NDIlib_framesync_capture_audio(p_instance, &w.inner, sample_rate,
                                         no_channels, no_samples);
        }
        return w;
      },
      py::arg("instance"), py::arg("sample_rate"), py::arg("no_channels"),
      py::arg("no_samples"));

  m.def(
      "framesync_capture_audio_v2",
      [](py::capsule instance, int sample_rate, int no_channels,
         int no_samples) {
        auto p_instance = static_cast<NDIlib_framesync_instance_type *>(
            instance.get_pointer());
        AudioFrameV3Wrapper w(sample_rate, no_channels, no_samples,
                              NDIlib_send_timecode_synthesize,
                              NDIlib_FourCC_audio_type_FLTP, 0, nullptr, 0);
        {
          py::gil_scoped_release release;
          NDIlib_framesync_capture_audio_v2(p_instance, &w.inner, sample_rate,
                                            no_channels, no_samples);
        }
        return w;
      },
      py::arg("instance"), py::arg("sample_rate"), py::arg("no_channels"),
      py::arg("no_samples"));

  m.def(
      "framesync_free_audio",
      [](py::capsule instance, AudioFrameV2Wrapper *p_audio_data) {
        auto p_instance = static_cast<NDIlib_framesync_instance_type *>(
            instance.get_pointer());
        NDIlib_framesync_free_audio(p_instance, &p_audio_data->inner);
      },
      py::arg("instance"), py::arg("audio_data"));

  m.def(
      "framesync_free_audio_v2",
      [](py::capsule instance, AudioFrameV3Wrapper *p_audio_data) {
        auto p_instance = static_cast<NDIlib_framesync_instance_type *>(
            instance.get_pointer());
        NDIlib_framesync_free_audio_v2(p_instance, &p_audio_data->inner);
      },
      py::arg("instance"), py::arg("audio_data"));

  m.def(
      "framesync_audio_queue_depth",
      [](py::capsule instance) {
        auto p_instance = static_cast<NDIlib_framesync_instance_type *>(
            instance.get_pointer());
        return NDIlib_framesync_audio_queue_depth(p_instance);
      },
      py::arg("instance"));

  m.def(
      "framesync_capture_video",
      [](py::capsule instance, NDIlib_frame_format_type_e field_type) {
        auto p_instance = static_cast<NDIlib_framesync_instance_type *>(
            instance.get_pointer());
        VideoFrameV2Wrapper w(0, 0, NDIlib_FourCC_video_type_UYVY, 30000, 1001,
                              0.0f, NDIlib_frame_format_type_progressive, 0, 0,
                              nullptr, 0);
        {
          py::gil_scoped_release release;
          NDIlib_framesync_capture_video(p_instance, &w.inner, field_type);
        }
        return w;
      },
      py::arg("instance"),
      py::arg("field_type") = NDIlib_frame_format_type_progressive);

  m.def(
      "framesync_free_video",
      [](py::capsule instance, VideoFrameV2Wrapper *p_video_data) {
        auto p_instance = static_cast<NDIlib_framesync_instance_type *>(
            instance.get_pointer());
        NDIlib_framesync_free_video(p_instance, &p_video_data->inner);
      },
      py::arg("instance"), py::arg("video_data"));

  // Processing.NDI.RecvListener

  py::class_<RecvListenerCreateWrapper>(m, "RecvListenerCreateV2")
      .def(py::init<>())
      .def(py::init<const std::string &>(),
           py::arg("url_address") = std::string())
      .def_property(
          "url_address",
          [](const RecvListenerCreateWrapper &s) { return s.url_address_str; },
          [](RecvListenerCreateWrapper &s, const std::string &v) {
            s.set_url_address(v);
          });

  m.def(
      "recv_listener_create",
      [](const RecvListenerCreateWrapper *p_create) {
        auto p =
            NDIlib_recv_listener_create(p_create ? &p_create->inner : nullptr);
        if (!p)
          return py::object(py::none());
        return py::object(py::capsule(p, "recv_listener", [](void *p) {
          NDIlib_recv_listener_destroy(
              static_cast<NDIlib_recv_listener_instance_t>(p));
        }));
      },
      py::arg("create_settings") = nullptr);

  m.def(
      "recv_listener_destroy",
      [](py::capsule instance) {
        NDIlib_recv_listener_destroy(
            static_cast<NDIlib_recv_listener_instance_t>(
                instance.get_pointer()));
      },
      py::arg("instance"));

  m.def(
      "recv_listener_is_connected",
      [](py::capsule instance) {
        return NDIlib_recv_listener_is_connected(
            static_cast<NDIlib_recv_listener_instance_t>(
                instance.get_pointer()));
      },
      py::arg("instance"));

  m.def(
      "recv_listener_get_server_url",
      [](py::capsule instance) -> py::object {
        auto url = NDIlib_recv_listener_get_server_url(
            static_cast<NDIlib_recv_listener_instance_t>(
                instance.get_pointer()));
        if (url)
          return py::str(url);
        return py::none();
      },
      py::arg("instance"));

  m.def(
      "recv_listener_get_receivers",
      [](py::capsule instance) {
        uint32_t num = 0;
        auto p = NDIlib_recv_listener_get_receivers(
            static_cast<NDIlib_recv_listener_instance_t>(
                instance.get_pointer()),
            &num);
        std::vector<ReceiverWrapper> result;
        for (uint32_t i = 0; i < num; ++i)
          result.emplace_back(p[i]);
        return result;
      },
      py::arg("instance"));

  m.def(
      "recv_listener_wait_for_receivers",
      [](py::capsule instance, uint32_t timeout_in_ms) {
        py::gil_scoped_release release;
        return NDIlib_recv_listener_wait_for_receivers(
            static_cast<NDIlib_recv_listener_instance_t>(
                instance.get_pointer()),
            timeout_in_ms);
      },
      py::arg("instance"), py::arg("timeout_in_ms"));

  m.def(
      "recv_listener_subscribe_events",
      [](py::capsule instance, const std::string &uuid) {
        NDIlib_recv_listener_subscribe_events(
            static_cast<NDIlib_recv_listener_instance_t>(
                instance.get_pointer()),
            uuid.c_str());
      },
      py::arg("instance"), py::arg("receiver_uuid"));

  m.def(
      "recv_listener_unsubscribe_events",
      [](py::capsule instance, const std::string &uuid) {
        NDIlib_recv_listener_unsubscribe_events(
            static_cast<NDIlib_recv_listener_instance_t>(
                instance.get_pointer()),
            uuid.c_str());
      },
      py::arg("instance"), py::arg("receiver_uuid"));

  m.def(
      "recv_listener_get_events",
      [](py::capsule instance, uint32_t timeout_in_ms) {
        uint32_t num = 0;
        auto p = NDIlib_recv_listener_get_events(
            static_cast<NDIlib_recv_listener_instance_t>(
                instance.get_pointer()),
            &num, timeout_in_ms);
        std::vector<ListenerEventWrapper> result;
        if (p) {
          for (uint32_t i = 0; i < num; ++i)
            result.emplace_back(p[i]);
          NDIlib_recv_listener_free_events(
              static_cast<NDIlib_recv_listener_instance_t>(
                  instance.get_pointer()),
              p);
        }
        return result;
      },
      py::arg("instance"), py::arg("timeout_in_ms") = 0);

  m.def(
      "recv_listener_send_connect",
      [](py::capsule instance, const std::string &receiver_uuid,
         py::object source_name) {
        const char *src = nullptr;
        std::string src_str;
        if (!source_name.is_none()) {
          src_str = source_name.cast<std::string>();
          src = src_str.c_str();
        }
        return NDIlib_recv_listener_send_connect(
            static_cast<NDIlib_recv_listener_instance_t>(
                instance.get_pointer()),
            receiver_uuid.c_str(), src);
      },
      py::arg("instance"), py::arg("receiver_uuid"),
      py::arg("source_name") = py::none());

  // Processing.NDI.SendListener

  py::class_<SendListenerCreateWrapper>(m, "SendListenerCreateV2")
      .def(py::init<>())
      .def(py::init<const std::string &>(),
           py::arg("url_address") = std::string())
      .def_property(
          "url_address",
          [](const SendListenerCreateWrapper &s) { return s.url_address_str; },
          [](SendListenerCreateWrapper &s, const std::string &v) {
            s.set_url_address(v);
          });

  m.def(
      "send_listener_create",
      [](const SendListenerCreateWrapper *p_create) {
        auto p =
            NDIlib_send_listener_create(p_create ? &p_create->inner : nullptr);
        if (!p)
          return py::object(py::none());
        return py::object(py::capsule(p, "send_listener", [](void *p) {
          NDIlib_send_listener_destroy(
              static_cast<NDIlib_send_listener_instance_t>(p));
        }));
      },
      py::arg("create_settings") = nullptr);

  m.def(
      "send_listener_destroy",
      [](py::capsule instance) {
        NDIlib_send_listener_destroy(
            static_cast<NDIlib_send_listener_instance_t>(
                instance.get_pointer()));
      },
      py::arg("instance"));

  m.def(
      "send_listener_is_connected",
      [](py::capsule instance) {
        return NDIlib_send_listener_is_connected(
            static_cast<NDIlib_send_listener_instance_t>(
                instance.get_pointer()));
      },
      py::arg("instance"));

  m.def(
      "send_listener_get_server_url",
      [](py::capsule instance) -> py::object {
        auto url = NDIlib_send_listener_get_server_url(
            static_cast<NDIlib_send_listener_instance_t>(
                instance.get_pointer()));
        if (url)
          return py::str(url);
        return py::none();
      },
      py::arg("instance"));

  m.def(
      "send_listener_get_senders",
      [](py::capsule instance) {
        uint32_t num = 0;
        auto p = NDIlib_send_listener_get_senders(
            static_cast<NDIlib_send_listener_instance_t>(
                instance.get_pointer()),
            &num);
        std::vector<SenderWrapper> result;
        for (uint32_t i = 0; i < num; ++i)
          result.emplace_back(p[i]);
        return result;
      },
      py::arg("instance"));

  m.def(
      "send_listener_wait_for_senders",
      [](py::capsule instance, uint32_t timeout_in_ms) {
        py::gil_scoped_release release;
        return NDIlib_send_listener_wait_for_senders(
            static_cast<NDIlib_send_listener_instance_t>(
                instance.get_pointer()),
            timeout_in_ms);
      },
      py::arg("instance"), py::arg("timeout_in_ms"));

  m.def(
      "send_listener_subscribe_events",
      [](py::capsule instance, const std::string &uuid) {
        NDIlib_send_listener_subscribe_events(
            static_cast<NDIlib_send_listener_instance_t>(
                instance.get_pointer()),
            uuid.c_str());
      },
      py::arg("instance"), py::arg("sender_uuid"));

  m.def(
      "send_listener_unsubscribe_events",
      [](py::capsule instance, const std::string &uuid) {
        NDIlib_send_listener_unsubscribe_events(
            static_cast<NDIlib_send_listener_instance_t>(
                instance.get_pointer()),
            uuid.c_str());
      },
      py::arg("instance"), py::arg("sender_uuid"));

  m.def(
      "send_listener_get_events",
      [](py::capsule instance, uint32_t timeout_in_ms) {
        uint32_t num = 0;
        auto p = NDIlib_send_listener_get_events(
            static_cast<NDIlib_send_listener_instance_t>(
                instance.get_pointer()),
            &num, timeout_in_ms);
        std::vector<ListenerEventWrapper> result;
        if (p) {
          for (uint32_t i = 0; i < num; ++i)
            result.emplace_back(p[i]);
          NDIlib_send_listener_free_events(
              static_cast<NDIlib_send_listener_instance_t>(
                  instance.get_pointer()),
              p);
        }
        return result;
      },
      py::arg("instance"), py::arg("timeout_in_ms") = 0);

  // Processing.NDI.RecvAdvertiser

  py::class_<RecvAdvertiserCreateWrapper>(m, "RecvAdvertiserCreateV2")
      .def(py::init<>())
      .def(py::init<const std::string &>(),
           py::arg("url_address") = std::string())
      .def_property(
          "url_address",
          [](const RecvAdvertiserCreateWrapper &s) {
            return s.url_address_str;
          },
          [](RecvAdvertiserCreateWrapper &s, const std::string &v) {
            s.set_url_address(v);
          });

  m.def(
      "recv_advertiser_create",
      [](const RecvAdvertiserCreateWrapper *p_create) {
        auto p = NDIlib_recv_advertiser_create(p_create ? &p_create->inner
                                                        : nullptr);
        if (!p)
          return py::object(py::none());
        return py::object(py::capsule(p, "recv_advertiser", [](void *p) {
          NDIlib_recv_advertiser_destroy(
              static_cast<NDIlib_recv_advertiser_instance_t>(p));
        }));
      },
      py::arg("create_settings") = nullptr);

  m.def(
      "recv_advertiser_destroy",
      [](py::capsule instance) {
        NDIlib_recv_advertiser_destroy(
            static_cast<NDIlib_recv_advertiser_instance_t>(
                instance.get_pointer()));
      },
      py::arg("instance"));

  m.def(
      "recv_advertiser_add_receiver",
      [](py::capsule advertiser, py::capsule receiver, bool allow_controlling,
         bool allow_monitoring, py::object input_group_name) {
        const char *name = nullptr;
        std::string name_str;
        if (!input_group_name.is_none()) {
          name_str = input_group_name.cast<std::string>();
          name = name_str.c_str();
        }
        return NDIlib_recv_advertiser_add_receiver(
            static_cast<NDIlib_recv_advertiser_instance_t>(
                advertiser.get_pointer()),
            static_cast<NDIlib_recv_instance_t>(receiver.get_pointer()),
            allow_controlling, allow_monitoring, name);
      },
      py::arg("advertiser"), py::arg("receiver"),
      py::arg("allow_controlling") = true, py::arg("allow_monitoring") = true,
      py::arg("input_group_name") = py::none());

  m.def(
      "recv_advertiser_del_receiver",
      [](py::capsule advertiser, py::capsule receiver) {
        return NDIlib_recv_advertiser_del_receiver(
            static_cast<NDIlib_recv_advertiser_instance_t>(
                advertiser.get_pointer()),
            static_cast<NDIlib_recv_instance_t>(receiver.get_pointer()));
      },
      py::arg("advertiser"), py::arg("receiver"));

  // Processing.NDI.SendAdvertiser

  py::class_<SendAdvertiserCreateWrapper>(m, "SendAdvertiserCreateV2")
      .def(py::init<>())
      .def(py::init<const std::string &>(),
           py::arg("url_address") = std::string())
      .def_property(
          "url_address",
          [](const SendAdvertiserCreateWrapper &s) {
            return s.url_address_str;
          },
          [](SendAdvertiserCreateWrapper &s, const std::string &v) {
            s.set_url_address(v);
          });

  m.def(
      "send_advertiser_create",
      [](const SendAdvertiserCreateWrapper *p_create) {
        auto p = NDIlib_send_advertiser_create(p_create ? &p_create->inner
                                                        : nullptr);
        if (!p)
          return py::object(py::none());
        return py::object(py::capsule(p, "send_advertiser", [](void *p) {
          NDIlib_send_advertiser_destroy(
              static_cast<NDIlib_send_advertiser_instance_t>(p));
        }));
      },
      py::arg("create_settings") = nullptr);

  m.def(
      "send_advertiser_destroy",
      [](py::capsule instance) {
        NDIlib_send_advertiser_destroy(
            static_cast<NDIlib_send_advertiser_instance_t>(
                instance.get_pointer()));
      },
      py::arg("instance"));

  m.def(
      "send_advertiser_add_sender",
      [](py::capsule advertiser, py::capsule sender, bool allow_monitoring) {
        return NDIlib_send_advertiser_add_sender(
            static_cast<NDIlib_send_advertiser_instance_t>(
                advertiser.get_pointer()),
            static_cast<NDIlib_send_instance_t>(sender.get_pointer()),
            allow_monitoring);
      },
      py::arg("advertiser"), py::arg("sender"),
      py::arg("allow_monitoring") = true);

  m.def(
      "send_advertiser_del_sender",
      [](py::capsule advertiser, py::capsule sender) {
        return NDIlib_send_advertiser_del_sender(
            static_cast<NDIlib_send_advertiser_instance_t>(
                advertiser.get_pointer()),
            static_cast<NDIlib_send_instance_t>(sender.get_pointer()));
      },
      py::arg("advertiser"), py::arg("sender"));
}
