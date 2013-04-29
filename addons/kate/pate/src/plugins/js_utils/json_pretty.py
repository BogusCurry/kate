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

# This code is based in this other:
# <http://www.muhuk.com/2008/11/extending-kate-with-pate/>
# This file originally was in this repository:
# <https://github.com/goinnn/Kate-plugins/blob/master/kate_plugins/jste_plugins/json_plugins.py>

import kate
import sys
try:
    import simplejson as json
    LIB_JSON = 'simplejson'
except ImportError:
    import json
    LIB_JSON = 'standard'

from PyKDE4.kdecore import i18n

from libkatepate import text
from libkatepate.errors import showError
from js_settings import (KATE_ACTIONS,
                         _INDENT_JSON_CFG,
                         _ENCODING_JSON_CFG,
                         DEFAULT_INDENT_JSON,
                         DEFAULT_ENCODING_JSON)


@kate.action(**KATE_ACTIONS['togglePrettyJsonFormat'])
def togglePrettyJsonFormat():
    """A simple JSON pretty printer. JSON formatter which a good indents"""
    currentDocument = kate.activeDocument()
    view = currentDocument.activeView()
    source = view.selectionText()
    js_utils_conf = kate.configuration.root.get('js_utils', {})
    if not source:
        showError(i18n('Please select a json text and press: %s') % KATE_ACTIONS['togglePrettyJsonFormat']['shortcut'])
    else:
        indent = js_utils_conf.get(_INDENT_JSON_CFG, DEFAULT_INDENT_JSON)
        encoding = js_utils_conf.get(_ENCODING_JSON_CFG, DEFAULT_ENCODING_JSON)
        try:
            if LIB_JSON == 'simplejson' or sys.version_info.major == 2:
                target = json.dumps(json.loads(source),
                                    indent=indent,
                                    encoding=encoding)
            else:
                target = json.dumps(json.loads(source),
                                    indent=indent)
            view.removeSelectionText()
            text.insertText(target)
        except ValueError as e:
            showError(i18n('This selected text is not a valid json text: %s') % e.message)

# kate: space-indent on; indent-width 4;
