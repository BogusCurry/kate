# -*- coding: utf-8 -*-
# Copyright (c) 2013 by Pablo Martín <goinnn@gmail.com>
#
# This software is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this software.  If not, see <http://www.gnu.org/licenses/>.

# This file originally was in this repository:
# <https://github.com/goinnn/Kate-plugins/blob/master/kate_plugins/jste_plugins/jquery_plugins.py>

import kate

from libkatepate import text

from js_settings import (KATE_ACTIONS,
                         _ENABLE_TEXT_JQUERY,
                         DEFAULT_TEXT_JQUERY)


@kate.action(**KATE_ACTIONS['insertReady'])
def insertReady():
    """Snippet with the ready code of the jQuery"""
    js_utils_conf = kate.configuration.root.get('js_utils')
    text_jquery = js_utils_conf.get(_ENABLE_TEXT_JQUERY,
                                    DEFAULT_TEXT_JQUERY)
    text.insertText(text_jquery, start_in_current_column=True)

# kate: space-indent on; indent-width 4;
