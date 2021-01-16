//
// Created by tobi on 12.01.21.
//
#include <arrow/api.h>
#include <arrow/tensor.h>
#include <arrow/buffer.h>
#include <arrow/ipc/api.h>
#include <arrow/io/api.h>
#include <zmqpp/zmqpp.hpp>
#include <iostream>
#include <random>

using namespace std;

shared_ptr<arrow::Buffer> create_and_serialize_tensor<typename T>(vector<int64_t> dims, vector<T> data) {
    auto buf = arrow::Buffer::Wrap<T>(data);

    // create a tensor
    auto result = arrow::Tensor::Make(arrow::float32(), buf, dims);
}

int main(int argc, char *argv[]) {
    while (1) {
        cout << "get new data..." << endl;
        vector<float> data = generate_data(524288);
        const vector<int64_t> dims = {2, 256, 512, 2};
        auto buf = arrow::Buffer::Wrap<float>(data);

        // create a tensor
        auto result = arrow::Tensor::Make(arrow::float32(), buf, dims);

        if (result.ok()) {
            auto tensor = result.ValueOrDie();

            auto result = tensor->CountNonZero();
            if (result.ok()) {
                cout << "created tensor with size: " << tensor->size() << " with " << result.ValueOrDie()
                     << " non-zero elements" << endl;
            }

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

                        cout << "tensor serialized meta: " << meta_len << " data: " << datasize << endl;


                        auto written_buffer_result = sink->Finish();

                        if (written_buffer_result.ok()) {
                            auto written_buffer = written_buffer_result.ValueOrDie();

                            cout << "written " << written_buffer->size() << " bytes" << endl;

                            zmqpp::message message;
                            message.add_raw(written_buffer->data(), written_buffer->size());

                            socket.send(message);
                        }
                    }
                }
            }
        }

        this_thread::sleep_for(chrono::seconds(1));
    }
}