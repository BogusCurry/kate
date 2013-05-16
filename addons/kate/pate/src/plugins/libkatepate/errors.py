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

# This core originally was in this repository:
# <https://github.com/goinnn/Kate-plugins/blob/master/kate_plugins/pyte_plugins/check_plugins/commons.py>

import kate
import sys

from PyKDE4.kdecore import i18n
from PyKDE4.ktexteditor import KTextEditor


class PythonVersionException(Exception):
    pass


def needs_python_version(major, minor=None, micro=None, text=''):
    if major != sys.version_info.major:
        raise PythonVersionException(text)
    elif minor and minor != sys.version_info.minor:
        raise PythonVersionException(text)
    elif micro and micro != sys.version_info.micro:
        raise PythonVersionException(text)


def needs_packages(packages):
    msg = i18n("You need install the following packages:\n")
    import_error = False
    for package, version in packages.items():
        try:
            __import__(package)
        except ImportError:
            import_error = True
            if '==' in version:
                package = version.split('==')[0]
                version = version.split('==')[1]
            msg += i18n("\t%1. Use easy_install (or pip install) %1==%2", package, version)
    if import_error:
        raise ImportError(msg)


def clearMarksOfError(doc, mark_iface):
    for line in range(doc.lines()):
        if mark_iface.mark(line) == mark_iface.Error:
            mark_iface.removeMark(line, mark_iface.Error)


def hideOldPopUps():
    mainWindow = kate.mainWindow()
    popups = kate.gui.TimeoutPassivePopup.popups.get(mainWindow, []) or []
    for popup in popups:
        popup.timer.stop()
        popup.hide()
        popup.setFixedHeight(0)
        popup.adjustSize()
        popup.originalHeight = popup.height()
        popup.offsetBottom = 0
        popup.move(0, 0)


def showErrors(message, errors, key_mark, doc, time=10, icon='dialog-warning',
               key_line='line', key_column='column',
               max_errors=3, show_popup=True, move_cursor=False):
    mark_iface = doc.markInterface()
    messages = {}
    view = kate.activeView()
    pos = view.cursorPosition()
    current_line = pos.line() + 1
    current_column = pos.column() + 1
    for error in errors:
        header = False
        line = error[key_line]
        column = error.get(key_column, 0)
        pos = _compress_key(line, column)
        if not messages.get(line, None):
            header = True
            messages[pos] = []
        error_message = _generateErrorMessage(error, key_line, key_column, header)
        messages[pos].append(error_message)
        mark_iface.setMark(line - 1, mark_iface.Error)

    messages_items = list(messages.items())  # Python 3 compatible
    messages_items.sort()
    if move_cursor:
        first_error, messages_show = _getErrorMessagesOrder(messages_items,
                                                            max_errors,
                                                            current_line,
                                                            current_column)
        line_to_move, column_to_move = _uncompress_key(messages_items[first_error][0])
        _moveCursorTFirstError(line_to_move, column_to_move)
    else:
        first_error, messages_show = _getErrorMessagesOrder(messages_items, max_errors)
    if show_popup:
        message = '%s\n%s' % (message, '\n'.join(messages_show))
        if len(messages_show) < len(errors):
            message += i18n('\n\nAnd other errors')
        showError(message, time, icon, minTextWidth=300)


def showError(message="error", time=10, icon="dialog-warning", minTextWidth=300):
    kate.gui.popup(message, time, icon, minTextWidth=300)


def showOk(message="Ok", time=3, icon='dialog-ok'):
    kate.gui.popup(message, time, icon='dialog-ok', minTextWidth=200)


def _compress_key(line, column):
    doc = kate.activeDocument()
    cipher = len('%s' % doc.lines())
    key_template = '%%%sd' % cipher
    key_template += '__%3d'
    return key_template % (line, column)


def _uncompress_key(key):
    line, column = key.split('__')
    return (int(line), int(column))


def _generateErrorMessage(error, key_line='line', key_column='column', header=True):
    message = ''
    exclude_keys = [key_line, key_column, 'filename']
    line = error[key_line]
    column = error.get(key_column, None)
    if header or column:
        column = error.get(key_column, None)
        if column:
            message = i18n('~*~ Position: (%1, %2)', line, column)
        else:
            message = i18n('~*~ Line: %1', line)
        message += ' ~*~'
    for key, value in error.items():
        if value and key not in exclude_keys:
            if key != 'message':
                message = '%s\n     * %s: %s' % (message, key, value)
            else:
                message = '%s\n     %s' % (message, value)
    return message


def _getErrorMessagesOrder(messages_items, max_errors, current_line=None, current_column=None):
    messages_order = []
    first_error = None
    num_messages = 0
    if not current_line:
        for line, message in messages_items:
            messages_order.extend(message)
            num_messages += 1
            if num_messages >= max_errors:
                break
        return (0, messages_order)
    for i, error in enumerate(messages_items):
        line, column = _uncompress_key(error[0])
        message = error[1]
        if line > current_line or (line == current_line and column > current_column):
            if first_error is None:
                first_error = i
            num_messages += 1
            messages_order.extend(message)
        if num_messages >= max_errors:
            break
    if len(messages_order) == max_errors:
        return (first_error, messages_order)
    for i, error in enumerate(messages_items):
        line, column = _uncompress_key(error[0])
        message = error[1]
        if line <= current_line:
            if first_error is None:
                first_error = i
            num_messages += 1
            messages_order.extend(message)
        else:
            break
        if num_messages >= max_errors:
            break
    return (first_error, messages_order)


def _moveCursorTFirstError(line, column=None):
    try:
        column = column or 0
        cursor = KTextEditor.Cursor(line - 1, column - 1)
        view = kate.activeView()
        view.setCursorPosition(cursor)
    except KeyError:
        pass
