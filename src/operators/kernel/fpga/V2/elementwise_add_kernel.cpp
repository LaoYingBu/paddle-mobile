/* Copyright (c) 2018 PaddlePaddle Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */
#ifdef ELEMENTWISEADD_OP

#include "operators/kernel/elementwise_add_kernel.h"

#include <string>
#include "fpga/V2/api.h"

namespace paddle_mobile {
namespace operators {

template <>
bool ElementwiseAddKernel<FPGA, float>::Init(ElementwiseAddParam<FPGA> *param) {
  auto *input_y = const_cast<LoDTensor *>(param->InputY());
  auto *out = param->Out();
  paddle_mobile::fpga::ActivationType activation_enable =
      paddle_mobile::fpga::NONE;
  int16_t leaky_relu_negative_slope = 0;
  auto *input_x = const_cast<LoDTensor *>(param->InputX());
  auto input_x_ptr = input_x->data<int8_t>();
  auto input_y_ptr = input_y->data<int8_t>();
  fpga::format_ofm(out);
  auto out_ptr = out->mutable_data<int8_t>();
  float Si_1 = input_x->scale[0];
  float Si_2 = input_y->scale[0];
  float So = out->scale[0];
  float C1 = Si_1 / So;
  float C2 = Si_2 / So;
  fpga::EWAddArgs ewaddArgs = {0};
  ewaddArgs.output.activation.activation_type = activation_enable;
  ewaddArgs.output.activation.leaky_relu_negative_slope =
      leaky_relu_negative_slope;
  ewaddArgs.const0 = fpga::fp32_2_fp16(C1);
  ewaddArgs.const1 = fpga::fp32_2_fp16(C2);
  ewaddArgs.image0.address = input_x_ptr;
  ewaddArgs.image0.channels = (uint32_t)input_x->dims()[1];
  ewaddArgs.image0.scale_address = input_x->scale;
  ewaddArgs.image0.height = (uint32_t)input_x->dims()[2];
  ewaddArgs.image0.width = (uint32_t)input_x->dims()[3];
  ewaddArgs.image0.pad_height = 0;
  ewaddArgs.image0.pad_width = 0;
  ewaddArgs.image1.address = input_y_ptr;
  ewaddArgs.image1.channels = (uint32_t)input_y->dims()[1];
  ewaddArgs.image1.scale_address = input_y->scale;
  ewaddArgs.image1.height = (uint32_t)input_y->dims()[2];
  ewaddArgs.image1.width = (uint32_t)input_y->dims()[3];
  ewaddArgs.image1.pad_height = 0;
  ewaddArgs.image1.pad_width = 0;
  ewaddArgs.output.scale_address = out->scale;
  ewaddArgs.output.address = out_ptr;
  fpga::expand_EW_arg(&ewaddArgs);
  param->SetFpgaArgs(ewaddArgs);
  return true;
}

template <>
void ElementwiseAddKernel<FPGA, float>::Compute(
    const ElementwiseAddParam<FPGA> &param) {
  fpga::ComputeFpgaEWAdd(param.FpgaArgs());
}
}  // namespace operators
}  // namespace paddle_mobile

#endif
