
#include "tensorflow/lite/micro/kernels/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

/*
#include "tensorflow/lite/c/builtin_op_data.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/c/c_api_types.h"
*/

tflite::AllOpsResolver resolver;
tflite::Model *model;
const int tensor_arena_size = 2 * 1024;
uint8_t tensor_arena[tensor_arena_size];
tflite::MicroInterpreter interpreter;
TfLiteTensor *input, *output;

int initTF()
{
    //model = ::tflite::GetModel(g_model);
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        return -1;
    }

    interpreter = interpreter(model, resolver, tensor_arena, tensor_arena_size, NULL);
    interpreter.AllocateTensors();
    input = interpreter.input(0);
    return 0;
}

int *inferenceModel(uint8_t *inputValue, uint8_t inputSize, float *result)
{
    memcpy(input->data.f, inputValue, inputSize);
    TfLiteStatus invoke_status = interpreter.Invoke();
    if (invoke_status != kTfLiteOk)
    {
        return -1;
    }
    output = interpreter.output(0);
    result = output->data.f;
    return 0;
}