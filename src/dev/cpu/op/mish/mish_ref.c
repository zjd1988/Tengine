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
 * Author: 942002795@qq.com
 */
#include <math.h>
#include "sys_port.h"
#include "module.h"
#include "tengine_errno.h"
#include "tengine_log.h"
#include "tengine_ir.h"
#include "../../cpu_node_ops.h"
#include "tengine_op.h"

int ref_mish_fp32(struct ir_tensor* input_tensor, struct ir_tensor* output_tensor, int num_thread)
{
    int w = input_tensor->dims[3];
    int h = output_tensor->dims[2];
    int channels = input_tensor->dims[1];
    int size = h * w;
    int c_step = h * w;

    float* input_data = input_tensor->data;
    float* out_data = output_tensor->data;

	int q = 0;
#pragma omp parallel for num_threads(num_thread)
    for (q = 0; q < channels; q++)
    {
        float* src = input_data + c_step * q;
        float* dst = out_data + c_step * q;

        for (int i = 0; i < size; i++)
        {
            dst[i] = src[i] * tanhf(log(1 + exp(src[i])));
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

static int run(struct node_ops* node_ops, struct exec_node* exec_node, struct exec_graph* exec_graph)
{
    struct ir_node* ir_node = exec_node->ir_node;
    struct ir_graph* ir_graph = ir_node->graph;
    struct ir_tensor* input_tensor;
    struct ir_tensor* output_tensor;

    input_tensor = get_ir_graph_tensor(ir_graph, ir_node->input_tensors[0]);
    output_tensor = get_ir_graph_tensor(ir_graph, ir_node->output_tensors[0]);

    ref_mish_fp32(input_tensor, output_tensor, exec_graph->num_thread);

    return 0;
}

static int reshape(struct node_ops* node_ops, struct exec_node* exec_node, struct exec_graph* exec_graph)
{
    struct ir_node* node = exec_node->ir_node;
    struct ir_graph* ir_graph = node->graph;
    struct ir_tensor* input = get_ir_graph_tensor(ir_graph, node->input_tensors[0]);
    struct ir_tensor* output = get_ir_graph_tensor(ir_graph, node->output_tensors[0]);

    int ret = set_ir_tensor_shape(output, input->dims, input->dim_num);
    return ret;
}

static int score(struct node_ops* node_ops, struct exec_graph* exec_graph, struct ir_node* exec_node)
{
    return OPS_SCORE_CANDO;
}

static struct node_ops hcl_node_ops = {.prerun = NULL,
                                       .run = run,
                                       .reshape = reshape,
                                       .postrun = NULL,
                                       .init_node = init_node,
                                       .release_node = release_node,
                                       .score = score};

static int reg_mish_hcl_ops(void* arg)
{
    return register_builtin_node_ops(OP_MISH, &hcl_node_ops);
}

static int unreg_mish_hcl_ops(void* arg)
{
    return unregister_builtin_node_ops(OP_MISH, &hcl_node_ops);
}

AUTO_REGISTER_OPS(reg_mish_hcl_ops);
AUTO_UNREGISTER_OPS(unreg_mish_hcl_ops);
