# Copyright 2006  Anthony Rowe and Adam Goode
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


include defs.mk


all: hal libs

hal:
	$(MAKE) -f hal.mk
	$(MAKE) -f hal.mk thumb=1

libs:
	$(MAKE) -C ../../lib
	$(MAKE) -C ../../lib thumb=1

clean:
	$(RM) *.a *.o
	$(MAKE) -C ../../lib clean
	$(MAKE) -C ../../lib clean thumb=1



.PHONY: clean all libs hal
