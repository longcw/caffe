#include <cmath>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "gtest/gtest.h"

#include "caffe/blob.hpp"
#include "caffe/common.hpp"
#include "caffe/filler.hpp"
#include "caffe/layers/weighted_masking_layer.hpp"

#include "caffe/test/test_caffe_main.hpp"
#include "caffe/test/test_gradient_check_util.hpp"

namespace caffe {

template <typename TypeParam>
class WeightedMaskingLayerTest :
      public MultiDeviceTest<TypeParam> {
  typedef typename TypeParam::Dtype Dtype;

 protected:
  WeightedMaskingLayerTest()
      : blob_bottom_data_(new Blob<Dtype>(3, 4, 5, 3)),
        blob_bottom_scale_(new Blob<Dtype>(3, 1, 5, 3)),
        blob_top_(new Blob<Dtype>(3, 4, 5, 3)) {
    // Fill the data vector
    FillerParameter data_filler_param;
    data_filler_param.set_std(1);
    GaussianFiller<Dtype> data_filler(data_filler_param);
    data_filler.Fill(blob_bottom_data_);
    data_filler.Fill(blob_bottom_scale_);
    blob_bottom_vec_.push_back(blob_bottom_data_);
    blob_bottom_vec_.push_back(blob_bottom_scale_);
    blob_top_vec_.push_back(blob_top_);
  }
  virtual ~WeightedMaskingLayerTest() {
    delete blob_bottom_data_;
    delete blob_bottom_scale_;
    delete blob_top_;
  }

  Blob<Dtype>* const blob_bottom_data_;
  Blob<Dtype>* const blob_bottom_scale_;
  Blob<Dtype>* const blob_top_;
  vector<Blob<Dtype>*> blob_bottom_vec_;
  vector<Blob<Dtype>*> blob_top_vec_;
};

TYPED_TEST_CASE(WeightedMaskingLayerTest, TestDtypesAndDevices);

TYPED_TEST(WeightedMaskingLayerTest,
           TestForward) {
  typedef typename TypeParam::Dtype Dtype;
  LayerParameter layer_param;
  WeightedMaskingLayer<Dtype> layer(layer_param);
  layer.SetUp(this->blob_bottom_vec_, this->blob_top_vec_);
  layer.Forward(this->blob_bottom_vec_, this->blob_top_vec_);

  int num = this->blob_bottom_data_->num();
  int channel = this->blob_bottom_data_->channels();
  int height  = this->blob_bottom_data_->height();
  int width   = this->blob_bottom_data_->width();

  for (int n = 0; n < num; ++n) {
    const Dtype* scale = this->blob_bottom_scale_->cpu_data()+this->blob_bottom_scale_->offset(n,0,0,0);
    for (int c = 0; c < channel; ++c) {
      const Dtype* data = this->blob_bottom_data_->cpu_data()+this->blob_bottom_data_->offset(n,c,0,0);
      const Dtype* top_data = this->blob_top_->cpu_data()+this->blob_top_->offset(n,c,0,0);
      for (int i = 0; i < height * width; ++i) {
        EXPECT_EQ(top_data[i], data[i] * scale[i]);
      }
    }
  }
}

TYPED_TEST(WeightedMaskingLayerTest,
           TestGradient) {
  typedef typename TypeParam::Dtype Dtype;
  LayerParameter layer_param;
  WeightedMaskingLayer<Dtype> layer(layer_param);
  GradientChecker<Dtype> checker(1e-2, 1e-3);
  checker.CheckGradientExhaustive(&layer, this->blob_bottom_vec_,
                                  this->blob_top_vec_,1);
}

}  // namespace caffe