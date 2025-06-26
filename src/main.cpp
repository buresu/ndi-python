#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <Processing.NDI.Lib.h>
#include <Processing.NDI.RecvAdvertiser.h>
#include <Processing.NDI.RecvListener.h>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <chrono>
#include <thread>
namespace py = pybind11;
using namespace pybind11::literals;
namespace ndi_utils {
    inline const char* string_to_nullable_cstr(const std::string& str) {
        return str.empty() ? nullptr : str.c_str();
    }
    inline void set_metadata_safe(std::vector<char>& buffer, const char*& target_ptr, const std::string& meta) {
        if (meta.empty()) {
            buffer.clear();
            target_ptr = nullptr;
        } else {
            buffer.assign(meta.begin(), meta.end());
            buffer.push_back('\0');
            target_ptr = buffer.data();
        }
    }
    inline std::string get_frame_type_description(NDIlib_frame_type_e frame_type) {
        switch (frame_type) {
            case NDIlib_frame_type_none:
                return "No frame (timeout)";
            case NDIlib_frame_type_video:
                return "Video frame captured";
            case NDIlib_frame_type_audio:
                return "Audio frame captured";
            case NDIlib_frame_type_metadata:
                return "Metadata captured";
            case NDIlib_frame_type_error:
                return "Capture error occurred";
            case NDIlib_frame_type_status_change:
                return "Status change detected";
            default:
                return "Unknown frame type (" + std::to_string(static_cast<int>(frame_type)) + ")";
        }
    }
}
class NDIException : public std::exception {
public:
    NDIException(const std::string& message) : message_(message) {}
    const char* what() const noexcept override {
        return message_.c_str();
    }
private:
    std::string message_;
};
inline void check_ndi_result(bool result, const std::string& operation) {
    if (!result) {
        throw NDIException("NDI operation failed: " + operation);
    }
}
enum class NDIErrorSeverity {
    WARNING,
    ERROR,
    CRITICAL
};
struct NDIOperationResult {
    bool success;
    NDIErrorSeverity severity;
    std::string message;
    NDIOperationResult(bool s, NDIErrorSeverity sev = NDIErrorSeverity::ERROR,
                      const std::string& msg = "")
        : success(s), severity(sev), message(msg) {}
    operator bool() const { return success; }
};
struct CaptureResult {
    NDIlib_frame_type_e frame_type;
    py::object video;
    py::object audio;
    py::object metadata;
    bool success;
    std::string status_message;
    CaptureResult(NDIlib_frame_type_e type = NDIlib_frame_type_none,
                  bool success = false,
                  const std::string& message = "")
        : frame_type(type), success(success), status_message(message),
          video(py::none()), audio(py::none()), metadata(py::none()) {}
    bool has_video() const { return frame_type == NDIlib_frame_type_video; }
    bool has_audio() const { return frame_type == NDIlib_frame_type_audio; }
    bool has_metadata() const { return frame_type == NDIlib_frame_type_metadata; }
    bool is_timeout() const { return frame_type == NDIlib_frame_type_none; }
    bool is_error() const { return frame_type == NDIlib_frame_type_error; }
    bool is_status_change() const { return frame_type == NDIlib_frame_type_status_change; }
    std::tuple<NDIlib_frame_type_e, py::object, py::object, py::object> to_tuple() const {
        return std::make_tuple(frame_type, video, audio, metadata);
    }
    std::tuple<bool, std::string, NDIlib_frame_type_e, py::object, py::object, py::object> to_safe_tuple() const {
        return std::make_tuple(success, status_message, frame_type, video, audio, metadata);
    }
};
class Source {
private:
    NDIlib_source_t source_;
    std::string ndi_name_;
    std::string url_address_;
public:
    Source(const std::string& name = "", const std::string& url = "")
        : ndi_name_(name), url_address_(url) {
        source_.p_ndi_name = ndi_utils::string_to_nullable_cstr(ndi_name_);
        source_.p_url_address = ndi_utils::string_to_nullable_cstr(url_address_);
    }
    Source(const Source& other)
        : ndi_name_(other.ndi_name_), url_address_(other.url_address_) {
        source_.p_ndi_name = ndi_utils::string_to_nullable_cstr(ndi_name_);
        source_.p_url_address = ndi_utils::string_to_nullable_cstr(url_address_);
    }
    Source& operator=(const Source& other) {
        if (this != &other) {
            ndi_name_ = other.ndi_name_;
            url_address_ = other.url_address_;
            source_.p_ndi_name = ndi_utils::string_to_nullable_cstr(ndi_name_);
            source_.p_url_address = ndi_utils::string_to_nullable_cstr(url_address_);
        }
        return *this;
    }
    Source(Source&& other) noexcept
        : ndi_name_(std::move(other.ndi_name_)),
          url_address_(std::move(other.url_address_)) {
        source_.p_ndi_name = ndi_utils::string_to_nullable_cstr(ndi_name_);
        source_.p_url_address = ndi_utils::string_to_nullable_cstr(url_address_);
    }
    Source& operator=(Source&& other) noexcept {
        if (this != &other) {
            ndi_name_ = std::move(other.ndi_name_);
            url_address_ = std::move(other.url_address_);
            source_.p_ndi_name = ndi_utils::string_to_nullable_cstr(ndi_name_);
            source_.p_url_address = ndi_utils::string_to_nullable_cstr(url_address_);
        }
        return *this;
    }
    std::string get_ndi_name() const { return ndi_name_; }
    void set_ndi_name(const std::string& name) {
        ndi_name_ = name;
        source_.p_ndi_name = ndi_utils::string_to_nullable_cstr(ndi_name_);
    }
    std::string get_url_address() const { return url_address_; }
    void set_url_address(const std::string& url) {
        url_address_ = url;
        source_.p_url_address = ndi_utils::string_to_nullable_cstr(url_address_);
    }
    const NDIlib_source_t* get() const { return &source_; }
    NDIlib_source_t* get() { return &source_; }
};
class VideoFrameV2 {
private:
    NDIlib_video_frame_v2_t frame_;
    py::array_t<uint8_t> data_array_;
    std::vector<char> metadata_buffer_;
public:
    VideoFrameV2(int xres = 0, int yres = 0,
                 NDIlib_FourCC_video_type_e fourcc = NDIlib_FourCC_video_type_UYVY,
                 int frame_rate_N = 30000, int frame_rate_D = 1001,
                 float aspect_ratio = 0.0f,
                 NDIlib_frame_format_type_e format = NDIlib_frame_format_type_progressive,
                 int64_t timecode = 0, int64_t timestamp = 0) {
        frame_.xres = xres;
        frame_.yres = yres;
        frame_.FourCC = fourcc;
        frame_.frame_rate_N = frame_rate_N;
        frame_.frame_rate_D = frame_rate_D;
        frame_.picture_aspect_ratio = aspect_ratio;
        frame_.frame_format_type = format;
        frame_.timecode = timecode;
        frame_.p_data = nullptr;
        frame_.line_stride_in_bytes = 0;
        frame_.p_metadata = nullptr;
        frame_.timestamp = timestamp;
    }
    explicit VideoFrameV2(const NDIlib_video_frame_v2_t& captured_frame) {
        frame_ = captured_frame;
        if (frame_.p_metadata) {
            std::string meta_str(frame_.p_metadata);
            set_metadata(meta_str);
        }
    }
    void set_data(py::array_t<uint8_t> array) {
        data_array_ = array;
        auto info = array.request();
        frame_.p_data = static_cast<uint8_t*>(info.ptr);
        if (info.ndim >= 2) {
            frame_.yres = info.shape[0];
            frame_.xres = info.shape[1];
            frame_.line_stride_in_bytes = info.strides[0];
            if (frame_.xres > 0) {
                frame_.picture_aspect_ratio = frame_.xres / float(frame_.yres);
            }
        }
    }
    py::array_t<uint8_t> get_data() const {
        if (!frame_.p_data) return py::array_t<uint8_t>();
        if (data_array_) return data_array_;
        std::vector<ssize_t> shape = {(ssize_t)frame_.yres, (ssize_t)frame_.xres,
                                     (ssize_t)(frame_.line_stride_in_bytes / frame_.xres)};
        std::vector<ssize_t> strides = {(ssize_t)frame_.line_stride_in_bytes,
                                       (ssize_t)(frame_.line_stride_in_bytes / frame_.xres), (ssize_t)1};
        py::buffer_info buf_info(
            (void*)frame_.p_data,
            sizeof(uint8_t),
            "B",
            3,
            shape,
            strides
        );
        return py::array_t<uint8_t>(buf_info);
    }
    void set_metadata(const std::string& meta) {
        ndi_utils::set_metadata_safe(metadata_buffer_, frame_.p_metadata, meta);
    }
    std::string get_metadata() const {
        if (metadata_buffer_.empty()) {
            return std::string();
        }
        return std::string(metadata_buffer_.data());
    }
    const NDIlib_video_frame_v2_t* get() const { return &frame_; }
    NDIlib_video_frame_v2_t* get() { return &frame_; }
    int get_xres() const { return frame_.xres; }
    void set_xres(int v) { frame_.xres = v; }
    int get_yres() const { return frame_.yres; }
    void set_yres(int v) { frame_.yres = v; }
    NDIlib_FourCC_video_type_e get_fourcc() const { return frame_.FourCC; }
    void set_fourcc(NDIlib_FourCC_video_type_e v) { frame_.FourCC = v; }
    int get_frame_rate_N() const { return frame_.frame_rate_N; }
    void set_frame_rate_N(int v) { frame_.frame_rate_N = v; }
    int get_frame_rate_D() const { return frame_.frame_rate_D; }
    void set_frame_rate_D(int v) { frame_.frame_rate_D = v; }
    float get_picture_aspect_ratio() const { return frame_.picture_aspect_ratio; }
    void set_picture_aspect_ratio(float v) { frame_.picture_aspect_ratio = v; }
    NDIlib_frame_format_type_e get_frame_format_type() const { return frame_.frame_format_type; }
    void set_frame_format_type(NDIlib_frame_format_type_e v) { frame_.frame_format_type = v; }
    int64_t get_timecode() const { return frame_.timecode; }
    void set_timecode(int64_t v) { frame_.timecode = v; }
    int get_line_stride_in_bytes() const { return frame_.line_stride_in_bytes; }
    void set_line_stride_in_bytes(int v) { frame_.line_stride_in_bytes = v; }
    int64_t get_timestamp() const { return frame_.timestamp; }
    void set_timestamp(int64_t v) { frame_.timestamp = v; }
};
class CapturedVideoFrame {
private:
    NDIlib_video_frame_v2_t frame_;
    py::capsule capsule_;
public:
    explicit CapturedVideoFrame(const NDIlib_video_frame_v2_t& frame, py::capsule cap)
        : frame_(frame), capsule_(cap) {}
    py::array_t<uint8_t> get_data() const {
        if (!frame_.p_data) return py::array_t<uint8_t>();
        std::vector<ssize_t> shape = {(ssize_t)frame_.yres, (ssize_t)frame_.xres,
                                     (ssize_t)(frame_.line_stride_in_bytes / frame_.xres)};
        std::vector<ssize_t> strides = {(ssize_t)frame_.line_stride_in_bytes,
                                       (ssize_t)(frame_.line_stride_in_bytes / frame_.xres), (ssize_t)1};
        py::buffer_info buf_info(
            (void*)frame_.p_data,
            sizeof(uint8_t),
            "B",
            3,
            shape,
            strides
        );
        return py::array_t<uint8_t>(buf_info, capsule_);
    }
    int get_xres() const { return frame_.xres; }
    int get_yres() const { return frame_.yres; }
    NDIlib_FourCC_video_type_e get_fourcc() const { return frame_.FourCC; }
    int get_frame_rate_N() const { return frame_.frame_rate_N; }
    int get_frame_rate_D() const { return frame_.frame_rate_D; }
    float get_picture_aspect_ratio() const { return frame_.picture_aspect_ratio; }
    NDIlib_frame_format_type_e get_frame_format_type() const { return frame_.frame_format_type; }
    int64_t get_timecode() const { return frame_.timecode; }
    int64_t get_timestamp() const { return frame_.timestamp; }
    int get_line_stride_in_bytes() const { return frame_.line_stride_in_bytes; }
};
class CapturedAudioFrameV2 {
private:
    NDIlib_audio_frame_v2_t frame_;
    py::capsule capsule_;
public:
    explicit CapturedAudioFrameV2(const NDIlib_audio_frame_v2_t& frame, py::capsule cap)
        : frame_(frame), capsule_(cap) {}
    py::array_t<float> get_data() const {
        if (!frame_.p_data) return py::array_t<float>();
        std::vector<ssize_t> shape = {(ssize_t)frame_.no_channels, (ssize_t)frame_.no_samples};
        std::vector<ssize_t> strides = {(ssize_t)frame_.channel_stride_in_bytes, sizeof(float)};
        py::buffer_info buf_info(
            (void*)frame_.p_data,
            sizeof(float),
            "f",
            2,
            shape,
            strides
        );
        return py::array_t<float>(buf_info, capsule_);
    }
    int get_sample_rate() const { return frame_.sample_rate; }
    int get_no_channels() const { return frame_.no_channels; }
    int get_no_samples() const { return frame_.no_samples; }
    int64_t get_timecode() const { return frame_.timecode; }
    int64_t get_timestamp() const { return frame_.timestamp; }
    int get_channel_stride_in_bytes() const { return frame_.channel_stride_in_bytes; }
};
class CapturedAudioFrameV3 {
private:
    NDIlib_audio_frame_v3_t frame_;
    py::capsule capsule_;
public:
    explicit CapturedAudioFrameV3(const NDIlib_audio_frame_v3_t& frame, py::capsule cap)
        : frame_(frame), capsule_(cap) {}
    py::array_t<uint8_t> get_data() const {
        if (!frame_.p_data) return py::array_t<uint8_t>();
        size_t channel_stride = frame_.channel_stride_in_bytes > 0 ?
                               frame_.channel_stride_in_bytes : frame_.no_samples * sizeof(uint8_t);
        std::vector<ssize_t> shape = {(ssize_t)frame_.no_channels, (ssize_t)frame_.no_samples};
        std::vector<ssize_t> strides = {(ssize_t)channel_stride, sizeof(uint8_t)};
        py::buffer_info buf_info(
            (void*)frame_.p_data,
            sizeof(uint8_t),
            "B",
            2,
            shape,
            strides
        );
        return py::array_t<uint8_t>(buf_info, capsule_);
    }
    int get_sample_rate() const { return frame_.sample_rate; }
    int get_no_channels() const { return frame_.no_channels; }
    int get_no_samples() const { return frame_.no_samples; }
    int64_t get_timecode() const { return frame_.timecode; }
    NDIlib_FourCC_audio_type_e get_fourcc() const { return frame_.FourCC; }
    int64_t get_timestamp() const { return frame_.timestamp; }
    int get_channel_stride_in_bytes() const { return frame_.channel_stride_in_bytes; }
};
class AudioFrameV2 {
private:
    NDIlib_audio_frame_v2_t frame_;
    py::array_t<float> data_array_;
    std::vector<char> metadata_buffer_;
public:
    AudioFrameV2(int sample_rate = 48000, int no_channels = 2, int no_samples = 0,
                 int64_t timecode = NDIlib_send_timecode_synthesize, int64_t timestamp = 0) {
        frame_.sample_rate = sample_rate;
        frame_.no_channels = no_channels;
        frame_.no_samples = no_samples;
        frame_.timecode = timecode;
        frame_.timestamp = timestamp;
        frame_.p_data = nullptr;
        frame_.channel_stride_in_bytes = 0;
        frame_.p_metadata = nullptr;
    }
    explicit AudioFrameV2(const NDIlib_audio_frame_v2_t& captured_frame) {
        frame_ = captured_frame;
        if (frame_.p_metadata) {
            std::string meta_str(frame_.p_metadata);
            set_metadata(meta_str);
        }
    }
    void set_data(py::array_t<float> array) {
        data_array_ = array;
        auto info = array.request();
        frame_.p_data = static_cast<float*>(info.ptr);
        frame_.no_channels = info.shape[0];
        frame_.no_samples = info.shape[1];
        frame_.channel_stride_in_bytes = info.strides[0];
    }
    py::array_t<float> get_data() const {
        if (!frame_.p_data) return py::array_t<float>();
        if (data_array_) return data_array_;
        std::vector<ssize_t> shape = {(ssize_t)frame_.no_channels, (ssize_t)frame_.no_samples};
        std::vector<ssize_t> strides = {(ssize_t)frame_.channel_stride_in_bytes, sizeof(float)};
        py::buffer_info buf_info(
            (void*)frame_.p_data,
            sizeof(float),
            "f",
            2,
            shape,
            strides
        );
        return py::array_t<float>(buf_info);
    }
    void set_metadata(const std::string& meta) {
        ndi_utils::set_metadata_safe(metadata_buffer_, frame_.p_metadata, meta);
    }
    std::string get_metadata() const {
        if (metadata_buffer_.empty()) {
            return std::string();
        }
        return std::string(metadata_buffer_.data());
    }
    const NDIlib_audio_frame_v2_t* get() const { return &frame_; }
    NDIlib_audio_frame_v2_t* get() { return &frame_; }
    int get_sample_rate() const { return frame_.sample_rate; }
    void set_sample_rate(int v) { frame_.sample_rate = v; }
    int get_no_channels() const { return frame_.no_channels; }
    void set_no_channels(int v) { frame_.no_channels = v; }
    int get_no_samples() const { return frame_.no_samples; }
    void set_no_samples(int v) { frame_.no_samples = v; }
    int64_t get_timecode() const { return frame_.timecode; }
    void set_timecode(int64_t v) { frame_.timecode = v; }
    int64_t get_timestamp() const { return frame_.timestamp; }
    void set_timestamp(int64_t v) { frame_.timestamp = v; }
    int get_channel_stride_in_bytes() const { return frame_.channel_stride_in_bytes; }
};
class AudioFrameV3 {
private:
    NDIlib_audio_frame_v3_t frame_;
    py::array_t<uint8_t> data_array_;
    std::vector<char> metadata_buffer_;
public:
    AudioFrameV3(int sample_rate = 48000, int no_channels = 2, int no_samples = 0,
                 int64_t timecode = NDIlib_send_timecode_synthesize,
                 NDIlib_FourCC_audio_type_e fourcc = NDIlib_FourCC_audio_type_FLTP,
                 int64_t timestamp = 0) {
        frame_.sample_rate = sample_rate;
        frame_.no_channels = no_channels;
        frame_.no_samples = no_samples;
        frame_.timecode = timecode;
        frame_.FourCC = fourcc;
        frame_.timestamp = timestamp;
        frame_.p_data = nullptr;
        frame_.channel_stride_in_bytes = 0;
        frame_.p_metadata = nullptr;
    }
    explicit AudioFrameV3(const NDIlib_audio_frame_v3_t& captured_frame) {
        frame_ = captured_frame;
        if (frame_.p_metadata) {
            std::string meta_str(frame_.p_metadata);
            set_metadata(meta_str);
        }
    }
    void set_data(py::array_t<uint8_t> array) {
        data_array_ = array;
        auto info = array.request();
        frame_.p_data = static_cast<uint8_t*>(info.ptr);
        frame_.no_channels = info.shape[0];
        frame_.no_samples = info.shape[1];
        frame_.channel_stride_in_bytes = info.strides[0];
    }
    py::array_t<uint8_t> get_data() const {
        if (!frame_.p_data) return py::array_t<uint8_t>();
        if (data_array_) return data_array_;
        size_t channel_stride = frame_.channel_stride_in_bytes > 0 ?
                               frame_.channel_stride_in_bytes : frame_.no_samples * sizeof(uint8_t);
        std::vector<ssize_t> shape = {(ssize_t)frame_.no_channels, (ssize_t)frame_.no_samples};
        std::vector<ssize_t> strides = {(ssize_t)channel_stride, sizeof(uint8_t)};
        py::buffer_info buf_info(
            (void*)frame_.p_data,
            sizeof(uint8_t),
            "B",
            2,
            shape,
            strides
        );
        return py::array_t<uint8_t>(buf_info);
    }
    void set_metadata(const std::string& meta) {
        ndi_utils::set_metadata_safe(metadata_buffer_, frame_.p_metadata, meta);
    }
    std::string get_metadata() const {
        if (metadata_buffer_.empty()) {
            return std::string();
        }
        return std::string(metadata_buffer_.data());
    }
    const NDIlib_audio_frame_v3_t* get() const { return &frame_; }
    NDIlib_audio_frame_v3_t* get() { return &frame_; }
    int get_sample_rate() const { return frame_.sample_rate; }
    void set_sample_rate(int v) { frame_.sample_rate = v; }
    int get_no_channels() const { return frame_.no_channels; }
    void set_no_channels(int v) { frame_.no_channels = v; }
    int get_no_samples() const { return frame_.no_samples; }
    void set_no_samples(int v) { frame_.no_samples = v; }
    int64_t get_timecode() const { return frame_.timecode; }
    void set_timecode(int64_t v) { frame_.timecode = v; }
    NDIlib_FourCC_audio_type_e get_fourcc() const { return frame_.FourCC; }
    void set_fourcc(NDIlib_FourCC_audio_type_e v) { frame_.FourCC = v; }
    int64_t get_timestamp() const { return frame_.timestamp; }
    void set_timestamp(int64_t v) { frame_.timestamp = v; }
    int get_channel_stride_in_bytes() const { return frame_.channel_stride_in_bytes; }
};
class MetadataFrame {
private:
    NDIlib_metadata_frame_t frame_;
    std::vector<char> buffer_;
public:
    MetadataFrame(const std::string& data = "", int64_t timecode = NDIlib_send_timecode_synthesize) {
        frame_.timecode = timecode;
        set_data(data);
    }
    explicit MetadataFrame(const NDIlib_metadata_frame_t& captured_frame) {
        frame_ = captured_frame;
        if (frame_.p_data && frame_.length > 0) {
            buffer_.assign(frame_.p_data, frame_.p_data + frame_.length);
            frame_.p_data = buffer_.data();
        } else {
            buffer_.clear();
            frame_.p_data = nullptr;
            frame_.length = 0;
        }
    }
    void set_data(const std::string& data) {
        if (data.empty()) {
            buffer_.clear();
            frame_.p_data = nullptr;
            frame_.length = 0;
        } else {
            buffer_.assign(data.begin(), data.end());
            buffer_.push_back('\0');
            frame_.p_data = buffer_.data();
            frame_.length = data.length();
        }
    }
    std::string get_data() const {
        if (buffer_.empty() || frame_.length == 0) {
            return std::string();
        }
        return std::string(buffer_.data(), frame_.length);
    }
    const NDIlib_metadata_frame_t* get() const { return &frame_; }
    NDIlib_metadata_frame_t* get() { return &frame_; }
    int get_length() const { return frame_.length; }
    int64_t get_timecode() const { return frame_.timecode; }
    void set_timecode(int64_t v) { frame_.timecode = v; }
};
class Tally {
private:
    NDIlib_tally_t tally_;
public:
    Tally(bool on_program = false, bool on_preview = false) {
        tally_.on_program = on_program;
        tally_.on_preview = on_preview;
    }
    const NDIlib_tally_t* get() const { return &tally_; }
    NDIlib_tally_t* get() { return &tally_; }
    bool get_on_program() const { return tally_.on_program; }
    void set_on_program(bool v) { tally_.on_program = v; }
    bool get_on_preview() const { return tally_.on_preview; }
    void set_on_preview(bool v) { tally_.on_preview = v; }
};
class RecvPerformance {
private:
    NDIlib_recv_performance_t perf_;
public:
    RecvPerformance(int64_t video_frames = 0, int64_t audio_frames = 0, int64_t metadata_frames = 0) {
        perf_.video_frames = video_frames;
        perf_.audio_frames = audio_frames;
        perf_.metadata_frames = metadata_frames;
    }
    explicit RecvPerformance(const NDIlib_recv_performance_t& perf) : perf_(perf) {}
    const NDIlib_recv_performance_t* get() const { return &perf_; }
    NDIlib_recv_performance_t* get() { return &perf_; }
    int64_t get_video_frames() const { return perf_.video_frames; }
    void set_video_frames(int64_t v) { perf_.video_frames = v; }
    int64_t get_audio_frames() const { return perf_.audio_frames; }
    void set_audio_frames(int64_t v) { perf_.audio_frames = v; }
    int64_t get_metadata_frames() const { return perf_.metadata_frames; }
    void set_metadata_frames(int64_t v) { perf_.metadata_frames = v; }
};
class RecvQueue {
private:
    NDIlib_recv_queue_t queue_;
public:
    RecvQueue(int video_frames = 0, int audio_frames = 0, int metadata_frames = 0) {
        queue_.video_frames = video_frames;
        queue_.audio_frames = audio_frames;
        queue_.metadata_frames = metadata_frames;
    }
    explicit RecvQueue(const NDIlib_recv_queue_t& queue) : queue_(queue) {}
    const NDIlib_recv_queue_t* get() const { return &queue_; }
    NDIlib_recv_queue_t* get() { return &queue_; }
    int get_video_frames() const { return queue_.video_frames; }
    void set_video_frames(int v) { queue_.video_frames = v; }
    int get_audio_frames() const { return queue_.audio_frames; }
    void set_audio_frames(int v) { queue_.audio_frames = v; }
    int get_metadata_frames() const { return queue_.metadata_frames; }
    void set_metadata_frames(int v) { queue_.metadata_frames = v; }
};
class FindCreate {
private:
    NDIlib_find_create_t create_;
    std::string groups_;
    std::string extra_ips_;
public:
    FindCreate(bool show_local = true,
               const std::string& groups = "",
               const std::string& extra_ips = "")
        : groups_(groups), extra_ips_(extra_ips) {
        create_.show_local_sources = show_local;
        create_.p_groups = ndi_utils::string_to_nullable_cstr(groups_);
        create_.p_extra_ips = ndi_utils::string_to_nullable_cstr(extra_ips_);
    }
    const NDIlib_find_create_t* get() const { return &create_; }
    bool get_show_local_sources() const { return create_.show_local_sources; }
    void set_show_local_sources(bool v) { create_.show_local_sources = v; }
    std::string get_groups() const { return groups_; }
    void set_groups(const std::string& groups) {
        groups_ = groups;
        create_.p_groups = ndi_utils::string_to_nullable_cstr(groups_);
    }
    std::string get_extra_ips() const { return extra_ips_; }
    void set_extra_ips(const std::string& extra_ips) {
        extra_ips_ = extra_ips;
        create_.p_extra_ips = ndi_utils::string_to_nullable_cstr(extra_ips_);
    }
};
class Finder {
private:
    std::unique_ptr<NDIlib_find_instance_type,
                    decltype(&NDIlib_find_destroy)> instance_;
public:
    explicit Finder(const FindCreate& create_settings)
        : instance_(NDIlib_find_create_v2(create_settings.get()),
                   &NDIlib_find_destroy) {
        if (!instance_) {
            throw NDIException("Failed to create NDI finder");
        }
    }
    Finder(const Finder&) = delete;
    Finder& operator=(const Finder&) = delete;
    Finder(Finder&&) = default;
    Finder& operator=(Finder&&) = default;
    py::list get_sources() {
        uint32_t count = 0;
        const NDIlib_source_t* sources;
        {
            py::gil_scoped_release release;
            sources = NDIlib_find_get_current_sources(instance_.get(), &count);
        }
        py::list result;
        for (uint32_t i = 0; i < count; ++i) {
            result.append(Source(
                sources[i].p_ndi_name ? sources[i].p_ndi_name : "",
                sources[i].p_url_address ? sources[i].p_url_address : ""
            ));
        }
        return result;
    }
    void wait_for_sources(uint32_t timeout_ms) {
        bool result;
        {
            py::gil_scoped_release release;
            result = NDIlib_find_wait_for_sources(instance_.get(), timeout_ms);
        }
        check_ndi_result(result, "find_wait_for_sources with timeout " + std::to_string(timeout_ms) + "ms");
    }
    bool try_wait_for_sources(uint32_t timeout_ms) {
        py::gil_scoped_release release;
        return NDIlib_find_wait_for_sources(instance_.get(), timeout_ms);
    }
    py::tuple wait_for_sources_safe(uint32_t timeout_ms) {
        try {
            bool result;
            {
                py::gil_scoped_release release;
                result = NDIlib_find_wait_for_sources(instance_.get(), timeout_ms);
            }
            if (result) {
                uint32_t count = 0;
                const NDIlib_source_t* sources;
                {
                    py::gil_scoped_release release;
                    sources = NDIlib_find_get_current_sources(instance_.get(), &count);
                }
                return py::make_tuple(true, "SUCCESS",
                    py::str("Sources updated, " + std::to_string(count) + " sources available"));
            } else {
                return py::make_tuple(false, "TIMEOUT",
                    py::str("No source changes detected within " + std::to_string(timeout_ms) + "ms"));
            }
        } catch (const std::exception& e) {
            return py::make_tuple(false, "ERROR",
                py::str("Source discovery error: " + std::string(e.what())));
        }
    }
};
class SendCreate {
private:
    NDIlib_send_create_t create_;
    std::string ndi_name_;
    std::string groups_;
public:
    SendCreate(const std::string& ndi_name = "",
               const std::string& groups = "",
               bool clock_video = true,
               bool clock_audio = true)
        : ndi_name_(ndi_name), groups_(groups) {
        create_.p_ndi_name = ndi_utils::string_to_nullable_cstr(ndi_name_);
        create_.p_groups = ndi_utils::string_to_nullable_cstr(groups_);
        create_.clock_video = clock_video;
        create_.clock_audio = clock_audio;
    }
    const NDIlib_send_create_t* get() const { return &create_; }
    std::string get_ndi_name() const { return ndi_name_; }
    void set_ndi_name(const std::string& name) {
        ndi_name_ = name;
        create_.p_ndi_name = ndi_utils::string_to_nullable_cstr(ndi_name_);
    }
    std::string get_groups() const { return groups_; }
    void set_groups(const std::string& groups) {
        groups_ = groups;
        create_.p_groups = ndi_utils::string_to_nullable_cstr(groups_);
    }
    bool get_clock_video() const { return create_.clock_video; }
    void set_clock_video(bool v) { create_.clock_video = v; }
    bool get_clock_audio() const { return create_.clock_audio; }
    void set_clock_audio(bool v) { create_.clock_audio = v; }
};
class Sender {
private:
    std::unique_ptr<NDIlib_send_instance_type,
                    decltype(&NDIlib_send_destroy)> instance_;
    NDIlib_metadata_frame_t captured_metadata_;
public:
    explicit Sender(const SendCreate& create_settings)
        : instance_(NDIlib_send_create(create_settings.get()),
                   &NDIlib_send_destroy),
          captured_metadata_{} {
        if (!instance_) {
            throw NDIException("Failed to create NDI sender");
        }
    }
    ~Sender() {
        if (captured_metadata_.p_data) {
            NDIlib_send_free_metadata(instance_.get(), &captured_metadata_);
        }
    }
    Sender(const Sender&) = delete;
    Sender& operator=(const Sender&) = delete;
    Sender(Sender&&) = default;
    Sender& operator=(Sender&&) = default;
    void send_video_v2(const VideoFrameV2& video_frame) {
        NDIlib_send_send_video_v2(instance_.get(), video_frame.get());
    }
    void send_video_async_v2(const VideoFrameV2& video_frame) {
        NDIlib_send_send_video_async_v2(instance_.get(), video_frame.get());
    }
    void send_audio_v2(const AudioFrameV2& audio_frame) {
        NDIlib_send_send_audio_v2(instance_.get(), audio_frame.get());
    }
    void send_audio_v3(const AudioFrameV3& audio_frame) {
        NDIlib_send_send_audio_v3(instance_.get(), audio_frame.get());
    }
    void send_metadata(const MetadataFrame& metadata) {
        NDIlib_send_send_metadata(instance_.get(), metadata.get());
    }
    std::tuple<NDIlib_frame_type_e, py::object, py::object, py::object>
    capture(uint32_t timeout_ms) {
        if (captured_metadata_.p_data) {
            NDIlib_send_free_metadata(instance_.get(), &captured_metadata_);
            captured_metadata_ = {};
        }
        NDIlib_frame_type_e type;
        {
            py::gil_scoped_release release;
            type = NDIlib_send_capture(instance_.get(), &captured_metadata_, timeout_ms);
        }
        py::object video = py::none(), audio = py::none(), metadata = py::none();
        if (type == NDIlib_frame_type_metadata) {
            py::dict meta_dict;
            meta_dict["data"] = py::bytes(captured_metadata_.p_data, captured_metadata_.length);
            meta_dict["timecode"] = captured_metadata_.timecode;
            meta_dict["length"] = captured_metadata_.length;
            metadata = meta_dict;
            NDIlib_send_free_metadata(instance_.get(), &captured_metadata_);
        }
        return std::make_tuple(type, video, audio, metadata);
    }
    CaptureResult capture_enhanced(uint32_t timeout_ms) {
        if (captured_metadata_.p_data) {
            NDIlib_send_free_metadata(instance_.get(), &captured_metadata_);
            captured_metadata_ = {};
        }
        NDIlib_frame_type_e type;
        {
            py::gil_scoped_release release;
            type = NDIlib_send_capture(instance_.get(), &captured_metadata_, timeout_ms);
        }
        CaptureResult result(type, true, ndi_utils::get_frame_type_description(type));
        switch (type) {
            case NDIlib_frame_type_metadata: {
                py::dict meta_dict;
                meta_dict["data"] = py::bytes(captured_metadata_.p_data, captured_metadata_.length);
                meta_dict["timecode"] = captured_metadata_.timecode;
                meta_dict["length"] = captured_metadata_.length;
                result.metadata = meta_dict;
                NDIlib_send_free_metadata(instance_.get(), &captured_metadata_);
                captured_metadata_ = {};
                break;
            }
            case NDIlib_frame_type_none:
                result.success = false;
                result.status_message = "Timeout: No metadata available within " + std::to_string(timeout_ms) + "ms";
                break;
            case NDIlib_frame_type_error:
                result.success = false;
                result.status_message = "Error: NDI capture operation failed";
                break;
            default:
                result.status_message = "Unexpected frame type: " + std::to_string(static_cast<int>(type));
                break;
        }
        return result;
    }
    void get_tally(Tally& tally, uint32_t timeout_in_ms) {
        bool result = NDIlib_send_get_tally(instance_.get(), tally.get(), timeout_in_ms);
        check_ndi_result(result, "send_get_tally");
    }
    bool try_get_tally(Tally& tally, uint32_t timeout_in_ms) {
        return NDIlib_send_get_tally(instance_.get(), tally.get(), timeout_in_ms);
    }
    int get_no_connections(uint32_t timeout_in_ms) {
        py::gil_scoped_release release;
        return NDIlib_send_get_no_connections(instance_.get(), timeout_in_ms);
    }
    void clear_connection_metadata() {
        NDIlib_send_clear_connection_metadata(instance_.get());
    }
    void add_connection_metadata(const MetadataFrame& metadata) {
        NDIlib_send_add_connection_metadata(instance_.get(), metadata.get());
    }
    void set_failover(const Source& failover_source) {
        NDIlib_send_set_failover(instance_.get(), failover_source.get());
    }
    Source get_source_name() {
        auto src = NDIlib_send_get_source_name(instance_.get());
        if (src) {
            return Source(
                src->p_ndi_name ? src->p_ndi_name : "",
                src->p_url_address ? src->p_url_address : ""
            );
        }
        return Source();
    }
};
class RecvCreate {
private:
    NDIlib_recv_create_v3_t create_;
    std::string recv_name_;
public:
    RecvCreate(const Source& source = Source(),
               NDIlib_recv_color_format_e color_format = NDIlib_recv_color_format_UYVY_BGRA,
               NDIlib_recv_bandwidth_e bandwidth = NDIlib_recv_bandwidth_highest,
               bool allow_video_fields = true,
               const std::string& recv_name = "")
        : recv_name_(recv_name) {
        create_.source_to_connect_to = *source.get();
        create_.color_format = color_format;
        create_.bandwidth = bandwidth;
        create_.allow_video_fields = allow_video_fields;
        create_.p_ndi_recv_name = ndi_utils::string_to_nullable_cstr(recv_name_);
    }
    const NDIlib_recv_create_v3_t* get() const { return &create_; }
    Source get_source() const {
        return Source(
            create_.source_to_connect_to.p_ndi_name ? create_.source_to_connect_to.p_ndi_name : "",
            create_.source_to_connect_to.p_url_address ? create_.source_to_connect_to.p_url_address : ""
        );
    }
    void set_source(const Source& source) {
        create_.source_to_connect_to = *source.get();
    }
    NDIlib_recv_color_format_e get_color_format() const { return create_.color_format; }
    void set_color_format(NDIlib_recv_color_format_e format) { create_.color_format = format; }
    NDIlib_recv_bandwidth_e get_bandwidth() const { return create_.bandwidth; }
    void set_bandwidth(NDIlib_recv_bandwidth_e bw) { create_.bandwidth = bw; }
    bool get_allow_video_fields() const { return create_.allow_video_fields; }
    void set_allow_video_fields(bool allow) { create_.allow_video_fields = allow; }
    std::string get_recv_name() const { return recv_name_; }
    void set_recv_name(const std::string& name) {
        recv_name_ = name;
        create_.p_ndi_recv_name = ndi_utils::string_to_nullable_cstr(recv_name_);
    }
};
class Receiver {
private:
    std::unique_ptr<NDIlib_recv_instance_type,
                    decltype(&NDIlib_recv_destroy)> instance_;
    NDIlib_video_frame_v2_t captured_video_;
    NDIlib_audio_frame_v2_t captured_audio_v2_;
    NDIlib_audio_frame_v3_t captured_audio_v3_;
    NDIlib_metadata_frame_t captured_metadata_;
    py::capsule video_capsule_;
    py::capsule audio_v2_capsule_;
    py::capsule audio_v3_capsule_;
    py::capsule metadata_capsule_;
public:
    explicit Receiver(const RecvCreate& create_settings)
        : instance_(NDIlib_recv_create_v3(create_settings.get()),
                   &NDIlib_recv_destroy) {
        if (!instance_) {
            throw NDIException("Failed to create NDI receiver");
        }
        captured_video_ = {};
        captured_audio_v2_ = {};
        captured_audio_v3_ = {};
        captured_metadata_ = {};
    }
    ~Receiver() {
        cleanup_all_frames();
    }
private:
    void cleanup_all_frames() {
        if (captured_video_.p_data) {
            NDIlib_recv_free_video_v2(instance_.get(), &captured_video_);
            captured_video_ = {};
        }
        if (captured_audio_v2_.p_data) {
            NDIlib_recv_free_audio_v2(instance_.get(), &captured_audio_v2_);
            captured_audio_v2_ = {};
        }
        if (captured_audio_v3_.p_data) {
            NDIlib_recv_free_audio_v3(instance_.get(), &captured_audio_v3_);
            captured_audio_v3_ = {};
        }
        if (captured_metadata_.p_data) {
            NDIlib_recv_free_metadata(instance_.get(), &captured_metadata_);
            captured_metadata_ = {};
        }
    }
    void cleanup_previous_frames() {
        cleanup_all_frames();
    }
    void cleanup_unused_frames(NDIlib_frame_type_e used_type) {
        if (used_type != NDIlib_frame_type_video && captured_video_.p_data) {
            NDIlib_recv_free_video_v2(instance_.get(), &captured_video_);
            captured_video_ = {};
        }
        if (used_type != NDIlib_frame_type_audio) {
            if (captured_audio_v2_.p_data) {
                NDIlib_recv_free_audio_v2(instance_.get(), &captured_audio_v2_);
                captured_audio_v2_ = {};
            }
            if (captured_audio_v3_.p_data) {
                NDIlib_recv_free_audio_v3(instance_.get(), &captured_audio_v3_);
                captured_audio_v3_ = {};
            }
        }
        if (used_type != NDIlib_frame_type_metadata && captured_metadata_.p_data) {
            NDIlib_recv_free_metadata(instance_.get(), &captured_metadata_);
            captured_metadata_ = {};
        }
    }
public:
    Receiver(const Receiver&) = delete;
    Receiver& operator=(const Receiver&) = delete;
    Receiver(Receiver&&) = default;
    Receiver& operator=(Receiver&&) = default;
    void connect(const Source& source) {
        py::gil_scoped_release release;
        NDIlib_recv_connect(instance_.get(), source.get());
    }
    void disconnect() {
        py::gil_scoped_release release;
        NDIlib_recv_connect(instance_.get(), nullptr);
    }
    py::tuple connect_safe(const Source& source, uint32_t retry_count = 3) {
        for (uint32_t attempt = 0; attempt < retry_count; ++attempt) {
            try {
                {
                    py::gil_scoped_release release;
                    NDIlib_recv_connect(instance_.get(), source.get());
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                int connections = get_no_connections();
                if (connections > 0) {
                    return py::make_tuple(true, "CONNECTED",
                        py::str("Successfully connected to " + source.get_ndi_name()));
                }
                if (attempt < retry_count - 1) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }
            } catch (const std::exception& e) {
                if (attempt == retry_count - 1) {
                    return py::make_tuple(false, "ERROR",
                        py::str("Connection failed after " + std::to_string(retry_count) + " attempts: " + e.what()));
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        }
        return py::make_tuple(false, "TIMEOUT",
            py::str("Connection timed out after " + std::to_string(retry_count) + " attempts"));
    }
    std::tuple<NDIlib_frame_type_e, py::object, py::object, py::object>
    capture_v2(uint32_t timeout_in_ms) {
        cleanup_previous_frames();
        NDIlib_frame_type_e type;
        {
            py::gil_scoped_release release;
            type = NDIlib_recv_capture_v2(instance_.get(), &captured_video_, &captured_audio_v2_, &captured_metadata_, timeout_in_ms);
        }
        py::object video = py::none(), audio = py::none(), metadata = py::none();
        if (type == NDIlib_frame_type_video) {
            struct VideoFrameDeleter {
                NDIlib_recv_instance_t instance;
                NDIlib_video_frame_v2_t frame;
                ~VideoFrameDeleter() {
                    NDIlib_recv_free_video_v2(instance, &frame);
                }
            };
            auto* deleter = new VideoFrameDeleter{instance_.get(), captured_video_};
            video_capsule_ = py::capsule(deleter, "video_frame", [](void* ptr) {
                delete static_cast<VideoFrameDeleter*>(ptr);
            });
            video = py::cast(CapturedVideoFrame(deleter->frame, video_capsule_));
            captured_video_ = {};
        }
        if (type == NDIlib_frame_type_audio) {
            struct AudioFrameV2Deleter {
                NDIlib_recv_instance_t instance;
                NDIlib_audio_frame_v2_t frame;
                ~AudioFrameV2Deleter() {
                    NDIlib_recv_free_audio_v2(instance, &frame);
                }
            };
            auto* deleter = new AudioFrameV2Deleter{instance_.get(), captured_audio_v2_};
            audio_v2_capsule_ = py::capsule(deleter, "audio_frame_v2", [](void* ptr) {
                delete static_cast<AudioFrameV2Deleter*>(ptr);
            });
            audio = py::cast(CapturedAudioFrameV2(deleter->frame, audio_v2_capsule_));
            captured_audio_v2_ = {};
        }
        if (type == NDIlib_frame_type_metadata) {
                py::dict meta_dict;
            meta_dict["data"] = py::bytes(captured_metadata_.p_data, captured_metadata_.length);
            meta_dict["timecode"] = captured_metadata_.timecode;
            meta_dict["length"] = captured_metadata_.length;
            metadata = meta_dict;
                NDIlib_recv_free_metadata(instance_.get(), &captured_metadata_);
            captured_metadata_ = {};
        }
        cleanup_unused_frames(type);
        return std::make_tuple(type, video, audio, metadata);
    }
    py::tuple capture_v2_safe(uint32_t timeout_in_ms, uint32_t max_retries = 2) {
        for (uint32_t retry = 0; retry <= max_retries; ++retry) {
            try {
                auto result = capture_v2(timeout_in_ms);
                NDIlib_frame_type_e frame_type = std::get<0>(result);
                if (frame_type == NDIlib_frame_type_error) {
                    if (retry < max_retries) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                        continue;
                    } else {
                        return py::make_tuple(false, "FRAME_ERROR",
                            py::str("Persistent frame error after " + std::to_string(max_retries + 1) + " attempts"),
                            py::none(), py::none(), py::none());
                    }
                }
                return py::make_tuple(true, "SUCCESS", py::str("Capture successful"),
                    std::get<1>(result), std::get<2>(result), std::get<3>(result));
            } catch (const NDIException& e) {
                if (retry < max_retries) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    continue;
                } else {
                    return py::make_tuple(false, "NDI_ERROR",
                        py::str("NDI error after " + std::to_string(max_retries + 1) + " attempts: " + e.what()),
                        py::none(), py::none(), py::none());
                }
            } catch (const std::exception& e) {
                return py::make_tuple(false, "SYSTEM_ERROR",
                    py::str("System error: " + std::string(e.what())),
                    py::none(), py::none(), py::none());
            }
        }
        return py::make_tuple(false, "UNKNOWN_ERROR", py::str("Unexpected error in capture"),
            py::none(), py::none(), py::none());
    }
    CaptureResult capture_v2_enhanced(uint32_t timeout_in_ms) {
        cleanup_previous_frames();
        NDIlib_frame_type_e type;
        {
            py::gil_scoped_release release;
            type = NDIlib_recv_capture_v2(instance_.get(), &captured_video_,
                                         &captured_audio_v2_, &captured_metadata_, timeout_in_ms);
        }
        CaptureResult result(type, true, ndi_utils::get_frame_type_description(type));
        switch (type) {
            case NDIlib_frame_type_video: {
                struct VideoFrameDeleter {
                    NDIlib_recv_instance_t instance;
                    NDIlib_video_frame_v2_t frame;
                    ~VideoFrameDeleter() {
                        NDIlib_recv_free_video_v2(instance, &frame);
                    }
                };
                auto* deleter = new VideoFrameDeleter{instance_.get(), captured_video_};
                auto video_capsule = py::capsule(deleter, "video_frame", [](void* ptr) {
                    delete static_cast<VideoFrameDeleter*>(ptr);
                });
                result.video = py::cast(CapturedVideoFrame(deleter->frame, video_capsule));
                    captured_video_ = {};
                break;
            }
            case NDIlib_frame_type_audio: {
                struct AudioFrameV2Deleter {
                    NDIlib_recv_instance_t instance;
                    NDIlib_audio_frame_v2_t frame;
                    ~AudioFrameV2Deleter() {
                        NDIlib_recv_free_audio_v2(instance, &frame);
                    }
                };
                auto* deleter = new AudioFrameV2Deleter{instance_.get(), captured_audio_v2_};
                auto audio_v2_capsule = py::capsule(deleter, "audio_frame_v2", [](void* ptr) {
                    delete static_cast<AudioFrameV2Deleter*>(ptr);
                });
                result.audio = py::cast(CapturedAudioFrameV2(deleter->frame, audio_v2_capsule));
                    captured_audio_v2_ = {};
                break;
            }
            case NDIlib_frame_type_metadata: {
                py::dict meta_dict;
                meta_dict["data"] = py::bytes(captured_metadata_.p_data, captured_metadata_.length);
                meta_dict["timecode"] = captured_metadata_.timecode;
                meta_dict["length"] = captured_metadata_.length;
                result.metadata = meta_dict;
                NDIlib_recv_free_metadata(instance_.get(), &captured_metadata_);
                captured_metadata_ = {};
                break;
            }
            case NDIlib_frame_type_none:
                result.success = false;
                result.status_message = "Timeout: No frame available within " + std::to_string(timeout_in_ms) + "ms";
                break;
            case NDIlib_frame_type_error:
                result.success = false;
                result.status_message = "Error: NDI capture operation failed";
                break;
            case NDIlib_frame_type_status_change:
                result.status_message = "Status change: Connection status has changed";
                break;
            default:
                result.status_message = "Unexpected frame type: " + std::to_string(static_cast<int>(type));
                break;
        }
            cleanup_unused_frames(type);
        return result;
    }
    std::tuple<NDIlib_frame_type_e, py::object, py::object, py::object>
    capture_v3(uint32_t timeout_in_ms) {
        cleanup_previous_frames();
        NDIlib_frame_type_e type;
        {
            py::gil_scoped_release release;
            type = NDIlib_recv_capture_v3(instance_.get(), &captured_video_, &captured_audio_v3_, &captured_metadata_, timeout_in_ms);
        }
        py::object video = py::none(), audio = py::none(), metadata = py::none();
        if (type == NDIlib_frame_type_video) {
            struct VideoFrameDeleter {
                NDIlib_recv_instance_t instance;
                NDIlib_video_frame_v2_t frame;
                ~VideoFrameDeleter() {
                    NDIlib_recv_free_video_v2(instance, &frame);
                }
            };
            auto* deleter = new VideoFrameDeleter{instance_.get(), captured_video_};
            video_capsule_ = py::capsule(deleter, "video_frame", [](void* ptr) {
                delete static_cast<VideoFrameDeleter*>(ptr);
            });
            video = py::cast(CapturedVideoFrame(deleter->frame, video_capsule_));
            captured_video_ = {};
        }
        if (type == NDIlib_frame_type_audio) {
            struct AudioFrameV3Deleter {
                NDIlib_recv_instance_t instance;
                NDIlib_audio_frame_v3_t frame;
                ~AudioFrameV3Deleter() {
                    NDIlib_recv_free_audio_v3(instance, &frame);
                }
            };
            auto* deleter = new AudioFrameV3Deleter{instance_.get(), captured_audio_v3_};
            audio_v3_capsule_ = py::capsule(deleter, "audio_frame_v3", [](void* ptr) {
                delete static_cast<AudioFrameV3Deleter*>(ptr);
            });
            audio = py::cast(CapturedAudioFrameV3(deleter->frame, audio_v3_capsule_));
            captured_audio_v3_ = {};
        }
        if (type == NDIlib_frame_type_metadata) {
                py::dict meta_dict;
            meta_dict["data"] = py::bytes(captured_metadata_.p_data, captured_metadata_.length);
            meta_dict["timecode"] = captured_metadata_.timecode;
            meta_dict["length"] = captured_metadata_.length;
            metadata = meta_dict;
                NDIlib_recv_free_metadata(instance_.get(), &captured_metadata_);
            captured_metadata_ = {};
        }
        cleanup_unused_frames(type);
        return std::make_tuple(type, video, audio, metadata);
    }
    py::tuple capture_v3_safe(uint32_t timeout_in_ms, uint32_t max_retries = 2) {
        for (uint32_t retry = 0; retry <= max_retries; ++retry) {
            try {
                auto result = capture_v3(timeout_in_ms);
                NDIlib_frame_type_e frame_type = std::get<0>(result);
                if (frame_type == NDIlib_frame_type_error) {
                    if (retry < max_retries) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                        continue;
                    } else {
                        return py::make_tuple(false, "FRAME_ERROR",
                            py::str("Persistent frame error after " + std::to_string(max_retries + 1) + " attempts"),
                            py::none(), py::none(), py::none());
                    }
                }
                return py::make_tuple(true, "SUCCESS", py::str("Capture successful"),
                    std::get<1>(result), std::get<2>(result), std::get<3>(result));
            } catch (const NDIException& e) {
                if (retry < max_retries) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    continue;
                } else {
                    return py::make_tuple(false, "NDI_ERROR",
                        py::str("NDI error after " + std::to_string(max_retries + 1) + " attempts: " + e.what()),
                        py::none(), py::none(), py::none());
                }
            } catch (const std::exception& e) {
                return py::make_tuple(false, "SYSTEM_ERROR",
                    py::str("System error: " + std::string(e.what())),
                    py::none(), py::none(), py::none());
            }
        }
        return py::make_tuple(false, "UNKNOWN_ERROR", py::str("Unexpected error in capture"),
            py::none(), py::none(), py::none());
    }
    void send_metadata(const MetadataFrame& metadata) {
        bool result = NDIlib_recv_send_metadata(instance_.get(), metadata.get());
        check_ndi_result(result, "recv_send_metadata");
    }
    bool try_send_metadata(const MetadataFrame& metadata) {
        return NDIlib_recv_send_metadata(instance_.get(), metadata.get());
    }
    void set_tally(const Tally& tally) {
        bool result = NDIlib_recv_set_tally(instance_.get(), tally.get());
        check_ndi_result(result, "recv_set_tally");
    }
    bool try_set_tally(const Tally& tally) {
        return NDIlib_recv_set_tally(instance_.get(), tally.get());
    }
    std::tuple<RecvPerformance, RecvPerformance> get_performance() {
        NDIlib_recv_performance_t total, dropped;
        NDIlib_recv_get_performance(instance_.get(), &total, &dropped);
        return std::make_tuple(RecvPerformance(total), RecvPerformance(dropped));
    }
    RecvQueue get_queue() {
        NDIlib_recv_queue_t total;
        NDIlib_recv_get_queue(instance_.get(), &total);
        return RecvQueue(total);
    }
    void clear_connection_metadata() {
        NDIlib_recv_clear_connection_metadata(instance_.get());
    }
    void add_connection_metadata(const MetadataFrame& metadata) {
        NDIlib_recv_add_connection_metadata(instance_.get(), metadata.get());
    }
    int get_no_connections() {
        return NDIlib_recv_get_no_connections(instance_.get());
    }
    py::str get_web_control() {
        const char* str;
        {
            py::gil_scoped_release release;
            str = NDIlib_recv_get_web_control(instance_.get());
        }
        if (!str) return py::str("");
        auto ustr = PyUnicode_DecodeLocale(str, nullptr);
        return py::reinterpret_steal<py::str>(ustr);
    }
    py::tuple get_source_name(uint32_t timeout_in_ms = 0) {
        const char* source_name = nullptr;
        bool changed;
        {
            py::gil_scoped_release release;
            changed = NDIlib_recv_get_source_name(instance_.get(), &source_name, timeout_in_ms);
        }
        if (changed && source_name) {
            std::string name(source_name);
            NDIlib_recv_free_string(instance_.get(), source_name);
            return py::make_tuple(true, py::str(name));
        }
        return py::make_tuple(changed, py::none());
    }
};
class RoutingCreate {
private:
    NDIlib_routing_create_t create_;
    std::string ndi_name_;
    std::string groups_;
public:
    RoutingCreate(const std::string& ndi_name = "",
                  const std::string& groups = "")
        : ndi_name_(ndi_name), groups_(groups) {
        create_.p_ndi_name = ndi_utils::string_to_nullable_cstr(ndi_name_);
        create_.p_groups = ndi_utils::string_to_nullable_cstr(groups_);
    }
    const NDIlib_routing_create_t* get() const { return &create_; }
    std::string get_ndi_name() const { return ndi_name_; }
    void set_ndi_name(const std::string& name) {
        ndi_name_ = name;
        create_.p_ndi_name = ndi_utils::string_to_nullable_cstr(ndi_name_);
    }
    std::string get_groups() const { return groups_; }
    void set_groups(const std::string& groups) {
        groups_ = groups;
        create_.p_groups = ndi_utils::string_to_nullable_cstr(groups_);
    }
};
class Router {
private:
    std::unique_ptr<NDIlib_routing_instance_type,
                    decltype(&NDIlib_routing_destroy)> instance_;
public:
    explicit Router(const RoutingCreate& create_settings)
        : instance_(NDIlib_routing_create(create_settings.get()),
                   &NDIlib_routing_destroy) {
        if (!instance_) {
            throw NDIException("Failed to create NDI router");
        }
    }
    Router(const Router&) = delete;
    Router& operator=(const Router&) = delete;
    Router(Router&&) = default;
    Router& operator=(Router&&) = default;
    void change(const Source& source) {
        bool result = NDIlib_routing_change(instance_.get(), source.get());
        check_ndi_result(result, "routing_change");
    }
    bool try_change(const Source& source) {
        return NDIlib_routing_change(instance_.get(), source.get());
    }
    void clear() {
        NDIlib_routing_clear(instance_.get());
    }
    int get_no_connections(uint32_t timeout_in_ms) {
        py::gil_scoped_release release;
        return NDIlib_routing_get_no_connections(instance_.get(), timeout_in_ms);
    }
    Source get_source_name() {
        auto src = NDIlib_routing_get_source_name(instance_.get());
        if (src) {
            return Source(
                src->p_ndi_name ? src->p_ndi_name : "",
                src->p_url_address ? src->p_url_address : ""
            );
        }
        return Source();
    }
};
PYBIND11_MODULE(NDIlib, m) {
  m.doc() = "NDI SDK for Python - High-performance video streaming over IP";
  if (!NDIlib_initialize()) {
      throw std::runtime_error("Failed to initialize NDI library");
  }
  std::atexit([]() {
      NDIlib_destroy();
  });
  py::register_exception<NDIException>(m, "NDIException");
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
      .value("FOURCC_VIDEO_TYPE_YV12", NDIlib_FourCC_video_type_YV12)
      .value("FOURCC_VIDEO_TYPE_I420", NDIlib_FourCC_video_type_I420)
      .value("FOURCC_VIDEO_TYPE_NV12", NDIlib_FourCC_video_type_NV12)
      .value("FOURCC_VIDEO_TYPE_BGRA", NDIlib_FourCC_video_type_BGRA)
      .value("FOURCC_VIDEO_TYPE_BGRX", NDIlib_FourCC_video_type_BGRX)
      .value("FOURCC_VIDEO_TYPE_RGBA", NDIlib_FourCC_video_type_RGBA)
      .value("FOURCC_VIDEO_TYPE_RGBX", NDIlib_FourCC_video_type_RGBX)
      .value("FOURCC_VIDEO_TYPE_P216", NDIlib_FourCC_video_type_P216)
      .value("FOURCC_VIDEO_TYPE_PA16", NDIlib_FourCC_video_type_PA16)
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
  py::enum_<NDIlib_recv_color_format_e>(m, "RecvColorFormat", py::arithmetic())
      .value("RECV_COLOR_FORMAT_BGRX_BGRA", NDIlib_recv_color_format_BGRX_BGRA)
      .value("RECV_COLOR_FORMAT_UYVY_BGRA", NDIlib_recv_color_format_UYVY_BGRA)
      .value("RECV_COLOR_FORMAT_RGBX_RGBA", NDIlib_recv_color_format_RGBX_RGBA)
      .value("RECV_COLOR_FORMAT_UYVY_RGBA", NDIlib_recv_color_format_UYVY_RGBA)
      .value("RECV_COLOR_FORMAT_FASTEST", NDIlib_recv_color_format_fastest)
      .value("RECV_COLOR_FORMAT_BEST", NDIlib_recv_color_format_best)
      .export_values();
  py::enum_<NDIlib_recv_bandwidth_e>(m, "RecvBandwidth", py::arithmetic())
      .value("RECV_BANDWIDTH_METADATA_ONLY", NDIlib_recv_bandwidth_metadata_only)
      .value("RECV_BANDWIDTH_AUDIO_ONLY", NDIlib_recv_bandwidth_audio_only)
      .value("RECV_BANDWIDTH_LOWEST", NDIlib_recv_bandwidth_lowest)
      .value("RECV_BANDWIDTH_HIGHEST", NDIlib_recv_bandwidth_highest)
      .export_values();
  m.attr("SEND_TIMECODE_SYNTHESIZE") = py::int_(INT64_MAX);
  m.attr("RECV_TIMESTAMP_UNDEFINED") = py::int_(INT64_MAX);
  m.def("initialize", &NDIlib_initialize, 
        "Initialize the NDI library. Must be called before any other NDI functions.");
  m.def("destroy", &NDIlib_destroy,
        "Destroy the NDI library and cleanup resources. Call when finished with NDI.");
  m.def("version", &NDIlib_version,
        "Get the NDI SDK version string.");
  m.def("get_runtime_version", []() {
    const NDIlib_v6* p_NDILib = NDIlib_v6_load();
    if (p_NDILib && p_NDILib->version) {
        return py::str(p_NDILib->version());
    }
    return py::str("Unknown");
  }, "Get the runtime version of the currently loaded NDI library.");
  m.def("is_supported_CPU", &NDIlib_is_supported_CPU,
        "Check if the current CPU supports NDI operations (requires SSE4.1).");
  py::class_<Source>(m, "Source")
      .def(py::init<const std::string&, const std::string&>(),
           py::arg("ndi_name") = "",
           py::arg("url_address") = "")
      .def_property("ndi_name",
                    &Source::get_ndi_name,
                    &Source::set_ndi_name)
      .def_property("url_address",
                    &Source::get_url_address,
                    &Source::set_url_address);
  py::class_<VideoFrameV2>(m, "VideoFrameV2")
      .def(py::init<int, int, NDIlib_FourCC_video_type_e,
                    int, int, float, NDIlib_frame_format_type_e, int64_t, int64_t>(),
           py::arg("xres") = 0, py::arg("yres") = 0,
           py::arg("fourcc") = NDIlib_FourCC_video_type_UYVY,
           py::arg("frame_rate_N") = 30000,
           py::arg("frame_rate_D") = 1001,
           py::arg("aspect_ratio") = 0.0f,
           py::arg("format") = NDIlib_frame_format_type_progressive,
           py::arg("timecode") = 0,
           py::arg("timestamp") = 0)
      .def_property("data", &VideoFrameV2::get_data, &VideoFrameV2::set_data)
      .def_property("metadata", &VideoFrameV2::get_metadata, &VideoFrameV2::set_metadata)
      .def_property("xres", &VideoFrameV2::get_xres, &VideoFrameV2::set_xres)
      .def_property("yres", &VideoFrameV2::get_yres, &VideoFrameV2::set_yres)
      .def_property("fourcc", &VideoFrameV2::get_fourcc, &VideoFrameV2::set_fourcc)
      .def_property("frame_rate_N", &VideoFrameV2::get_frame_rate_N, &VideoFrameV2::set_frame_rate_N)
      .def_property("frame_rate_D", &VideoFrameV2::get_frame_rate_D, &VideoFrameV2::set_frame_rate_D)
      .def_property("picture_aspect_ratio", &VideoFrameV2::get_picture_aspect_ratio, &VideoFrameV2::set_picture_aspect_ratio)
      .def_property("frame_format_type", &VideoFrameV2::get_frame_format_type, &VideoFrameV2::set_frame_format_type)
      .def_property("timecode", &VideoFrameV2::get_timecode, &VideoFrameV2::set_timecode)
      .def_property("line_stride_in_bytes", &VideoFrameV2::get_line_stride_in_bytes, &VideoFrameV2::set_line_stride_in_bytes)
      .def_property("timestamp", &VideoFrameV2::get_timestamp, &VideoFrameV2::set_timestamp);
  py::class_<FindCreate>(m, "FindCreate")
      .def(py::init<bool, const std::string&, const std::string&>(),
           py::arg("show_local") = true,
           py::arg("groups") = "",
           py::arg("extra_ips") = "")
      .def_property("show_local_sources", &FindCreate::get_show_local_sources, &FindCreate::set_show_local_sources)
      .def_property("groups", &FindCreate::get_groups, &FindCreate::set_groups)
      .def_property("extra_ips", &FindCreate::get_extra_ips, &FindCreate::set_extra_ips);
  py::class_<Finder>(m, "Finder")
      .def(py::init<const FindCreate&>())
      .def("get_sources", &Finder::get_sources)
      .def("wait_for_sources", &Finder::wait_for_sources,
           py::arg("timeout_ms"))
      .def("try_wait_for_sources", &Finder::try_wait_for_sources,
           py::arg("timeout_ms"))
      .def("wait_for_sources_safe", &Finder::wait_for_sources_safe,
           py::arg("timeout_ms"));
  py::class_<SendCreate>(m, "SendCreate")
      .def(py::init<const std::string&, const std::string&, bool, bool>(),
           py::arg("ndi_name") = "",
           py::arg("groups") = "",
           py::arg("clock_video") = true,
           py::arg("clock_audio") = true)
      .def_property("ndi_name", &SendCreate::get_ndi_name, &SendCreate::set_ndi_name)
      .def_property("groups", &SendCreate::get_groups, &SendCreate::set_groups)
      .def_property("clock_video", &SendCreate::get_clock_video, &SendCreate::set_clock_video)
      .def_property("clock_audio", &SendCreate::get_clock_audio, &SendCreate::set_clock_audio);
  py::class_<Sender>(m, "Sender")
      .def(py::init<const SendCreate&>())
      .def("send_video_v2", &Sender::send_video_v2, py::arg("video_frame"))
      .def("send_video_async_v2", &Sender::send_video_async_v2, py::arg("video_frame"))
      .def("send_audio_v2", &Sender::send_audio_v2, py::arg("audio_frame"))
      .def("send_audio_v3", &Sender::send_audio_v3, py::arg("audio_frame"))
      .def("send_metadata", &Sender::send_metadata, py::arg("metadata"))
      .def("capture", &Sender::capture, py::arg("timeout_in_ms") = 5000)
      .def("capture_enhanced", &Sender::capture_enhanced, py::arg("timeout_in_ms") = 5000,
           "Enhanced capture method returning CaptureResult with better type safety")
      .def("get_tally", &Sender::get_tally, py::arg("tally"), py::arg("timeout_in_ms"))
      .def("try_get_tally", &Sender::try_get_tally, py::arg("tally"), py::arg("timeout_in_ms"))
      .def("get_no_connections", &Sender::get_no_connections, py::arg("timeout_in_ms"))
      .def("clear_connection_metadata", &Sender::clear_connection_metadata)
      .def("add_connection_metadata", &Sender::add_connection_metadata, py::arg("metadata"))
      .def("set_failover", &Sender::set_failover, py::arg("failover_source"))
      .def("get_source_name", &Sender::get_source_name);
  py::class_<RoutingCreate>(m, "RoutingCreate")
      .def(py::init<const std::string&, const std::string&>(),
           py::arg("ndi_name") = "",
           py::arg("groups") = "")
      .def_property("ndi_name",
                    &RoutingCreate::get_ndi_name,
                    &RoutingCreate::set_ndi_name)
      .def_property("groups",
                    &RoutingCreate::get_groups,
                    &RoutingCreate::set_groups);
  py::class_<Router>(m, "Router")
      .def(py::init<const RoutingCreate&>())
      .def("change", &Router::change, py::arg("source"))
      .def("try_change", &Router::try_change, py::arg("source"))
      .def("clear", &Router::clear)
      .def("get_no_connections", &Router::get_no_connections,
           py::arg("timeout_in_ms") = 0)
      .def("get_source_name", &Router::get_source_name);
  py::class_<RecvCreate>(m, "RecvCreate")
      .def(py::init<const Source&, NDIlib_recv_color_format_e, NDIlib_recv_bandwidth_e, bool, const std::string&>(),
           py::arg("source") = Source(),
           py::arg("color_format") = NDIlib_recv_color_format_UYVY_BGRA,
           py::arg("bandwidth") = NDIlib_recv_bandwidth_highest,
           py::arg("allow_video_fields") = true,
           py::arg("recv_name") = "")
      .def_property("source", &RecvCreate::get_source, &RecvCreate::set_source)
      .def_property("color_format", &RecvCreate::get_color_format, &RecvCreate::set_color_format)
      .def_property("bandwidth", &RecvCreate::get_bandwidth, &RecvCreate::set_bandwidth)
      .def_property("allow_video_fields", &RecvCreate::get_allow_video_fields, &RecvCreate::set_allow_video_fields)
      .def_property("recv_name", &RecvCreate::get_recv_name, &RecvCreate::set_recv_name);
  py::class_<Receiver>(m, "Receiver")
      .def(py::init<const RecvCreate&>())
      .def("connect", &Receiver::connect, py::arg("source"))
      .def("disconnect", &Receiver::disconnect)
      .def("capture_v2", &Receiver::capture_v2, py::arg("timeout_in_ms") = 5000)
      .def("capture_v3", &Receiver::capture_v3, py::arg("timeout_in_ms") = 5000)
      .def("capture_v2_enhanced", &Receiver::capture_v2_enhanced, py::arg("timeout_in_ms") = 5000,
           "Enhanced capture_v2 method returning CaptureResult with better type safety")
      .def("connect_safe", &Receiver::connect_safe,
           py::arg("source"), py::arg("retry_count") = 3)
      .def("capture_v2_safe", &Receiver::capture_v2_safe,
           py::arg("timeout_in_ms") = 5000, py::arg("max_retries") = 2)
      .def("capture_v3_safe", &Receiver::capture_v3_safe,
           py::arg("timeout_in_ms") = 5000, py::arg("max_retries") = 2)
      .def("send_metadata", &Receiver::send_metadata)
      .def("try_send_metadata", &Receiver::try_send_metadata)
      .def("set_tally", &Receiver::set_tally)
      .def("try_set_tally", &Receiver::try_set_tally)
      .def("get_performance", &Receiver::get_performance)
      .def("get_queue", &Receiver::get_queue)
      .def("clear_connection_metadata", &Receiver::clear_connection_metadata)
      .def("add_connection_metadata", &Receiver::add_connection_metadata)
      .def("get_no_connections", &Receiver::get_no_connections)
      .def("get_web_control", &Receiver::get_web_control)
      .def("get_source_name", &Receiver::get_source_name, py::arg("timeout_in_ms") = 0);
  py::class_<AudioFrameV2>(m, "AudioFrameV2")
      .def(py::init<int, int, int, int64_t, int64_t>(),
           py::arg("sample_rate") = 48000,
           py::arg("no_channels") = 2,
           py::arg("no_samples") = 0,
           py::arg("timecode") = NDIlib_send_timecode_synthesize,
           py::arg("timestamp") = 0)
      .def_property("data", &AudioFrameV2::get_data, &AudioFrameV2::set_data)
      .def_property("metadata", &AudioFrameV2::get_metadata, &AudioFrameV2::set_metadata)
      .def_property("sample_rate", &AudioFrameV2::get_sample_rate, &AudioFrameV2::set_sample_rate)
      .def_property("no_channels", &AudioFrameV2::get_no_channels, &AudioFrameV2::set_no_channels)
      .def_property("no_samples", &AudioFrameV2::get_no_samples, &AudioFrameV2::set_no_samples)
      .def_property("timecode", &AudioFrameV2::get_timecode, &AudioFrameV2::set_timecode)
      .def_property("timestamp", &AudioFrameV2::get_timestamp, &AudioFrameV2::set_timestamp)
      .def_property_readonly("channel_stride_in_bytes", &AudioFrameV2::get_channel_stride_in_bytes);
  py::class_<AudioFrameV3>(m, "AudioFrameV3")
      .def(py::init<int, int, int, int64_t, NDIlib_FourCC_audio_type_e, int64_t>(),
           py::arg("sample_rate") = 48000,
           py::arg("no_channels") = 2,
           py::arg("no_samples") = 0,
           py::arg("timecode") = NDIlib_send_timecode_synthesize,
           py::arg("fourcc") = NDIlib_FourCC_audio_type_FLTP,
           py::arg("timestamp") = 0)
      .def_property("data", &AudioFrameV3::get_data, &AudioFrameV3::set_data)
      .def_property("metadata", &AudioFrameV3::get_metadata, &AudioFrameV3::set_metadata)
      .def_property("sample_rate", &AudioFrameV3::get_sample_rate, &AudioFrameV3::set_sample_rate)
      .def_property("no_channels", &AudioFrameV3::get_no_channels, &AudioFrameV3::set_no_channels)
      .def_property("no_samples", &AudioFrameV3::get_no_samples, &AudioFrameV3::set_no_samples)
      .def_property("timecode", &AudioFrameV3::get_timecode, &AudioFrameV3::set_timecode)
      .def_property("fourcc", &AudioFrameV3::get_fourcc, &AudioFrameV3::set_fourcc)
      .def_property("timestamp", &AudioFrameV3::get_timestamp, &AudioFrameV3::set_timestamp)
      .def_property_readonly("channel_stride_in_bytes", &AudioFrameV3::get_channel_stride_in_bytes);
  py::class_<MetadataFrame>(m, "MetadataFrame")
      .def(py::init<const std::string&, int64_t>(),
           py::arg("data") = "",
           py::arg("timecode") = NDIlib_send_timecode_synthesize)
      .def_property("data", &MetadataFrame::get_data, &MetadataFrame::set_data)
      .def_property("timecode", &MetadataFrame::get_timecode, &MetadataFrame::set_timecode)
      .def_property_readonly("length", &MetadataFrame::get_length);
  py::class_<Tally>(m, "Tally")
      .def(py::init<bool, bool>(),
           py::arg("on_program") = false,
           py::arg("on_preview") = false)
      .def_property("on_program", &Tally::get_on_program, &Tally::set_on_program)
      .def_property("on_preview", &Tally::get_on_preview, &Tally::set_on_preview);
  py::class_<RecvPerformance>(m, "RecvPerformance")
      .def(py::init<int64_t, int64_t, int64_t>(),
           py::arg("video_frames") = 0,
           py::arg("audio_frames") = 0,
           py::arg("metadata_frames") = 0)
      .def_property("video_frames", &RecvPerformance::get_video_frames, &RecvPerformance::set_video_frames)
      .def_property("audio_frames", &RecvPerformance::get_audio_frames, &RecvPerformance::set_audio_frames)
      .def_property("metadata_frames", &RecvPerformance::get_metadata_frames, &RecvPerformance::set_metadata_frames);
  py::class_<RecvQueue>(m, "RecvQueue")
      .def(py::init<int, int, int>(),
           py::arg("video_frames") = 0,
           py::arg("audio_frames") = 0,
           py::arg("metadata_frames") = 0)
      .def_property("video_frames", &RecvQueue::get_video_frames, &RecvQueue::set_video_frames)
      .def_property("audio_frames", &RecvQueue::get_audio_frames, &RecvQueue::set_audio_frames)
      .def_property("metadata_frames", &RecvQueue::get_metadata_frames, &RecvQueue::set_metadata_frames);
  py::class_<CapturedVideoFrame>(m, "CapturedVideoFrame")
      .def_property_readonly("data", &CapturedVideoFrame::get_data)
      .def_property_readonly("xres", &CapturedVideoFrame::get_xres)
      .def_property_readonly("yres", &CapturedVideoFrame::get_yres)
      .def_property_readonly("fourcc", &CapturedVideoFrame::get_fourcc)
      .def_property_readonly("frame_rate_N", &CapturedVideoFrame::get_frame_rate_N)
      .def_property_readonly("frame_rate_D", &CapturedVideoFrame::get_frame_rate_D)
      .def_property_readonly("picture_aspect_ratio", &CapturedVideoFrame::get_picture_aspect_ratio)
      .def_property_readonly("frame_format_type", &CapturedVideoFrame::get_frame_format_type)
      .def_property_readonly("timecode", &CapturedVideoFrame::get_timecode)
      .def_property_readonly("timestamp", &CapturedVideoFrame::get_timestamp)
      .def_property_readonly("line_stride_in_bytes", &CapturedVideoFrame::get_line_stride_in_bytes);
  py::class_<CapturedAudioFrameV2>(m, "CapturedAudioFrameV2")
      .def_property_readonly("data", &CapturedAudioFrameV2::get_data)
      .def_property_readonly("sample_rate", &CapturedAudioFrameV2::get_sample_rate)
      .def_property_readonly("no_channels", &CapturedAudioFrameV2::get_no_channels)
      .def_property_readonly("no_samples", &CapturedAudioFrameV2::get_no_samples)
      .def_property_readonly("timecode", &CapturedAudioFrameV2::get_timecode)
      .def_property_readonly("timestamp", &CapturedAudioFrameV2::get_timestamp)
      .def_property_readonly("channel_stride_in_bytes", &CapturedAudioFrameV2::get_channel_stride_in_bytes);
  py::class_<CapturedAudioFrameV3>(m, "CapturedAudioFrameV3")
      .def_property_readonly("data", &CapturedAudioFrameV3::get_data)
      .def_property_readonly("sample_rate", &CapturedAudioFrameV3::get_sample_rate)
      .def_property_readonly("no_channels", &CapturedAudioFrameV3::get_no_channels)
      .def_property_readonly("no_samples", &CapturedAudioFrameV3::get_no_samples)
      .def_property_readonly("timecode", &CapturedAudioFrameV3::get_timecode)
      .def_property_readonly("fourcc", &CapturedAudioFrameV3::get_fourcc)
      .def_property_readonly("timestamp", &CapturedAudioFrameV3::get_timestamp)
      .def_property_readonly("channel_stride_in_bytes", &CapturedAudioFrameV3::get_channel_stride_in_bytes);
    py::class_<CaptureResult>(m, "CaptureResult",
        "Enhanced capture result with type-safe frame data and status information")
        .def(py::init<NDIlib_frame_type_e, bool, const std::string&>(),
             py::arg("frame_type") = NDIlib_frame_type_none,
             py::arg("success") = false,
             py::arg("message") = "",
             "Create a capture result object")
        .def_readwrite("frame_type", &CaptureResult::frame_type,
                      "The type of frame that was captured")
        .def_readwrite("video", &CaptureResult::video,
                      "Video frame data (None if not a video frame)")
        .def_readwrite("audio", &CaptureResult::audio,
                      "Audio frame data (None if not an audio frame)")
        .def_readwrite("metadata", &CaptureResult::metadata,
                      "Metadata (None if not metadata)")
        .def_readwrite("success", &CaptureResult::success,
                      "Whether the capture operation was successful")
        .def_readwrite("status_message", &CaptureResult::status_message,
                      "Human-readable status or error message")
        .def("has_video", &CaptureResult::has_video,
             "Check if this result contains video data")
        .def("has_audio", &CaptureResult::has_audio,
             "Check if this result contains audio data")
        .def("has_metadata", &CaptureResult::has_metadata,
             "Check if this result contains metadata")
        .def("is_timeout", &CaptureResult::is_timeout,
             "Check if this result represents a timeout")
        .def("is_error", &CaptureResult::is_error,
             "Check if this result represents an error")
        .def("is_status_change", &CaptureResult::is_status_change,
             "Check if this result represents a status change")
        .def("to_tuple", &CaptureResult::to_tuple,
             "Convert to legacy tuple format (frame_type, video, audio, metadata)")
        .def("to_safe_tuple", &CaptureResult::to_safe_tuple,
             "Convert to safe tuple format (success, message, frame_type, video, audio, metadata)")
        .def("__repr__", [](const CaptureResult& r) {
            return "<CaptureResult frame_type=" + std::to_string(static_cast<int>(r.frame_type)) +
                   " success=" + (r.success ? "True" : "False") +
                   " message='" + r.status_message + "'>";
        });
}