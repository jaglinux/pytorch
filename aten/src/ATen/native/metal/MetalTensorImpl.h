#ifndef MetalTensorImpl_h
#define MetalTensorImpl_h

#include <ATen/OpaqueTensorImpl.h>
#include <ATen/WrapDimUtils.h>
#import <ATen/native/metal/MetalTensorImplStorage.h>
#import <ATen/native/metal/mpscnn/MPSImageWrapper.h>

namespace at {
template <typename OpaqueHandle>
struct TORCH_API MetalTensorImpl : public OpaqueTensorImpl<OpaqueHandle> {
  MetalTensorImpl(
      at::DispatchKeySet key_set,
      const caffe2::TypeMeta& data_type,
      c10::Device device,
      OpaqueHandle opaque_handle,
      c10::IntArrayRef sizes,
      c10::IntArrayRef strides)
      : OpaqueTensorImpl<OpaqueHandle>(
            key_set,
            data_type,
            device,
            opaque_handle,
            sizes),
        strides_(strides.vec()) {
    TensorImpl::set_has_contiguity_policy(TensorImpl::HasContiguityPolicy::CustomBehavior);
  }

  IntArrayRef strides() const override {
    return strides_;
  }

  bool is_contiguous_custom(c10::MemoryFormat memory_format) const override {
    return true;
  }

  int64_t stride(int64_t d) const override {
    d = at::maybe_wrap_dim(d, this->dim(), false);
    return strides_[d];
  }

  void release_resources() override {
    using MetalTensorImplStorage = at::native::metal::MetalTensorImplStorage;
    auto&& handle = (MetalTensorImplStorage)this->opaque_handle();
    handle.texture()->recycleImage();
    OpaqueTensorImpl<OpaqueHandle>::release_resources();
  }

 private:
  const char* tensorimpl_type_name() const override {
    return "MetalTensorImpl";
  }

  SmallVector<int64_t, 5> strides_;
};
} // namespace at

#endif /* MetalTensorImpl_h*/
