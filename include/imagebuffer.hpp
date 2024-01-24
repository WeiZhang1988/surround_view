#ifndef SVS_IMAGEBUFFER_HPP
#define SVS_IMAGEBUFFER_HPP

#include <mutex>
#include <queue>
#include <unordered_set>
#include <string>
#include <memory>
#include <opencv2/opencv.hpp>
#include "semaphore.hpp"
#include "param_setting.hpp"

namespace SVS{
template<typename StoredDataType>
class Buffer {
  public:
  Buffer(int _buffer_size = 5) : buffer_size_(_buffer_size) {
    free_slots_.release(_buffer_size);
  }
  void add(StoredDataType _data, bool _drop_if_full = false) {
    clear_buffer_add_.acquire();
    if (_drop_if_full) {
      if (free_slots_.try_acquire()) {
        {
          std::unique_lock<decltype(queue_mutex_)> lock{queue_mutex_};
          queue_.push(_data);
        }
        used_slots_.release();
      } 
    } else {
      free_slots_.acquire();
      {
        std::unique_lock<decltype(queue_mutex_)> lock{queue_mutex_};
        queue_.push(_data);
      }
      used_slots_.release();
    }
    clear_buffer_add_.release();
  }
  StoredDataType get() {
    clear_buffer_get_.acquire();
    used_slots_.acquire();
    StoredDataType data;
    {
      std::unique_lock<decltype(queue_mutex_)> lock{queue_mutex_};
      data = queue_.front();
      queue_.pop();
    }
    free_slots_.release();
    clear_buffer_get_.release();
    return data;
  }
  bool clear() {
    if ((queue_.size()) > 0) {
      if (clear_buffer_add_.try_acquire()) {
        if (clear_buffer_get_.try_acquire()) {
          free_slots_.release(queue_.size());
          free_slots_.acquire(buffer_size_);
          used_slots_.acquire(queue_.size());
          for (int i=0; i<queue_.size(); i++) {
            queue_.pop();
          }
          free_slots_.release(buffer_size_);
          clear_buffer_get_.release();
        } else {
          return false;
        }
        clear_buffer_add_.release();
        return true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  }
  int size() {
    return queue_.size();
  }
  int maxsize() {
    return buffer_size_;
  }
  bool isfull() {
    return queue_.size() >= buffer_size_;
  }
  bool isempty() {
    return queue_.empty();
  }
  protected:
  int buffer_size_;
  counting_semaphore<> free_slots_{0}, used_slots_{0}, clear_buffer_add_{1}, clear_buffer_get_{1};
  std::mutex queue_mutex_;
  std::queue<StoredDataType> queue_;
};

class MultiBufferManager : public std::enable_shared_from_this<MultiBufferManager>{
  public:
  MultiBufferManager(bool _do_sync = true) : do_sync_(_do_sync) {}
  void create_buffer_for_device(int _device_id, int _buffer_size, bool _sync=true) {
    if (_sync) {
      std::unique_lock<decltype(mutex_)> lock{mutex_};
      sync_devices_.insert(_device_id);
    }
    buffer_maps_[_device_id] = std::make_shared<Buffer<ImageFrame>>(_buffer_size);
  }
  template<typename ThreadType>
  void bind_thread(std::shared_ptr<ThreadType> _sptr_thread, int _buffer_size, bool _sync=true) {
    _sptr_thread->buffer_manager = shared_from_this();
  }
  void add(int _device_id, ImageFrame _image_frame, bool _drop_if_full) {
    buffer_maps_[_device_id]->add(_image_frame, _drop_if_full);
  }
  ImageFrame get (int _device_id) {
    return buffer_maps_[_device_id]->get();
  }
  void remove_device(int _device_id) {
    buffer_maps_.erase(_device_id);
    {
      std::unique_lock<decltype(mutex_)> lock{mutex_};
      if (sync_devices_.find(_device_id) != sync_devices_.end()) {
        sync_devices_.erase(_device_id);
        cv_.notify_all();
      }
    }
  }
  void sync(int _device_id) {
    std::unique_lock<decltype(mutex_)> lock{mutex_};
    if (sync_devices_.find(_device_id) != sync_devices_.end()) {
      arrived_ += 1;
      if (do_sync_ && (arrived_ == sync_devices_.size())) {
        cv_.notify_all();
      } else {
        cv_.wait(lock);
      }
      arrived_ -= 1;
    }
  }
  void wake_all() {
    std::unique_lock<decltype(mutex_)> lock{mutex_};
    cv_.notify_all();
  }
  void set_sync(bool _enable) {
    do_sync_ = _enable;
  }
  bool sync_enabled() {
    return do_sync_;
  }
  bool sync_enabled_for_device(int _device_id) {
    return (sync_devices_.find(_device_id) != sync_devices_.end());
  }
  bool contains(int _device_id) {
    return buffer_maps_.find(_device_id) != buffer_maps_.end();
  }
  std::string str() {
    std::string keys, devices;
    for (const std::pair<int, std::shared_ptr<Buffer<ImageFrame>>> &item : buffer_maps_) {
      keys += (", " + std::to_string(item.first));
    }
    for (const int &item : sync_devices_) {
      devices += (", " + std::to_string(item));
    }
    return std::string(typeid(MultiBufferManager).name()) + ":\n" + \
           "sync: " + std::to_string(do_sync_) + "\n" + \
           "device: " + keys + "\n" + \
           "sync enabled devices: " + devices + "\n";
  }
  protected:
  std::unordered_set<int> sync_devices_;
  bool do_sync_;
  std::condition_variable cv_;
  std::mutex mutex_;
  std::map<int, std::shared_ptr<Buffer<ImageFrame>>> buffer_maps_;
  int arrived_ = 0;
};

class ProjectedImageBufferManager : public std::enable_shared_from_this<ProjectedImageBufferManager>{
  public:
  ProjectedImageBufferManager(bool _drop_if_full = true, int _buffer_size = 8) : drop_if_full_(_drop_if_full) {
    sptr_buffer_ = std::make_shared<Buffer<std::map<int,cv::Mat>>>(_buffer_size);
  }
  template<typename ThreadType>
  void bind_thread(std::shared_ptr<ThreadType> _sptr_thread) {
    {
      std::unique_lock<std::mutex> lock{mutex_};
      sync_devices_.insert(_sptr_thread->device_id_);
    }
    std::string name = _sptr_thread->camera_model_.camera_name_;
    cv::Size shape = static_settings.project_shapes[name];
    current_images_[_sptr_thread->device_id] = std::make_shared<cv::Mat>(cv::Mat::zeros(shape.height,shape.width,3));
    _sptr_thread->proc_buffer_manager_ = shared_from_this();
  }
  std::map<int,cv::Mat> get () {
    return sptr_buffer_->get();
  }
  void set_image_for_device(int _device_id, cv::Mat _image) {
    if (sync_devices_.find(_device_id) != sync_devices_.end()) {
      current_images_[_device_id] = _image;
    } else {
      throw std::runtime_error("Device not held by the buffer: " + std::to_string(_device_id));
    }
  }
  void sync(int _device_id) {
    std::unique_lock<std::mutex> lock{mutex_};
    if (sync_devices_.find(_device_id) != sync_devices_.end()) {
      arrived_ += 1;
      if (arrived_ == sync_devices_.size()) {
        sptr_buffer_->add(current_images_, drop_if_full_);
        cv_.notify_all();
      } else {
        cv_.wait(lock);
      }
      arrived_ -= 1;
    }
  }
  void wake_all() {
    std::unique_lock<std::mutex> lock{mutex_};
    cv_.notify_all();
  }
  bool contains(int _device_id) {
    return sync_devices_.find(_device_id) != sync_devices_.end();
  }
  std::string str() {
    std::string devices;
    for (const int &item : sync_devices_) {
      devices += (", " + std::to_string(item));
    }
    return std::string(typeid(ProjectedImageBufferManager).name()) + ":\n" + \
           "devices: " + devices + "\n";
  }
  protected:
  bool drop_if_full_;
  std::shared_ptr<Buffer<std::map<int,cv::Mat>>> sptr_buffer_;
  std::unordered_set<int> sync_devices_;
  std::condition_variable cv_;
  std::mutex mutex_;
  int arrived_ = 0;
  std::map<int,cv::Mat> current_images_;
};
} //namespace SVS

#endif //SVS_IMAGEBUFFER_HPP