<!--
#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#
-->

# Apache app_cage

## Overview

Apache app_cage is a skeleton for new Apache Mynewt projects.  The user downloads
this skeleton by issuing the "newt new" command (using Apache Newt).  Apache
app_cage also contains an example app and target for use with Apache Mynewt to
help you get started.

## Building

Apache app_cage contains an example Apache Mynewt application called app_cage.
When executed on suitably equipped hardware, this application repeatedly blinks
an LED.  The below procedure describes how to build this application for the
Apache Mynewt simulator.

1. Download and install Apache Newt.

You will need to download the Apache Newt tool, as documented in the [Getting Started Guide](https://mynewt.apache.org/latest/get_started/index.html).

2. Download the Apache Mynewt Core package (executed from the app_cage directory).

```no-highlight
    $ newt install
```

3. Build the app_cage app for the sim platform using the "my_app_cage_sim" target
(executed from the app_cage directory).

```no-highlight
    $ newt build my_app_cage_sim
```

The Apache Newt tool should indicate the location of the generated app_cage
executable.  Since the simulator does not have an LED to blink, this version of
app_cage is not terribly exciting - a printed message indicating the current LED
state.  To learn how to build app_cage for actual hardware, please see the
[Getting Started Guide](https://mynewt.apache.org/latest/get_started/index.html).
