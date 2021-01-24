//
// Created by tobi on 12.01.21.
//
#include <arrow/api.h>
#include <arrow/tensor.h>
#include <arrow/buffer.h>
#include <arrow/ipc/api.h>
#include <arrow/io/api.h>
#include <iostream>
#include <random>

#include "../include/arrow.h"

using namespace std;

shared_ptr<arrow::Buffer> create_and_serialize_tensor(const vector<int64_t>& dims, const vector<float>& data) {

    auto buf = arrow::Buffer::Wrap<float>(data);


    // create a tensor
    auto result = arrow::Tensor::Make(arrow::float32(), buf, dims);

    if (result.ok()) {
        auto tensor = result.ValueOrDie();

        // create serialized tensor
        auto sink_result = arrow::io::BufferOutputStream::Create();

        if (sink_result.ok()) {
            auto sink = sink_result.ValueOrDie();
            int32_t meta_len = 0;
            int64_t datasize;
            auto result = arrow::ipc::GetTensorSize(*tensor, &datasize);
            if (result.ok()) {
                auto written = arrow::ipc::WriteTensor(*tensor, sink.get(), &meta_len, &datasize);
                if (written.ok()) {

                    auto written_buffer_result = sink->Finish();

                    if (written_buffer_result.ok()) {
                        auto written_buffer = written_buffer_result.ValueOrDie();

                        return written_buffer;
                    }
                }
            }
        }
    }
    throw std::invalid_argument("cloud not write buffer");
}