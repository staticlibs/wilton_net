/*
 * Copyright 2017, alex at staticlibs.net
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* 
 * File:   wilton_net.h
 * Author: alex
 *
 * Created on October 17, 2017, 8:57 PM
 */

#ifndef WILTON_NET_H
#define WILTON_NET_H

#include "wilton/wilton.h"

#ifdef __cplusplus
extern "C" {
#endif

char* wilton_net_wait_for_tcp_connection(
        const char* ip_addr,
        int ip_addr_len,
        int tcp_port,
        int timeout_millis);

#ifdef __cplusplus
}
#endif

#endif /* WILTON_NET_H */

