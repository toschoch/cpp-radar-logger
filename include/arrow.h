//
// Created by tobi on 16.01.21.
//

#ifndef RADARREADER_ARROW_H
#define RADARREADER_ARROW_H

#include <arrow/tensor.h>
#include <arrow/buffer.h>

std::shared_ptr<arrow::Buffer> create_and_serialize_tensor(const std::vector<int64_t>& dims, const std::vector<float>& data);

#endif //RADARREADER_ARROW_H
