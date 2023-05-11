#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include <Processing.NDI.Lib.h>

namespace py = pybind11;

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

  py::class_<NDIlib_source_t>(m, "Source")
      .def(py::init<const char *, const char *>(),
           py::arg("p_ndi_name") = nullptr, py::arg("p_url_address") = nullptr)
      .def_property(
          "ndi_name",
          [](const NDIlib_source_t &self) {
            if (!self.p_ndi_name)
              return py::str();
            auto ustr = PyUnicode_DecodeLocale(self.p_ndi_name, nullptr);
            return py::reinterpret_steal<py::str>(ustr);
          },
          [](NDIlib_source_t &self, const std::string &name) {
            static std::unordered_map<NDIlib_source_t *, std::string> strs;
            strs[&self] = py::str(name);
            self.p_ndi_name = strs[&self].c_str();
          })
      .def_property(
          "url_address",
          [](const NDIlib_source_t &self) {
            if (!self.p_url_address)
              return py::str();
            auto ustr = PyUnicode_DecodeLocale(self.p_url_address, nullptr);
            return py::reinterpret_steal<py::str>(ustr);
          },
          [](NDIlib_source_t &self, const std::string &address) {
            static std::unordered_map<NDIlib_source_t *, std::string> strs;
            strs[&self] = py::str(address);
            self.p_url_address = strs[&self].c_str();
          });

  py::class_<NDIlib_video_frame_v2_t>(m, "VideoFrameV2")
      .def(py::init<int, int, NDIlib_FourCC_video_type_e, int, int, float,
                    NDIlib_frame_format_type_e, int64_t, uint8_t *, int,
                    const char *, int64_t>(),
           py::arg("xres") = 0, py::arg("yres") = 0,
           py::arg("FourCC") = NDIlib_FourCC_video_type_UYVY,
           py::arg("frame_rate_N") = 30000, py::arg("frame_rate_D") = 1001,
           py::arg("picture_aspect_ratio") = 0.0f,
           py::arg("frame_format_type") = NDIlib_frame_format_type_progressive,
           py::arg("timecode") = 0, py::arg("p_data") = 0,
           py::arg("line_stride_in_bytes") = 0, py::arg("p_metadata") = nullptr,
           py::arg("timestamp") = 0)
      .def_readwrite("xres", &NDIlib_video_frame_v2_t::xres)
      .def_readwrite("yres", &NDIlib_video_frame_v2_t::yres)
      .def_readwrite("FourCC", &NDIlib_video_frame_v2_t::FourCC)
      .def_readwrite("frame_rate_N", &NDIlib_video_frame_v2_t::frame_rate_N)
      .def_readwrite("frame_rate_D", &NDIlib_video_frame_v2_t::frame_rate_D)
      .def_readwrite("picture_aspect_ratio",
                     &NDIlib_video_frame_v2_t::picture_aspect_ratio)
      .def_readwrite("frame_format_type",
                     &NDIlib_video_frame_v2_t::frame_format_type)
      .def_readwrite("timecode", &NDIlib_video_frame_v2_t::timecode)
      .def_property(
          "data",
          [](const NDIlib_video_frame_v2_t &self) {
            int r = self.yres;
            int c = self.xres;
            size_t b1 = self.line_stride_in_bytes;
            size_t b2 = c > 0 ? b1 / c : 0;
            size_t b3 = sizeof(uint8_t);
            auto buffer_info = py::buffer_info(
                self.p_data, b3, py::format_descriptor<uint8_t>::format(), 3,
                {r, c, int(b2)}, {b1, b2, b3});
            return py::array(buffer_info);
          },
          [](NDIlib_video_frame_v2_t &self, const py::array_t<uint8_t> &array) {
            auto info = array.request();
            self.p_data = static_cast<uint8_t *>(info.ptr);
            self.picture_aspect_ratio = info.shape[1] / float(info.shape[0]);
            self.xres = info.shape[1];
            self.yres = info.shape[0];
            self.line_stride_in_bytes = info.strides[0];
          })
      .def_readwrite("line_stride_in_bytes",
                     &NDIlib_video_frame_v2_t::line_stride_in_bytes)
      .def_property(
          "metadata",
          [](const NDIlib_video_frame_v2_t &self) {
            if (!self.p_metadata)
              return py::str();
            auto ustr = PyUnicode_DecodeLocale(self.p_metadata, nullptr);
            return py::reinterpret_steal<py::str>(ustr);
          },
          [](NDIlib_video_frame_v2_t &self, const std::string &data) {
            static std::unordered_map<NDIlib_video_frame_v2_t *, std::string>
                strs;
            strs[&self] = py::str(data);
            self.p_metadata = strs[&self].c_str();
          })
      .def_readwrite("timestamp", &NDIlib_video_frame_v2_t::timestamp);

  py::class_<NDIlib_audio_frame_v2_t>(m, "AudioFrameV2")
      .def(py::init<int, int, int, int64_t, float *, int, const char *,
                    int64_t>(),
           py::arg("sample_rate") = 48000, py::arg("no_channels") = 2,
           py::arg("no_samples") = 0,
           py::arg("timecode") = NDIlib_send_timecode_synthesize,
           py::arg("p_data") = 0, py::arg("channel_stride_in_bytes") = 0,
           py::arg("p_metadata") = nullptr, py::arg("timestamp") = 0)
      .def_readwrite("sample_rate", &NDIlib_audio_frame_v2_t::sample_rate)
      .def_readwrite("no_channels", &NDIlib_audio_frame_v2_t::no_channels)
      .def_readwrite("no_samples", &NDIlib_audio_frame_v2_t::no_samples)
      .def_readwrite("timecode", &NDIlib_audio_frame_v2_t::timecode)
      .def_property(
          "data",
          [](const NDIlib_audio_frame_v2_t &self) {
            size_t col = self.no_samples;
            size_t row = self.no_channels;
            size_t size = sizeof(float);
            auto buffer_info = py::buffer_info(
                self.p_data, size, py::format_descriptor<float>::format(), 2,
                {row, col}, {col * size, size});
            return py::array(buffer_info);
          },
          [](NDIlib_audio_frame_v2_t &self, py::array_t<float> &array) {
            auto info = array.request();
            self.p_data = static_cast<float *>(info.ptr);
            self.no_channels = info.shape[0];
            self.no_samples = info.shape[1];
            self.channel_stride_in_bytes = info.strides[0];
          })
      .def_readwrite("channel_stride_in_bytes",
                     &NDIlib_audio_frame_v2_t::channel_stride_in_bytes)
      .def_property(
          "metadata",
          [](const NDIlib_audio_frame_v2_t &self) {
            if (!self.p_metadata)
              return py::str();
            auto ustr = PyUnicode_DecodeLocale(self.p_metadata, nullptr);
            return py::reinterpret_steal<py::str>(ustr);
          },
          [](NDIlib_audio_frame_v2_t &self, const std::string &data) {
            static std::unordered_map<NDIlib_audio_frame_v2_t *, std::string>
                strs;
            strs[&self] = py::str(data);
            self.p_metadata = strs[&self].c_str();
          })
      .def_readwrite("timestamp", &NDIlib_audio_frame_v2_t::timestamp);

  py::class_<NDIlib_audio_frame_v3_t>(m, "AudioFrameV3")
      .def(py::init<int, int, int, int64_t, NDIlib_FourCC_audio_type_e,
                    uint8_t *, int, const char *, int64_t>(),
           py::arg("sample_rate") = 48000, py::arg("no_channels") = 2,
           py::arg("no_samples") = 0,
           py::arg("timecode") = NDIlib_send_timecode_synthesize,
           py::arg("FourCC") = NDIlib_FourCC_audio_type_FLTP,
           py::arg("p_data") = 0, py::arg("channel_stride_in_bytes") = 0,
           py::arg("p_metadata") = nullptr, py::arg("timestamp") = 0)
      .def_readwrite("sample_rate", &NDIlib_audio_frame_v3_t::sample_rate)
      .def_readwrite("no_channels", &NDIlib_audio_frame_v3_t::no_channels)
      .def_readwrite("no_samples", &NDIlib_audio_frame_v3_t::no_samples)
      .def_readwrite("timecode", &NDIlib_audio_frame_v3_t::timecode)
      .def_readwrite("FourCC", &NDIlib_audio_frame_v3_t::FourCC)
      .def_property(
          "data",
          [](const NDIlib_audio_frame_v3_t &self) {
            size_t col = self.no_samples;
            size_t row = self.no_channels;
            size_t size = sizeof(uint8_t);
            auto buffer_info = py::buffer_info(
                self.p_data, size, py::format_descriptor<uint8_t>::format(), 2,
                {row, col}, {col * size * 4, size});
            return py::array(buffer_info);
          },
          [](NDIlib_audio_frame_v3_t &self, py::array_t<uint8_t> &array) {
            auto info = array.request();
            self.p_data = static_cast<uint8_t *>(info.ptr);
            self.no_channels = info.shape[0];
            self.no_samples = info.shape[1];
            self.channel_stride_in_bytes = info.strides[0];
          })
      .def_readwrite("channel_stride_in_bytes",
                     &NDIlib_audio_frame_v3_t::channel_stride_in_bytes)
      .def_property(
          "metadata",
          [](const NDIlib_audio_frame_v3_t &self) {
            if (!self.p_metadata)
              return py::str();
            auto ustr = PyUnicode_DecodeLocale(self.p_metadata, nullptr);
            return py::reinterpret_steal<py::str>(ustr);
          },
          [](NDIlib_audio_frame_v3_t &self, const std::string &data) {
            static std::unordered_map<NDIlib_audio_frame_v3_t *, std::string>
                strs;
            strs[&self] = py::str(data);
            self.p_metadata = strs[&self].c_str();
          })
      .def_readwrite("timestamp", &NDIlib_audio_frame_v3_t::timestamp);

  py::class_<NDIlib_metadata_frame_t>(m, "MetadataFrame")
      .def(py::init<int, int64_t, char *>(), py::arg("length") = 0,
           py::arg("timecode") = NDIlib_send_timecode_synthesize,
           py::arg("p_data") = nullptr)
      .def_readwrite("length", &NDIlib_metadata_frame_t::length)
      .def_readwrite("timecode", &NDIlib_metadata_frame_t::timecode)
      .def_property(
          "data",
          [](const NDIlib_metadata_frame_t &self) {
            if (!self.p_data)
              return py::str();
            auto ustr = PyUnicode_DecodeLocale(self.p_data, nullptr);
            return py::reinterpret_steal<py::str>(ustr);
          },
          [](NDIlib_metadata_frame_t &self, const std::string &data) {
            static std::unordered_map<NDIlib_metadata_frame_t *, std::string>
                strs;
            strs[&self] = py::str(data);
            self.p_data = &strs[&self][0];
          });

  py::class_<NDIlib_tally_t>(m, "Tally")
      .def(py::init<bool, bool>(), py::arg("on_program") = false,
           py::arg("on_preview") = false)
      .def_readwrite("on_program", &NDIlib_tally_t::on_program)
      .def_readwrite("on_preview", &NDIlib_tally_t::on_preview);

  // Processing.NDI.Lib
  m.def("initialize", &NDIlib_initialize);

  m.def("destroy", &NDIlib_destroy);

  m.def("version", &NDIlib_version);

  m.def("is_supported_CPU", &NDIlib_is_supported_CPU);

  // Processing.NDI.Find
  py::class_<NDIlib_find_create_t>(m, "FindCreate")
      .def(py::init<bool, const char *, const char *>(),
           py::arg("show_local_sources") = true, py::arg("p_groups") = nullptr,
           py::arg("p_extra_ips") = nullptr)
      .def_readwrite("show_local_sources",
                     &NDIlib_find_create_t::show_local_sources)
      .def_property(
          "groups",
          [](const NDIlib_find_create_t &self) {
            if (!self.p_groups)
              return py::str();
            auto ustr = PyUnicode_DecodeLocale(self.p_groups, nullptr);
            return py::reinterpret_steal<py::str>(ustr);
          },
          [](NDIlib_find_create_t &self, const std::string &groups) {
            static std::unordered_map<NDIlib_find_create_t *, std::string> strs;
            strs[&self] = py::str(groups);
            self.p_groups = strs[&self].c_str();
          })
      .def_property(
          "extra_ips",
          [](const NDIlib_find_create_t &self) {
            if (!self.p_extra_ips)
              return py::str();
            auto ustr = PyUnicode_DecodeLocale(self.p_extra_ips, nullptr);
            return py::reinterpret_steal<py::str>(ustr);
          },
          [](NDIlib_find_create_t &self, const std::string &extra_ips) {
            static std::unordered_map<NDIlib_find_create_t *, std::string> strs;
            strs[&self] = py::str(extra_ips);
            self.p_extra_ips = strs[&self].c_str();
          });

  m.def(
      "find_create_v2",
      [](const NDIlib_find_create_t *p_create_settings) {
        auto p_instance = NDIlib_find_create_v2(p_create_settings);
        return py::capsule(p_instance, "FindInstance");
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
        for (uint32_t i = 0; i < count; ++i)
          out.append(sources + i);
        return out;
      },
      py::arg("instance"));

  m.def(
      "find_wait_for_sources",
      [](py::capsule instance, uint32_t timeout_in_ms) {
        auto p_instance =
            static_cast<NDIlib_find_instance_type *>(instance.get_pointer());
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

  py::class_<NDIlib_recv_create_v3_t>(m, "RecvCreateV3")
      .def(py::init<const NDIlib_source_t, NDIlib_recv_color_format_e,
                    NDIlib_recv_bandwidth_e, bool, const char *>(),
           py::arg("source_to_connect_to") = NDIlib_source_t(),
           py::arg("color_format") = NDIlib_recv_color_format_UYVY_BGRA,
           py::arg("bandwidth") = NDIlib_recv_bandwidth_highest,
           py::arg("allow_video_fields") = true,
           py::arg("p_ndi_recv_name") = nullptr)
      .def_readwrite("source_to_connect_to",
                     &NDIlib_recv_create_v3_t::source_to_connect_to)
      .def_readwrite("color_format", &NDIlib_recv_create_v3_t::color_format)
      .def_readwrite("bandwidth", &NDIlib_recv_create_v3_t::bandwidth)
      .def_readwrite("allow_video_fields",
                     &NDIlib_recv_create_v3_t::allow_video_fields)
      .def_property(
          "ndi_recv_name",
          [](const NDIlib_recv_create_v3_t &self) {
            if (!self.p_ndi_recv_name)
              return py::str();
            auto ustr = PyUnicode_DecodeLocale(self.p_ndi_recv_name, nullptr);
            return py::reinterpret_steal<py::str>(ustr);
          },
          [](NDIlib_recv_create_v3_t &self, const std::string &ndi_recv_name) {
            static std::unordered_map<NDIlib_recv_create_v3_t *, std::string>
                strs;
            strs[&self] = py::str(ndi_recv_name);
            self.p_ndi_recv_name = strs[&self].c_str();
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
      [](const NDIlib_recv_create_v3_t *p_create_settings) {
        auto p_instance = NDIlib_recv_create_v3(p_create_settings);
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
      [](py::capsule instance, const NDIlib_source_t *p_src) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        NDIlib_recv_connect(p_instance, p_src);
      },
      py::arg("instance"), py::arg("source") = nullptr);

  m.def(
      "recv_capture_v2",
      [](py::capsule instance, uint32_t timeout_in_ms) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        NDIlib_video_frame_v2_t video_frame;
        NDIlib_audio_frame_v2_t audio_frame;
        NDIlib_metadata_frame_t metadata_frame;
        auto type =
            NDIlib_recv_capture_v2(p_instance, &video_frame, &audio_frame,
                                   &metadata_frame, timeout_in_ms);
        return std::tuple<NDIlib_frame_type_e, NDIlib_video_frame_v2_t,
                          NDIlib_audio_frame_v2_t, NDIlib_metadata_frame_t>(
            type, video_frame, audio_frame, metadata_frame);
      },
      py::arg("instance"), py::arg("timeout_in_ms"));

  m.def(
      "recv_capture_v3",
      [](py::capsule instance, uint32_t timeout_in_ms) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        NDIlib_video_frame_v2_t video_frame;
        NDIlib_audio_frame_v3_t audio_frame;
        NDIlib_metadata_frame_t metadata_frame;
        auto type =
            NDIlib_recv_capture_v3(p_instance, &video_frame, &audio_frame,
                                   &metadata_frame, timeout_in_ms);
        return std::tuple<NDIlib_frame_type_e, NDIlib_video_frame_v2_t,
                          NDIlib_audio_frame_v3_t, NDIlib_metadata_frame_t>(
            type, video_frame, audio_frame, metadata_frame);
      },
      py::arg("instance"), py::arg("timeout_in_ms"));

  m.def(
      "recv_free_video_v2",
      [](py::capsule instance, const NDIlib_video_frame_v2_t *p_video_data) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        NDIlib_recv_free_video_v2(p_instance, p_video_data);
      },
      py::arg("instance"), py::arg("video_data") = nullptr);

  m.def(
      "recv_free_audio_v2",
      [](py::capsule instance, const NDIlib_audio_frame_v2_t *p_audio_data) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        NDIlib_recv_free_audio_v2(p_instance, p_audio_data);
      },
      py::arg("instance"), py::arg("audio_data") = nullptr);

  m.def(
      "recv_free_audio_v3",
      [](py::capsule instance, const NDIlib_audio_frame_v3_t *p_audio_data) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        NDIlib_recv_free_audio_v3(p_instance, p_audio_data);
      },
      py::arg("instance"), py::arg("audio_data") = nullptr);

  m.def(
      "recv_free_metadata",
      [](py::capsule instance, const NDIlib_metadata_frame_t *p_metadata) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        NDIlib_recv_free_metadata(p_instance, p_metadata);
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
      [](py::capsule instance, const NDIlib_metadata_frame_t *p_metadata) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        return NDIlib_recv_send_metadata(p_instance, p_metadata);
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
      [](py::capsule instance, const NDIlib_metadata_frame_t *p_metadata) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        NDIlib_recv_add_connection_metadata(p_instance, p_metadata);
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
        auto ustr = PyUnicode_DecodeLocale(str, nullptr);
        return py::reinterpret_steal<py::str>(ustr);
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
        auto ustr = PyUnicode_DecodeLocale(str, nullptr);
        return py::reinterpret_steal<py::str>(ustr);
      },
      py::arg("instance"));

  m.def(
      "recv_recording_get_error",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        auto str = NDIlib_recv_recording_get_error(p_instance);
        auto ustr = PyUnicode_DecodeLocale(str, nullptr);
        return py::reinterpret_steal<py::str>(ustr);
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
  py::class_<NDIlib_send_create_t>(m, "SendCreate")
      .def(py::init<const char *, const char *, bool, bool>(),
           py::arg("p_ndi_name") = nullptr, py::arg("p_groups") = nullptr,
           py::arg("clock_video") = true, py::arg("clock_audio") = true)
      .def_property(
          "ndi_name",
          [](const NDIlib_send_create_t &self) {
            if (!self.p_ndi_name)
              return py::str();
            auto ustr = PyUnicode_DecodeLocale(self.p_ndi_name, nullptr);
            return py::reinterpret_steal<py::str>(ustr);
          },
          [](NDIlib_send_create_t &self, const char *name) {
            static std::unordered_map<NDIlib_send_create_t *, std::string> strs;
            strs[&self] = py::str(name);
            self.p_ndi_name = strs[&self].c_str();
          })
      .def_property(
          "groups",
          [](const NDIlib_send_create_t &self) {
            if (!self.p_groups)
              return py::str();
            auto ustr = PyUnicode_DecodeLocale(self.p_groups, nullptr);
            return py::reinterpret_steal<py::str>(ustr);
          },
          [](NDIlib_send_create_t &self, const std::string &groups) {
            static std::unordered_map<NDIlib_send_create_t *, std::string> strs;
            strs[&self] = py::str(groups);
            self.p_groups = strs[&self].c_str();
          })
      .def_readwrite("clock_video", &NDIlib_send_create_t::clock_video)
      .def_readwrite("clock_audio", &NDIlib_send_create_t::clock_audio);

  m.def(
      "send_create",
      [](const NDIlib_send_create_t *p_create_settings) {
        auto p_instance = NDIlib_send_create(p_create_settings);
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
      [](py::capsule instance, const NDIlib_video_frame_v2_t *p_video_data) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        NDIlib_send_send_video_v2(p_instance, p_video_data);
      },
      py::arg("instance"), py::arg("video_data"));

  m.def(
      "send_send_video_async_v2",
      [](py::capsule instance, const NDIlib_video_frame_v2_t *p_video_data) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        NDIlib_send_send_video_async_v2(p_instance, p_video_data);
      },
      py::arg("instance"), py::arg("video_data"));

  m.def(
      "send_send_audio_v2",
      [](py::capsule instance, const NDIlib_audio_frame_v2_t *p_audio_data) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        NDIlib_send_send_audio_v2(p_instance, p_audio_data);
      },
      py::arg("instance"), py::arg("audio_data"));

  m.def(
      "send_send_audio_v3",
      [](py::capsule instance, const NDIlib_audio_frame_v3_t *p_audio_data) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        NDIlib_send_send_audio_v3(p_instance, p_audio_data);
      },
      py::arg("instance"), py::arg("audio_data"));

  m.def(
      "send_send_metadata",
      [](py::capsule instance, const NDIlib_metadata_frame_t *p_metadata) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        NDIlib_send_send_metadata(p_instance, p_metadata);
      },
      py::arg("instance"), py::arg("metadata"));

  m.def(
      "send_capture",
      [](py::capsule instance, NDIlib_metadata_frame_t *p_metadata,
         uint32_t timeout_in_ms) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        NDIlib_send_capture(p_instance, p_metadata, timeout_in_ms);
      },
      py::arg("instance"), py::arg("metadata"), py::arg("timeout_in_ms"));

  m.def(
      "send_free_metadata",
      [](py::capsule instance, const NDIlib_metadata_frame_t *p_metadata) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        NDIlib_send_free_metadata(p_instance, p_metadata);
      },
      py::arg("instance"), py::arg("metadata"));

  m.def(
      "send_get_tally",
      [](py::capsule instance, NDIlib_tally_t *p_tally,
         uint32_t timeout_in_ms) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        return NDIlib_send_get_tally(p_instance, p_tally, timeout_in_ms);
      },
      py::arg("instance"), py::arg("tally"), py::arg("timeout_in_ms"));

  m.def(
      "send_get_no_connections",
      [](py::capsule instance, uint32_t timeout_in_ms) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
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
      [](py::capsule instance, const NDIlib_metadata_frame_t *p_metadata) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        NDIlib_send_add_connection_metadata(p_instance, p_metadata);
      },
      py::arg("instance"), py::arg("metadata"));

  m.def(
      "send_set_failover",
      [](py::capsule instance, const NDIlib_source_t *p_failover_source) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        NDIlib_send_set_failover(p_instance, p_failover_source);
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
  py::class_<NDIlib_routing_create_t>(m, "RoutingCreate")
      .def(py::init<const char *, const char *>(),
           py::arg("p_ndi_name") = nullptr, py::arg("p_groups") = nullptr)
      .def_property(
          "ndi_name",
          [](const NDIlib_routing_create_t &self) {
            if (!self.p_ndi_name)
              return py::str();
            auto ustr = PyUnicode_DecodeLocale(self.p_ndi_name, nullptr);
            return py::reinterpret_steal<py::str>(ustr);
          },
          [](NDIlib_routing_create_t &self, const char *name) {
            static std::unordered_map<NDIlib_routing_create_t *, std::string>
                strs;
            strs[&self] = py::str(name);
            self.p_ndi_name = strs[&self].c_str();
          })
      .def_property(
          "groups",
          [](const NDIlib_routing_create_t &self) {
            if (!self.p_groups)
              return py::str();
            auto ustr = PyUnicode_DecodeLocale(self.p_groups, nullptr);
            return py::reinterpret_steal<py::str>(ustr);
          },
          [](NDIlib_routing_create_t &self, const std::string &groups) {
            static std::unordered_map<NDIlib_routing_create_t *, std::string>
                strs;
            strs[&self] = py::str(groups);
            self.p_groups = strs[&self].c_str();
          });

  m.def(
      "routing_create",
      [](const NDIlib_routing_create_t *p_create_settings) {
        auto p_instance = NDIlib_routing_create(p_create_settings);
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
      [](py::capsule instance, const NDIlib_source_t *p_source) {
        auto p_instance =
            static_cast<NDIlib_routing_instance_type *>(instance.get_pointer());
        return NDIlib_routing_change(p_instance, p_source);
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
        return NDIlib_routing_get_no_connections(p_instance, timeout_in_ms);
      },
      py::arg("instance"), py::arg("timeout_in_ms"));

  m.def(
      "routing_get_source_name(",
      [](py::capsule instance) {
        auto p_instance =
            static_cast<NDIlib_routing_instance_type *>(instance.get_pointer());
        return NDIlib_routing_get_source_name(p_instance);
      },
      py::arg("instance"));

  // Processing.NDI.utilities
  py::class_<NDIlib_audio_frame_interleaved_16s_t>(m,
                                                   "AudioFrameInterleaved16s")
      .def(py::init<int, int, int, int64_t, int, int16_t *>(),
           py::arg("sample_rate") = 48000, py::arg("no_channels") = 2,
           py::arg("no_samples") = 0,
           py::arg("timecode") = NDIlib_send_timecode_synthesize,
           py::arg("reference_level") = 0, py::arg("p_data") = 0)
      .def_readwrite("sample_rate",
                     &NDIlib_audio_frame_interleaved_16s_t::sample_rate)
      .def_readwrite("no_channels",
                     &NDIlib_audio_frame_interleaved_16s_t::no_channels)
      .def_readwrite("no_samples",
                     &NDIlib_audio_frame_interleaved_16s_t::no_samples)
      .def_readwrite("timecode",
                     &NDIlib_audio_frame_interleaved_16s_t::timecode)
      .def_readwrite("reference_level",
                     &NDIlib_audio_frame_interleaved_16s_t::reference_level)
      .def_property(
          "data",
          [](const NDIlib_audio_frame_interleaved_16s_t &self) {
            size_t col = self.no_samples;
            size_t row = self.no_channels;
            size_t size = sizeof(int16_t);
            auto buffer_info = py::buffer_info(
                self.p_data, size, py::format_descriptor<int16_t>::format(), 2,
                {row, col}, {col * size, size});
            return py::array(buffer_info);
          },
          [](NDIlib_audio_frame_interleaved_16s_t &self,
             py::array_t<int16_t> &array) {
            auto info = array.request();
            self.p_data = static_cast<int16_t *>(info.ptr);
            self.no_channels = info.shape[0];
            self.no_samples = info.shape[1];
          });

  py::class_<NDIlib_audio_frame_interleaved_32s_t>(m,
                                                   "AudioFrameInterleaved32s")
      .def(py::init<int, int, int, int64_t, int, int32_t *>(),
           py::arg("sample_rate") = 48000, py::arg("no_channels") = 2,
           py::arg("no_samples") = 0,
           py::arg("timecode") = NDIlib_send_timecode_synthesize,
           py::arg("reference_level") = 0, py::arg("p_data") = 0)
      .def_readwrite("sample_rate",
                     &NDIlib_audio_frame_interleaved_32s_t::sample_rate)
      .def_readwrite("no_channels",
                     &NDIlib_audio_frame_interleaved_32s_t::no_channels)
      .def_readwrite("no_samples",
                     &NDIlib_audio_frame_interleaved_32s_t::no_samples)
      .def_readwrite("timecode",
                     &NDIlib_audio_frame_interleaved_32s_t::timecode)
      .def_readwrite("reference_level",
                     &NDIlib_audio_frame_interleaved_32s_t::reference_level)
      .def_property(
          "data",
          [](const NDIlib_audio_frame_interleaved_32s_t &self) {
            size_t col = self.no_samples;
            size_t row = self.no_channels;
            size_t size = sizeof(int32_t);
            auto buffer_info = py::buffer_info(
                self.p_data, size, py::format_descriptor<int32_t>::format(), 2,
                {row, col}, {col * size, size});
            return py::array(buffer_info);
          },
          [](NDIlib_audio_frame_interleaved_32s_t &self,
             py::array_t<int32_t> &array) {
            auto info = array.request();
            self.p_data = static_cast<int32_t *>(info.ptr);
            self.no_channels = info.shape[0];
            self.no_samples = info.shape[1];
          });

  py::class_<NDIlib_audio_frame_interleaved_32f_t>(m,
                                                   "AudioFrameInterleaved32f")
      .def(py::init<int, int, int, int64_t, float *>(),
           py::arg("sample_rate") = 48000, py::arg("no_channels") = 2,
           py::arg("no_samples") = 0,
           py::arg("timecode") = NDIlib_send_timecode_synthesize,
           py::arg("p_data") = 0)
      .def_readwrite("sample_rate",
                     &NDIlib_audio_frame_interleaved_32f_t::sample_rate)
      .def_readwrite("no_channels",
                     &NDIlib_audio_frame_interleaved_32f_t::no_channels)
      .def_readwrite("no_samples",
                     &NDIlib_audio_frame_interleaved_32f_t::no_samples)
      .def_readwrite("timecode",
                     &NDIlib_audio_frame_interleaved_32f_t::timecode)
      .def_property(
          "data",
          [](const NDIlib_audio_frame_interleaved_32f_t &self) {
            size_t col = self.no_samples;
            size_t row = self.no_channels;
            size_t size = sizeof(float);
            auto buffer_info = py::buffer_info(
                self.p_data, size, py::format_descriptor<float>::format(), 2,
                {row, col}, {col * size, size});
            return py::array(buffer_info);
          },
          [](NDIlib_audio_frame_interleaved_32f_t &self,
             py::array_t<float> &array) {
            auto info = array.request();
            self.p_data = static_cast<float *>(info.ptr);
            self.no_channels = info.shape[0];
            self.no_samples = info.shape[1];
          });

  m.def(
      "util_send_send_audio_interleaved_16s",
      [](py::capsule instance,
         const NDIlib_audio_frame_interleaved_16s_t *p_audio_data) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        NDIlib_util_send_send_audio_interleaved_16s(p_instance, p_audio_data);
      },
      py::arg("instance"), py::arg("audio_data"));

  m.def(
      "util_send_send_audio_interleaved_32s",
      [](py::capsule instance,
         const NDIlib_audio_frame_interleaved_32s_t *p_audio_data) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        NDIlib_util_send_send_audio_interleaved_32s(p_instance, p_audio_data);
      },
      py::arg("instance"), py::arg("audio_data"));

  m.def(
      "util_send_send_audio_interleaved_32f",
      [](py::capsule instance,
         const NDIlib_audio_frame_interleaved_32f_t *p_audio_data) {
        auto p_instance =
            static_cast<NDIlib_send_instance_type *>(instance.get_pointer());
        NDIlib_util_send_send_audio_interleaved_32f(p_instance, p_audio_data);
      },
      py::arg("instance"), py::arg("audio_data"));

  m.def("util_audio_to_interleaved_16s_v2",
        &NDIlib_util_audio_to_interleaved_16s_v2, py::arg("src"),
        py::arg("dst"));

  m.def("util_audio_from_interleaved_16s_v2",
        &NDIlib_util_audio_from_interleaved_16s_v2, py::arg("src"),
        py::arg("dst"));

  m.def("util_audio_to_interleaved_32s_v2",
        &NDIlib_util_audio_to_interleaved_32s_v2, py::arg("src"),
        py::arg("dst"));

  m.def("util_audio_from_interleaved_32s_v2",
        &NDIlib_util_audio_from_interleaved_32s_v2, py::arg("src"),
        py::arg("dst"));

  m.def("util_audio_to_interleaved_32f_v2",
        &NDIlib_util_audio_to_interleaved_32f_v2, py::arg("src"),
        py::arg("dst"));

  m.def("util_audio_from_interleaved_32f_v2",
        &NDIlib_util_audio_from_interleaved_32f_v2, py::arg("src"),
        py::arg("dst"));

  m.def("util_V210_to_P216", &NDIlib_util_V210_to_P216, py::arg("src_v210"),
        py::arg("dst_p216"));

  m.def("util_P216_to_V210", &NDIlib_util_P216_to_V210, py::arg("src_p216"),
        py::arg("dst_v210"));

  // Processing.NDI.deprecated
  // TODO

  // Processing.NDI.FrameSync
  m.def(
      "framesync_create",
      [](py::capsule instance) {
        auto p_recv_instance =
            static_cast<NDIlib_recv_instance_type *>(instance.get_pointer());
        auto p_instance = NDIlib_framesync_create(p_recv_instance);
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
        NDIlib_audio_frame_v2_t audio_frame;
        NDIlib_framesync_capture_audio(p_instance, &audio_frame, sample_rate,
                                       no_channels, no_samples);
        return audio_frame;
      },
      py::arg("instance"), py::arg("sample_rate"), py::arg("no_channels"),
      py::arg("no_samples"));

  m.def(
      "framesync_capture_audio_v2",
      [](py::capsule instance, int sample_rate, int no_channels,
         int no_samples) {
        auto p_instance = static_cast<NDIlib_framesync_instance_type *>(
            instance.get_pointer());
        NDIlib_audio_frame_v3_t audio_frame;
        NDIlib_framesync_capture_audio_v2(p_instance, &audio_frame, sample_rate,
                                          no_channels, no_samples);
        return audio_frame;
      },
      py::arg("instance"), py::arg("sample_rate"), py::arg("no_channels"),
      py::arg("no_samples"));

  m.def(
      "framesync_free_audio",
      [](py::capsule instance, NDIlib_audio_frame_v2_t *p_audio_data) {
        auto p_instance = static_cast<NDIlib_framesync_instance_type *>(
            instance.get_pointer());
        NDIlib_framesync_free_audio(p_instance, p_audio_data);
      },
      py::arg("instance"), py::arg("audio_data"));

  m.def(
      "framesync_free_audio_v2",
      [](py::capsule instance, NDIlib_audio_frame_v3_t *p_audio_data) {
        auto p_instance = static_cast<NDIlib_framesync_instance_type *>(
            instance.get_pointer());
        NDIlib_framesync_free_audio_v2(p_instance, p_audio_data);
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
        NDIlib_video_frame_v2_t video_frame;
        NDIlib_framesync_capture_video(p_instance, &video_frame, field_type);
        return video_frame;
      },
      py::arg("instance"),
      py::arg("field_type") = NDIlib_frame_format_type_progressive);

  m.def(
      "framesync_free_video",
      [](py::capsule instance, NDIlib_video_frame_v2_t *p_video_data) {
        auto p_instance = static_cast<NDIlib_framesync_instance_type *>(
            instance.get_pointer());
        NDIlib_framesync_free_video(p_instance, p_video_data);
      },
      py::arg("instance"), py::arg("video_data"));
}
