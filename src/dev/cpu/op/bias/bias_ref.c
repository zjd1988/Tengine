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
 * Author: chh@openailab.com
 */

#include "sys_port.h"
#include "module.h"
#include "tengine_errno.h"
#include "tengine_log.h"
#include "tengine_ir.h"
#include "../../cpu_node_ops.h"
#include "tengine_op.h"
#include <math.h>

int ref_bias_fp32(struct ir_tensor* input_tensor, struct ir_tensor* output_tensor, struct ir_tensor* bias_tensor,
                  int num_thread)
{
    int channels = input_tensor->dims[1];
    int h = input_tensor->dims[2];
    int w = input_tensor->dims[3];
    int size = h * w;

    float* in_data = input_tensor->data;
    float* bias = bias_tensor->data;
    float* out_data = output_tensor->data;

    int c = 0;
#pragma omp parallel for num_threads(num_thread)
    for (c = 0; c < channels; c++)
    {
        float* out_ptr = out_data + c * size;
        float* in_ptr = in_data + c * size;
        for (int i = 0; i < size; i++)
        {
            out_ptr[i] = in_ptr[i] + bias[c];
        }
    }

    return 0;
}

static int init_node(struct node_ops* node_ops, struct exec_node* exec_node, struct exec_graph* exec_graph)
{
    return 0;
}

static int release_node(struct node_ops* node_ops, struct exec_node* exec_node, struct exec_graph* exec_graph)
{
    return 0;
}

static int prerun(struct node_ops* node_ops, struct exec_node* exec_node, struct exec_graph* exec_graph)
{
    return 0;
}

static int run(struct node_ops* node_ops, struct exec_node* exec_node, struct exec_graph* exec_graph)
{
    struct ir_node* ir_node = exec_node->ir_node;
    struct ir_graph* ir_graph = ir_node->graph;
    struct ir_tensor* input_tensor;
    struct ir_tensor* bias_tensor;
    struct ir_tensor* output_tensor;
    int layout = ir_graph->graph_layout;

    input_tensor = get_ir_graph_tensor(ir_graph, ir_node->input_tensors[0]);
    bias_tensor = get_ir_graph_tensor(ir_graph, ir_node->input_tensors[1]);
    output_tensor = get_ir_graph_tensor(ir_graph, ir_node->output_tensors[0]);

    // inplace inference
    // if(input_tensor->data != output_tensor->data)
    // {
    //     TLOG_ERR("input and output are not the same mem\n");
    //     set_tengine_errno(EFAULT);
    //     return -1;
    // }

    int ret = ref_bias_fp32(input_tensor, output_tensor, bias_tensor, exec_graph->num_thread);
    if (ret != 0)
        return -1;

    return 0;
}

static int score(struct node_ops* node_ops, struct exec_graph* exec_graph, struct ir_node* exec_node)
{
    return OPS_SCORE_CANDO;
}

static struct node_ops hcl_node_ops = {.prerun = prerun,
                                       .run = run,
                                       .reshape = NULL,
                                       .postrun = NULL,
                                       .init_node = init_node,
                                       .release_node = release_node,
                                       .score = score};

static int reg_bias_hcl_ops(void* arg)
{
    return register_builtin_node_ops(OP_BIAS, &hcl_node_ops);
}

static int unreg_bias_hcl_ops(void* arg)
{
    return unregister_builtin_node_ops(OP_BIAS, &hcl_node_ops);
}

AUTO_REGISTER_OPS(reg_bias_hcl_ops);
AUTO_UNREGISTER_OPS(unreg_bias_hcl_ops);
