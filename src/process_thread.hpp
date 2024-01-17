#ifndef SVS_PROCESS_THREAD_HPP
#define SVS_PROCESS_THREAD_HPP

#include <mutex>
#include <string>
#include <memory>
#include <opencv2/opencv.hpp>
#include "base_thread.hpp"
#include "imagebuffer.hpp"

namespace SVS{
class MultiBufferManager;

class CameraProcessingThread : public BaseThread {
  public:
  CameraProcessingThread(std::shared_ptr<MultiBufferManager> _sptr_capture_buffer_manager, std::shared_ptr<MultiBufferManager> _sptr_proc_buffer_manager, int _device_id, FisheyeCameraModel _camera_model, bool _drop_if_full=true)
    : sptr_capture_buffer_manager_(_sptr_capture_buffer_manager), sptr_proc_buffer_manager_(_sptr_proc_buffer_manager), device_id_(_device_id), camera_model_(_camera_model), drop_if_full_(_drop_if_full) {}
  protected:
  std::shared_ptr<MultiBufferManager> sptr_capture_buffer_manager_ = std::shared_ptr<MultiBufferManager>(nullptr);
  std::shared_ptr<MultiBufferManager> sptr_proc_buffer_manager_ = std::shared_ptr<MultiBufferManager>(nullptr);
  int device_id_;
  FisheyeCameraModel camera_model_;
  bool drop_if_full_;
};
}//namespace SVS
#endif