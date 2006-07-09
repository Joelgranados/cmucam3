# Copyright 2006  Anthony Rowe and Adam Goode
#
# This file is part of cc3.
#
# cc3 is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# cc3 is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with cc3; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


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
