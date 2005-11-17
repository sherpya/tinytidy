#!/usr/bin/env python
# -*- Mode: Python; tab-width: 4 -*-
#
# DistUtil Setup for TinyTidy module
# Copyright (C) 2005 Sherpya <sherpya@netfarm.it>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTIBILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
# ======================================================================
from distutils.core import setup, Extension

tinytidy = Extension('tinytidy',
                     sources = [ 'tinytidy.c' ],
                     libraries = [ 'tidy' ])

setup(name = 'TinyTidy', version = '0.1',
      description = 'tinytidy',
      ext_modules = [ tinytidy ])
