#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include "PyAudioFrame.hpp"
#include "PyMetadataFrame.hpp"
#include "PySource.hpp"
#include "PyVideoFrame.hpp"

namespace py = pybind11;

PYBIND11_MODULE(NDIlib, m) {

  m.doc() = "NDI Plugin";

  // Processing.NDI.structs
  enum FrameType {
    FRAME_TYPE_NONE = NDIlib_frame_type_none,
    FRAME_TYPE_VIDEO = NDIlib_frame_type_video,
    FRAME_TYPE_AUDIO = NDIlib_frame_type_audio,
    FRAME_TYPE_METADATA = NDIlib_frame_type_metadata,
    FRAME_TYPE_ERROR = NDIlib_frame_type_error,
    FRANE_TYPE_STATUS_CHANGE = NDIlib_frame_type_status_change,
    FRAME_TYPE_MAX = NDIlib_frame_type_max
  };

  py::enum_<FrameType>(m, "FrameType", py::arithmetic())
      .value("FRAME_TYPE_NONE", FrameType::FRAME_TYPE_NONE)
      .value("FRAME_TYPE_VIDEO", FrameType::FRAME_TYPE_VIDEO)
      .value("FRAME_TYPE_AUDIO", FrameType::FRAME_TYPE_AUDIO)
      .value("FRAME_TYPE_METADATA", FrameType::FRAME_TYPE_METADATA)
      .value("FRAME_TYPE_ERROR", FrameType::FRAME_TYPE_ERROR)
      .value("FRANE_TYPE_STATUS_CHANGE", FrameType::FRANE_TYPE_STATUS_CHANGE)
      .value("FRAME_TYPE_MAX", FrameType::FRAME_TYPE_MAX)
      .export_values();

  enum FourCCVideoType {
    FOURCC_VIDEO_TYPE_UYVY = NDIlib_FourCC_video_type_UYVY,
    FOURCC_VIDEO_TYPE_UYVA = NDIlib_FourCC_video_type_UYVA,
    FOURCC_VIDEO_TYPE_P216 = NDIlib_FourCC_video_type_P216,
    FOURCC_VIDEO_TYPE_PA16 = NDIlib_FourCC_video_type_PA16,
    FOURCC_VIDEO_TYPE_YV12 = NDIlib_FourCC_video_type_YV12,
    FOURCC_VIDEO_TYPE_I420 = NDIlib_FourCC_video_type_I420,
    FOURCC_VIDEO_TYPE_NV12 = NDIlib_FourCC_video_type_NV12,
    FOURCC_VIDEO_TYPE_BGRA = NDIlib_FourCC_video_type_BGRA,
    FOURCC_VIDEO_TYPE_BGRX = NDIlib_FourCC_video_type_BGRX,
    FOURCC_VIDEO_TYPE_RGBA = NDIlib_FourCC_video_type_RGBA,
    FOURCC_VIDEO_TYPE_RGBX = NDIlib_FourCC_video_type_RGBX,
    FOURCC_VIDEO_TYPE_MAX = NDIlib_FourCC_video_type_max,
  };

  py::enum_<FourCCVideoType>(m, "FourCCVideoType", py::arithmetic())
      .value("FOURCC_VIDEO_TYPE_UYVY", FourCCVideoType::FOURCC_VIDEO_TYPE_UYVY)
      .value("FOURCC_VIDEO_TYPE_UYVA", FourCCVideoType::FOURCC_VIDEO_TYPE_UYVA)
      .value("FOURCC_VIDEO_TYPE_P216", FourCCVideoType::FOURCC_VIDEO_TYPE_P216)
      .value("FOURCC_VIDEO_TYPE_PA16", FourCCVideoType::FOURCC_VIDEO_TYPE_PA16)
      .value("FOURCC_VIDEO_TYPE_YV12", FourCCVideoType::FOURCC_VIDEO_TYPE_YV12)
      .value("FOURCC_VIDEO_TYPE_I420", FourCCVideoType::FOURCC_VIDEO_TYPE_I420)
      .value("FOURCC_VIDEO_TYPE_NV12", FourCCVideoType::FOURCC_VIDEO_TYPE_NV12)
      .value("FOURCC_VIDEO_TYPE_BGRA", FourCCVideoType::FOURCC_VIDEO_TYPE_BGRA)
      .value("FOURCC_VIDEO_TYPE_BGRX", FourCCVideoType::FOURCC_VIDEO_TYPE_BGRX)
      .value("FOURCC_VIDEO_TYPE_RGBA", FourCCVideoType::FOURCC_VIDEO_TYPE_RGBA)
      .value("FOURCC_VIDEO_TYPE_RGBX", FourCCVideoType::FOURCC_VIDEO_TYPE_RGBX)
      .value("FOURCC_VIDEO_TYPE_MAX", FourCCVideoType::FOURCC_VIDEO_TYPE_MAX)
      .export_values();

  enum FourCCAudioType {
    FOURCC_AUDIO_TYPE_FLTP = NDIlib_FourCC_audio_type_FLTP,
    FOURCC_AUDIO_TYPE_MAX = NDIlib_FourCC_audio_type_max,
  };

  py::enum_<FourCCAudioType>(m, "FourCCAudioType", py::arithmetic())
      .value("FOURCC_AUDIO_TYPE_FLTP", FourCCAudioType::FOURCC_AUDIO_TYPE_FLTP)
      .value("FOURCC_AUDIO_TYPE_MAX", FourCCAudioType::FOURCC_AUDIO_TYPE_MAX)
      .export_values();

  enum FrameFormatType {
    FRAME_FORMAT_TYPE_PROGRESSIVE = NDIlib_frame_format_type_progressive,
    FRAME_FORMAT_TYPE_INTERLEAVED = NDIlib_frame_format_type_interleaved,
    FRAME_FORMAT_TYPE_FIELD_0 = NDIlib_frame_format_type_field_0,
    FRAME_FORMAT_TYPE_FIELD_1 = NDIlib_frame_format_type_field_1,
    FRAME_FORMAT_TYPE_MAX = NDIlib_frame_format_type_max
  };

  py::enum_<FrameFormatType>(m, "FrameFormatType", py::arithmetic())
      .value("FRAME_FORMAT_TYPE_PROGRESSIVE",
             FrameFormatType::FRAME_FORMAT_TYPE_PROGRESSIVE)
      .value("FRAME_FORMAT_TYPE_INTERLEAVED",
             FrameFormatType::FRAME_FORMAT_TYPE_INTERLEAVED)
      .value("FRAME_FORMAT_TYPE_FIELD_0",
             FrameFormatType::FRAME_FORMAT_TYPE_FIELD_0)
      .value("FRAME_FORMAT_TYPE_FIELD_1",
             FrameFormatType::FRAME_FORMAT_TYPE_FIELD_1)
      .value("FRAME_FORMAT_TYPE_MAX", FrameFormatType::FRAME_FORMAT_TYPE_MAX)
      .export_values();

  m.attr("SEND_TIMECODE_SYNTHESIZE") = py::int_(INT64_MAX);

  m.attr("RECV_TIMESTAMP_UNDEFINED") = py::int_(INT64_MAX);

  py::class_<PySource>(m, "Source")
      .def(py::init<const std::string &, const std::string &>(),
           py::arg("ndi_name") = nullptr, py::arg("url_address") = nullptr)
      .def_property("ndi_name", &PySource::getNdiName, &PySource::setNdiName)
      .def_property("url_address", &PySource::getUrlAddress,
                    &PySource::setUrlAddress);

  py::class_<PyVideoFrameV2>(m, "VideoFrameV2")
      .def(py::init<int, int, NDIlib_FourCC_type_e, int, int, float,
                    NDIlib_frame_format_type_e, int64_t, uint8_t *, int,
                    const std::string &, int64_t>(),
           py::arg("xres") = 0, py::arg("yres") = 0,
           py::arg("FourCC") = FOURCC_VIDEO_TYPE_UYVY,
           py::arg("frame_rate_N") = 30000, py::arg("frame_rate_D") = 1001,
           py::arg("picture_aspect_ratio") = 0.0f,
           py::arg("frame_format_type") = FRAME_FORMAT_TYPE_PROGRESSIVE,
           py::arg("timecode") = 0, py::arg("data") = nullptr,
           py::arg("line_stride_in_bytes") = 0, py::arg("metadata") = nullptr,
           py::arg("timestamp") = 0)
      .def_readwrite("xres", &PyVideoFrameV2::xres)
      .def_readwrite("yres", &PyVideoFrameV2::yres)
      .def_readwrite("FourCC", &PyVideoFrameV2::FourCC)
      .def_readwrite("frame_rate_N", &PyVideoFrameV2::frame_rate_N)
      .def_readwrite("frame_rate_D", &PyVideoFrameV2::frame_rate_D)
      .def_readwrite("picture_aspect_ratio",
                     &PyVideoFrameV2::picture_aspect_ratio)
      .def_readwrite("frame_format_type", &PyVideoFrameV2::frame_format_type)
      .def_readwrite("timecode", &PyVideoFrameV2::timecode)
      .def_property("data", nullptr, &PyVideoFrameV2::setData)
      .def_readwrite("line_stride_in_bytes",
                     &PyVideoFrameV2::line_stride_in_bytes)
      .def_property("metadata", &PyVideoFrameV2::getMetadata,
                    &PyVideoFrameV2::setMetadata)
      .def_readwrite("timestamp", &PyVideoFrameV2::timestamp);

  py::class_<PyAudioFrameV2>(m, "AudioFrameV2")
      .def(py::init<int, int, int, int64_t, float *, int, const std::string &,
                    int64_t>(),
           py::arg("sample_rate") = 48000, py::arg("no_channels") = 2,
           py::arg("no_samples") = 0,
           py::arg("timecode") = NDIlib_send_timecode_synthesize,
           py::arg("data") = nullptr, py::arg("channel_stride_in_bytes") = 0,
           py::arg("metadata") = nullptr, py::arg("timestamp") = 0)
      .def_readwrite("sample_rate", &PyAudioFrameV2::sample_rate)
      .def_readwrite("no_channels", &PyAudioFrameV2::no_channels)
      .def_readwrite("no_samples", &PyAudioFrameV2::no_samples)
      .def_readwrite("timecode", &PyAudioFrameV2::timecode)
      .def_property("data", nullptr, &PyAudioFrameV2::setData)
      .def_readwrite("channel_stride_in_bytes",
                     &NDIlib_audio_frame_v2_t::channel_stride_in_bytes)
      .def_property("metadata", &PyAudioFrameV2::getMetadata,
                    &PyAudioFrameV2::setMetadata)
      .def_readwrite("timestamp", &PyAudioFrameV2::timestamp);

  py::class_<PyMetadataFrame>(m, "MetadataFrame")
      .def(py::init<int, int64_t, const std::string &>(), py::arg("length") = 0,
           py::arg("timecode") = NDIlib_send_timecode_synthesize,
           py::arg("data") = nullptr)
      .def_readwrite("length", &PyMetadataFrame::length)
      .def_readwrite("timecode", &PyMetadataFrame::timecode)
      .def_property("data", &PyMetadataFrame::getData,
                    &PyMetadataFrame::setData);

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
           py::arg("show_local_sources") = true, py::arg("groups") = nullptr,
           py::arg("extra_ips") = nullptr)
      .def_readwrite("show_local_sources",
                     &NDIlib_find_create_t::show_local_sources)
      .def_readwrite("groups", &NDIlib_find_create_t::p_groups)
      .def_readwrite("extra_ips", &NDIlib_find_create_t::p_extra_ips);

  m.def("find_create_v2", &NDIlib_find_create_v2,
        py::arg("create_settings") = nullptr);

  m.def("find_destroy", &NDIlib_find_destroy, py::arg("instance"));

  m.def("find_get_current_sources", &NDIlib_find_get_current_sources,
        py::arg("instance"), py::arg("no_sources"));

  m.def("find_wait_for_sources", &NDIlib_find_wait_for_sources,
        py::arg("instance"), py::arg("timeout_in_ms"));

  // Processing.NDI.Recv
  enum RecvBandwidth {
    RECV_BANDWIDTH_METADATA_ONLY = NDIlib_recv_bandwidth_metadata_only,
    RECV_BANDWIDTH_AUDIO_ONLY = NDIlib_recv_bandwidth_audio_only,
    RECV_BANDWIDTH_LOWEST = NDIlib_recv_bandwidth_lowest,
    RECV_BANDWIDTH_HIGHEST = NDIlib_recv_bandwidth_highest
  };

  py::enum_<RecvBandwidth>(m, "RecvBandwidth", py::arithmetic())
      .value("RECV_BANDWIDTH_METADATA_ONLY",
             RecvBandwidth::RECV_BANDWIDTH_METADATA_ONLY)
      .value("RECV_BANDWIDTH_AUDIO_ONLY",
             RecvBandwidth::RECV_BANDWIDTH_AUDIO_ONLY)
      .value("RECV_BANDWIDTH_LOWEST", RecvBandwidth::RECV_BANDWIDTH_LOWEST)
      .value("RECV_BANDWIDTH_HIGHEST", RecvBandwidth::RECV_BANDWIDTH_HIGHEST)
      .export_values();

  enum RecvColorFormat {
    RECV_COLOR_FORMAT_BGRX_BGRA = NDIlib_recv_color_format_BGRX_BGRA,
    RECV_COLOR_FORMAT_UYVY_BGRA = NDIlib_recv_color_format_UYVY_BGRA,
    RECV_COLOR_FORMAT_RGBX_RGBA = NDIlib_recv_color_format_RGBX_RGBA,
    RECV_COLOR_FORMAT_UYVY_RGBA = NDIlib_recv_color_format_UYVY_RGBA,
#ifdef _WIN32
    RECV_COLOR_FORMAT_BGRX_BGRA_FLIPPED =
        NDIlib_recv_color_format_BGRX_BGRA_flipped,
#endif
    RECV_COLOR_FORMAT_FASTEST = NDIlib_recv_color_format_fastest,
    RECV_COLOR_FORMAT_E_BGRX_BGRA = NDIlib_recv_color_format_e_BGRX_BGRA,
    RECV_COLOR_FORMAT_E_UYVY_BGRA = NDIlib_recv_color_format_e_UYVY_BGRA,
    RECV_COLOR_FORMAT_E_RGBX_RGBA = NDIlib_recv_color_format_e_RGBX_RGBA,
    RECV_COLOR_FORMAT_E_UYVY_RGBA = NDIlib_recv_color_format_e_UYVY_RGBA
  };

  py::enum_<RecvColorFormat>(m, "RecvColorFormat", py::arithmetic())
      .value("RECV_COLOR_FORMAT_BGRX_BGRA",
             RecvColorFormat::RECV_COLOR_FORMAT_BGRX_BGRA)
      .value("RECV_COLOR_FORMAT_UYVY_BGRA",
             RecvColorFormat::RECV_COLOR_FORMAT_UYVY_BGRA)
      .value("RECV_COLOR_FORMAT_RGBX_RGBA",
             RecvColorFormat::RECV_COLOR_FORMAT_RGBX_RGBA)
      .value("RECV_COLOR_FORMAT_UYVY_RGBA",
             RecvColorFormat::RECV_COLOR_FORMAT_UYVY_RGBA)
#ifdef _WIN32
      .value("RECV_COLOR_FORMAT_BGRX_BGRA_FLIPPED",
             RecvColorFormat::RECV_COLOR_FORMAT_BGRX_BGRA_FLIPPED)
#endif
      .value("RECV_COLOR_FORMAT_FASTEST",
             RecvColorFormat::RECV_COLOR_FORMAT_FASTEST)
      .value("RECV_COLOR_FORMAT_E_BGRX_BGRA",
             RecvColorFormat::RECV_COLOR_FORMAT_E_BGRX_BGRA)
      .value("RECV_COLOR_FORMAT_E_UYVY_BGRA",
             RecvColorFormat::RECV_COLOR_FORMAT_E_UYVY_BGRA)
      .value("RECV_COLOR_FORMAT_E_RGBX_RGBA",
             RecvColorFormat::RECV_COLOR_FORMAT_E_RGBX_RGBA)
      .value("RECV_COLOR_FORMAT_E_UYVY_RGBA",
             RecvColorFormat::RECV_COLOR_FORMAT_E_UYVY_RGBA)
      .export_values();

  py::class_<NDIlib_recv_create_v3_t>(m, "RecvCreateV3")
      .def(py::init<const NDIlib_source_t, NDIlib_recv_color_format_e,
                    NDIlib_recv_bandwidth_e, bool, const char *>(),
           py::arg("source_to_connect_to") = NDIlib_source_t(),
           py::arg("color_format") = RECV_COLOR_FORMAT_UYVY_BGRA,
           py::arg("bandwidth") = RECV_BANDWIDTH_HIGHEST,
           py::arg("allow_video_fields") = true, py::arg("ndi_name") = nullptr)
      .def_readwrite("source_to_connect_to",
                     &NDIlib_recv_create_v3_t::source_to_connect_to)
      .def_readwrite("color_format", &NDIlib_recv_create_v3_t::color_format)
      .def_readwrite("bandwidth", &NDIlib_recv_create_v3_t::bandwidth)
      .def_readwrite("allow_video_fields",
                     &NDIlib_recv_create_v3_t::allow_video_fields)
      .def_readwrite("ndi_recv_name",
                     &NDIlib_recv_create_v3_t::p_ndi_recv_name);

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

  m.def("recv_create_v3", &NDIlib_recv_create_v3,
        py::arg("instance") = nullptr);

  m.def("recv_destroy", &NDIlib_recv_destroy, py::arg("instance"));

  m.def("recv_connect", &NDIlib_recv_connect, py::arg("instance"),
        py::arg("source") = nullptr);

  m.def("recv_capture_v2", &NDIlib_recv_capture_v2, py::arg("instance"),
        py::arg("video_data"), py::arg("audio_data"), py::arg("metadata"),
        py::arg("timeout_in_ms"));

  m.def("recv_free_video_v2", &NDIlib_recv_free_video_v2, py::arg("instance"),
        py::arg("video_data"));

  m.def("recv_free_audio_v2", &NDIlib_recv_free_audio_v2, py::arg("instance"),
        py::arg("audio_data"));

  m.def("recv_free_metadata", &NDIlib_recv_free_metadata, py::arg("instance"),
        py::arg("metadata"));

  m.def("recv_free_string", &NDIlib_recv_free_string, py::arg("instance"),
        py::arg("string"));

  m.def("recv_send_metadata", &NDIlib_recv_send_metadata, py::arg("instance"),
        py::arg("metadata_frame"));

  m.def("recv_set_tally", &NDIlib_recv_set_tally, py::arg("instance"),
        py::arg("tally"));

  m.def("recv_get_performance", &NDIlib_recv_get_performance,
        py::arg("instance"), py::arg("total"), py::arg("dropped"));

  m.def("recv_get_queue", &NDIlib_recv_get_queue, py::arg("instance"),
        py::arg("total"));

  m.def("recv_clear_connection_metadata",
        &NDIlib_recv_clear_connection_metadata, py::arg("instance"));

  m.def("recv_add_connection_metadata", &NDIlib_recv_add_connection_metadata,
        py::arg("instance"), py::arg("metadata"));

  m.def("recv_get_no_connections", &NDIlib_recv_get_no_connections,
        py::arg("instance"));

  m.def("recv_get_web_control", &NDIlib_recv_get_web_control,
        py::arg("instance"));

  // Processing.NDI.Recv.ex
  m.def("recv_ptz_is_supported", &NDIlib_recv_ptz_is_supported,
        py::arg("instance"));

  m.def("recv_recording_is_supported", &NDIlib_recv_recording_is_supported,
        py::arg("instance"));

  m.def("recv_ptz_zoom", &NDIlib_recv_ptz_zoom, py::arg("instance"),
        py::arg("zoom_value"));

  m.def("recv_ptz_zoom_speed", &NDIlib_recv_ptz_zoom_speed, py::arg("instance"),
        py::arg("zoom_speed"));

  m.def("recv_ptz_pan_tilt", &NDIlib_recv_ptz_pan_tilt, py::arg("instance"),
        py::arg("pan_value"), py::arg("tilt_value"));

  m.def("recv_ptz_pan_tilt_speed", &NDIlib_recv_ptz_pan_tilt_speed,
        py::arg("instance"), py::arg("pan_speed"), py::arg("tilt_speed"));

  m.def("recv_ptz_store_preset", &NDIlib_recv_ptz_store_preset,
        py::arg("instance"), py::arg("preset_no"));

  m.def("recv_ptz_recall_preset", &NDIlib_recv_ptz_recall_preset,
        py::arg("instance"), py::arg("preset_no"), py::arg("speed"));

  m.def("recv_ptz_auto_focus", &NDIlib_recv_ptz_auto_focus,
        py::arg("instance"));

  m.def("recv_ptz_focus", &NDIlib_recv_ptz_focus, py::arg("instance"),
        py::arg("focus_value"));

  m.def("recv_ptz_focus_speed", &NDIlib_recv_ptz_focus_speed,
        py::arg("instance"), py::arg("focus_speed"));

  m.def("recv_ptz_white_balance_auto", &NDIlib_recv_ptz_white_balance_auto,
        py::arg("instance"));

  m.def("recv_ptz_white_balance_indoor", &NDIlib_recv_ptz_white_balance_indoor,
        py::arg("instance"));

  m.def("recv_ptz_white_balance_outdoor",
        &NDIlib_recv_ptz_white_balance_outdoor, py::arg("instance"));

  m.def("recv_ptz_white_balance_oneshot",
        &NDIlib_recv_ptz_white_balance_oneshot, py::arg("instance"));

  m.def("recv_ptz_white_balance_manual", &NDIlib_recv_ptz_white_balance_manual,
        py::arg("instance"), py::arg("red"), py::arg("blue"));

  m.def("recv_ptz_exposure_auto", &NDIlib_recv_ptz_exposure_auto,
        py::arg("instance"));

  m.def("recv_ptz_exposure_manual", &NDIlib_recv_ptz_exposure_manual,
        py::arg("instance"), py::arg("exposure_level"));

  m.def("recv_recording_start", &NDIlib_recv_recording_start,
        py::arg("instance"), py::arg("filename_hint"));

  m.def("recv_recording_stop", &NDIlib_recv_recording_stop,
        py::arg("instance"));

  m.def("recv_recording_set_audio_level",
        &NDIlib_recv_recording_set_audio_level, py::arg("instance"),
        py::arg("level_dB"));

  m.def("recv_recording_is_recording", &NDIlib_recv_recording_is_recording,
        py::arg("instance"));

  m.def("recv_recording_get_filename", &NDIlib_recv_recording_get_filename,
        py::arg("instance"));

  m.def("recv_recording_get_error", &NDIlib_recv_recording_get_error,
        py::arg("instance"));

  py::class_<NDIlib_recv_recording_time_t>(m, "RecvRecordingTime")
      .def(py::init<>())
      .def_readwrite("no_frames", &NDIlib_recv_recording_time_t::no_frames)
      .def_readwrite("start_time", &NDIlib_recv_recording_time_t::start_time)
      .def_readwrite("last_time", &NDIlib_recv_recording_time_t::last_time);

  m.def("recv_recording_get_times", &NDIlib_recv_recording_get_times,
        py::arg("instance"), py::arg("times"));

  // Processing.NDI.Send
  py::class_<NDIlib_send_create_t>(m, "SendCreate")
      .def(py::init<const char *, const char *, bool, bool>(),
           py::arg("ndi_name") = nullptr, py::arg("groups") = nullptr,
           py::arg("clock_video") = true, py::arg("clock_audio") = true)
      .def_readwrite("ndi_name", &NDIlib_send_create_t::p_ndi_name)
      .def_readwrite("groups", &NDIlib_send_create_t::p_groups)
      .def_readwrite("clock_video", &NDIlib_send_create_t::clock_video)
      .def_readwrite("clock_audio", &NDIlib_send_create_t::clock_audio);

  m.def("send_create", &NDIlib_send_create,
        py::arg("create_settings") = nullptr);

  m.def("send_destroy", &NDIlib_send_destroy, py::arg("instance"));

  m.def("send_send_video_v2", &NDIlib_send_send_video_v2, py::arg("instance"),
        py::arg("video_data"));

  m.def("send_send_video_async_v2", &NDIlib_send_send_video_async_v2,
        py::arg("instance"), py::arg("video_data"));

  m.def("send_send_audio_v2", &NDIlib_send_send_audio_v2, py::arg("instance"),
        py::arg("audio_data"));

  m.def("send_send_metadata", &NDIlib_send_send_metadata, py::arg("instance"),
        py::arg("metadata"));

  m.def("send_capture", &NDIlib_send_capture, py::arg("instance"),
        py::arg("metadata"), py::arg("timeout_in_ms"));

  m.def("send_free_metadata", &NDIlib_send_free_metadata, py::arg("instance"),
        py::arg("metadata"));

  m.def("send_get_tally", &NDIlib_send_get_tally, py::arg("instance"),
        py::arg("tally"), py::arg("timeout_in_ms"));

  m.def("send_get_no_connections", &NDIlib_send_get_no_connections,
        py::arg("instance"), py::arg("timeout_in_ms"));

  m.def("send_clear_connection_metadata",
        &NDIlib_send_clear_connection_metadata, py::arg("instance"));

  m.def("send_add_connection_metadata", &NDIlib_send_add_connection_metadata,
        py::arg("instance"), py::arg("metadata"));

  m.def("send_set_failover", &NDIlib_send_set_failover, py::arg("instance"),
        py::arg("failover_source"));

  m.def("send_get_source_name", &NDIlib_send_get_source_name,
        py::arg("instance"));

  // Processing.NDI.Routing
  py::class_<NDIlib_routing_create_t>(m, "RoutingCreate")
      .def(py::init<const char *, const char *>(),
           py::arg("ndi_name") = nullptr, py::arg("groups") = nullptr)
      .def_readwrite("ndi_name", &NDIlib_routing_create_t::p_ndi_name)
      .def_readwrite("groups", &NDIlib_routing_create_t::p_groups);

  m.def("routing_create", &NDIlib_routing_create, py::arg("create_settings"));

  m.def("routing_destroy", &NDIlib_routing_destroy, py::arg("instance"));

  m.def("routing_change", &NDIlib_routing_change, py::arg("instance"),
        py::arg("source"));

  m.def("routing_clear", &NDIlib_routing_clear, py::arg("instance"));

  // Processing.NDI.utilities
  py::class_<NDIlib_audio_frame_interleaved_16s_t>(m,
                                                   "AudioFrameInterleaved16s")
      .def(py::init<int, int, int, int64_t, int, int16_t *>(),
           py::arg("sample_rate") = 48000, py::arg("no_channels") = 2,
           py::arg("no_samples") = 0,
           py::arg("timecode") = NDIlib_send_timecode_synthesize,
           py::arg("reference_level") = 0, py::arg("data") = NULL)
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
      .def_readwrite("data", &NDIlib_audio_frame_interleaved_16s_t::p_data);

  py::class_<NDIlib_audio_frame_interleaved_32s_t>(m,
                                                   "AudioFrameInterleaved32s")
      .def(py::init<int, int, int, int64_t, int, int32_t *>(),
           py::arg("sample_rate") = 48000, py::arg("no_channels") = 2,
           py::arg("no_samples") = 0,
           py::arg("timecode") = NDIlib_send_timecode_synthesize,
           py::arg("reference_level") = 0, py::arg("data") = NULL)
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
      .def_readwrite("data", &NDIlib_audio_frame_interleaved_32s_t::p_data);

  py::class_<NDIlib_audio_frame_interleaved_32f_t>(m,
                                                   "AudioFrameInterleaved32f")
      .def(py::init<int, int, int, int64_t, float *>(),
           py::arg("sample_rate") = 48000, py::arg("no_channels") = 2,
           py::arg("no_samples") = 0,
           py::arg("timecode") = NDIlib_send_timecode_synthesize,
           py::arg("data") = NULL)
      .def_readwrite("sample_rate",
                     &NDIlib_audio_frame_interleaved_32f_t::sample_rate)
      .def_readwrite("no_channels",
                     &NDIlib_audio_frame_interleaved_32f_t::no_channels)
      .def_readwrite("no_samples",
                     &NDIlib_audio_frame_interleaved_32f_t::no_samples)
      .def_readwrite("timecode",
                     &NDIlib_audio_frame_interleaved_32f_t::timecode)
      .def_readwrite("data", &NDIlib_audio_frame_interleaved_32f_t::p_data);

  m.def("util_send_send_audio_interleaved_16s",
        &NDIlib_util_send_send_audio_interleaved_16s, py::arg("instance"),
        py::arg("audio_data"));

  m.def("util_send_send_audio_interleaved_32s",
        &NDIlib_util_send_send_audio_interleaved_32s, py::arg("instance"),
        py::arg("audio_data"));

  m.def("util_send_send_audio_interleaved_32f",
        &NDIlib_util_send_send_audio_interleaved_32f, py::arg("instance"),
        py::arg("audio_data"));

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

  // Processing.NDI.deprecated
  // TODO

  // Processing.NDI.FrameSync
  m.def("framesync_create", &NDIlib_framesync_create, py::arg("receiver"));

  m.def("framesync_destroy", &NDIlib_framesync_destroy, py::arg("instance"));

  m.def("framesync_capture_audio", &NDIlib_framesync_capture_audio,
        py::arg("instance"), py::arg("audio_data"), py::arg("sample_rate"),
        py::arg("no_channels"), py::arg("no_samples"));

  m.def("framesync_free_audio", &NDIlib_framesync_free_audio,
        py::arg("instance"), py::arg("audio_data"));

  m.def("framesync_capture_video", &NDIlib_framesync_capture_video,
        py::arg("instance"), py::arg("video_data"),
        py::arg("field_type") = FRAME_FORMAT_TYPE_PROGRESSIVE);

  m.def("framesync_free_video", &NDIlib_framesync_free_video,
        py::arg("instance"), py::arg("video_data"));
}
