#include <string>

#include <Processing.NDI.Lib.h>

class PySendCreate : public NDIlib_send_create_t {
public:
  PySendCreate(const std::string &ndi_name_, const std::string &groups_,
               bool clock_video_, bool clock_audio_);
  virtual ~PySendCreate();

  const std::string &getNdiName() const;
  void setNdiName(const std::string &name);

  const std::string &getGroups() const;
  void setGroups(const std::string &groups);

private:
  std::string _ndi_name;
  std::string _groups;
};
