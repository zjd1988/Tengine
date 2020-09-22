/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * License); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * AS IS BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/*
 * Copyright (c) 2020, OPEN AI LAB
 * Author: qtang@openailab.com
 */

#include <math.h>
#include "sys_port.h"
#include "module.h"
#include "tengine_errno.h"
#include "tengine_log.h"
#include "tengine_ir.h"
#include "../../cpu_node_ops.h"
#include "tengine_op.h"

static int init_node(struct node_ops* node_ops, struct exec_node* exec_node, struct exec_graph* exec_graph)
{
    return 0;
}

static int release_node(struct node_ops* node_ops, struct exec_node* exec_node, struct exec_graph* exec_graph)
{
    return 0;
}

static int run(struct node_ops* node_ops, struct exec_node* exec_node, struct exec_graph* exec_graph)
{
    struct ir_node* ir_node = exec_node->ir_node;
    struct ir_graph* ir_graph = ir_node->graph;
    struct ir_tensor* input_tensor = get_ir_graph_tensor(ir_graph, ir_node->input_tensors[0]);
    struct ir_tensor* output_tensor = get_ir_graph_tensor(ir_graph, ir_node->output_tensors[0]);

    if (input_tensor->data == output_tensor->data)
        return 0;

    int size = 1;
    for (int i = 0; i < input_tensor->dim_num; i++)
        size *= input_tensor->dims[i];

    switch (input_tensor->data_type)
    {
        case TENGINE_DT_FP32:
        case TENGINE_DT_INT32: {
            size *= 4;
            break;
        }
        case TENGINE_DT_FP16:
        case TENGINE_DT_INT16: {
            size *= 2;
            break;
        }
        case TENGINE_DT_UINT8:
        case TENGINE_DT_INT8: {
            size *= 1;
            break;
        }
        default:
            set_tengine_errno(EINVAL);
            return -1;
    }

    if (size <= 0)
    {
        set_tengine_errno(EINVAL);
        return -1;
    }

    /* transpose nchw to nhwc */
    if (ir_graph->model_layout == TENGINE_LAYOUT_NHWC)
    {
        if (output_tensor->data_type == TENGINE_DT_FP32)
        {
            if (output_tensor->dim_num == 4)
            {
                int in_ch = input_tensor->dims[1];
                int in_h = input_tensor->dims[2];
                int in_w = input_tensor->dims[3];

                int out_ch = output_tensor->dims[1];
                int out_h = output_tensor->dims[2];
                int out_w = output_tensor->dims[3];

                float* input_fp32 = input_tensor->data;
                float* output_fp32 = output_tensor->data;
                float* data_fp32_temp = ( float* )malloc(size);

                int index = 0;
                for (int h = 0; h < in_h; h++)
                    for (int w = 0; w < in_w; w++)
                        for (int c = 0; c < in_ch; c++)
                            data_fp32_temp[index++] = input_fp32[c * in_h * in_w + h * in_w + w];

                /* transpose nhwc to nchw */
                index = 0;
                for (int c = 0; c < out_ch; c++)
                    for (int h = 0; h < out_h; h++)
                        for (int w = 0; w < out_w; w++)
                        {
                            output_fp32[index] = data_fp32_temp[h * out_w * out_ch + w * out_ch + c];
                            index++;
                        }

                free(data_fp32_temp);
                return 0;
            }
            else if (output_tensor->dim_num == 3)
            {
                int in_ch = input_tensor->dims[1];
                int in_h = input_tensor->dims[2];
                int in_w = input_tensor->dims[3];

                int out_ch = output_tensor->dims[1];
                int out_w = output_tensor->dims[2];

                float* input_fp32 = input_tensor->data;
                float* output_fp32 = output_tensor->data;
                float* data_fp32_temp = ( float* )malloc(size);

                int index = 0;
                for (int h = 0; h < in_h; h++)
                    for (int w = 0; w < in_w; w++)
                        for (int c = 0; c < in_ch; c++)
                            data_fp32_temp[index++] = input_fp32[c * in_h * in_w + h * in_w + w];

                /* transpose nhwc to nchw */
                index = 0;
                for (int c = 0; c < out_ch; c++)
                    for (int w = 0; w < out_w; w++)
                    {
                        output_fp32[index] = data_fp32_temp[w * out_ch + c];
                        index++;
                    }

                free(data_fp32_temp);
                return 0;
            }
        }
        else
        {
            if (output_tensor->dim_num == 4)
            {
                int in_ch = input_tensor->dims[1];
                int in_h = input_tensor->dims[2];
                int in_w = input_tensor->dims[3];

                int out_ch = output_tensor->dims[1];
                int out_h = output_tensor->dims[2];
                int out_w = output_tensor->dims[3];

                uint8_t* input_uint8 = input_tensor->data;
                uint8_t* output_uint8 = output_tensor->data;
                uint8_t* data_uint8_temp = ( uint8_t* )malloc(size);

                int index = 0;
                for (int h = 0; h < in_h; h++)
                    for (int w = 0; w < in_w; w++)
                        for (int c = 0; c < in_ch; c++)
                            data_uint8_temp[index++] = input_uint8[c * in_h * in_w + h * in_w + w];

                /* transpose nhwc to nchw */
                index = 0;
                for (int c = 0; c < out_ch; c++)
                    for (int h = 0; h < out_h; h++)
                        for (int w = 0; w < out_w; w++)
                        {
                            output_uint8[index] = data_uint8_temp[h * out_w * out_ch + w * out_ch + c];
                            index++;
                        }

                free(data_uint8_temp);
                return 0;
            }
            else if (output_tensor->dim_num == 3)
            {
                int in_ch = input_tensor->dims[1];
                int in_h = input_tensor->dims[2];
                int in_w = input_tensor->dims[3];

                int out_ch = output_tensor->dims[1];
                int out_w = output_tensor->dims[2];

                uint8_t* input_uint8 = input_tensor->data;
                uint8_t* output_uint8 = output_tensor->data;
                uint8_t* data_uint8_temp = ( uint8_t* )malloc(size);

                int index = 0;
                for (int h = 0; h < in_h; h++)
                    for (int w = 0; w < in_w; w++)
                        for (int c = 0; c < in_ch; c++)
                            data_uint8_temp[index++] = input_uint8[c * in_h * in_w + h * in_w + w];

                /* transpose nhwc to nchw */
                index = 0;
                for (int c = 0; c < out_ch; c++)
                    for (int w = 0; w < out_w; w++)
                    {
                        output_uint8[index] = data_uint8_temp[w * out_ch + c];
                        index++;
                    }

                free(data_uint8_temp);
                return 0;
            }
        }
    }

    /* another */
    memmove(output_tensor->data, input_tensor->data, size);

    return 0;
}

static int score(struct node_ops* node_ops, struct exec_graph* exec_graph, struct ir_node* exec_node)
{
    return OPS_SCORE_BEST;
}

static struct node_ops reshape_node_ops = {.prerun = NULL,
                                           .run = run,
                                           .reshape = NULL,
                                           .postrun = NULL,
                                           .init_node = init_node,
                                           .release_node = release_node,
                                           .score = score};

static int reg_reshape_ops(void* arg)
{
    return register_builtin_node_ops(OP_RESHAPE, &reshape_node_ops);
}

static int unreg_reshape_ops(void* arg)
{
    return unregister_builtin_node_ops(OP_RESHAPE, &reshape_node_ops);
}

AUTO_REGISTER_OPS(reg_reshape_ops);
AUTO_UNREGISTER_OPS(unreg_reshape_ops);
