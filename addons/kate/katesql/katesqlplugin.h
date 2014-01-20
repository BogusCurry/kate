/*
   Copyright (C) 2010  Marco Mentasti  <marcomentasti@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KATESQLPLUGIN_H
#define KATESQLPLUGIN_H

#include <ktexteditor/view.h>
#include <ktexteditor/plugin.h>
#include <ktexteditor/application.h>
#include <ktexteditor/mainwindow.h>
#include <ktexteditor/configpageinterface.h>

#include <kpluginfactory.h>

class KateSQLPlugin : public KTextEditor::Plugin, public KTextEditor::ConfigPageInterface
{
  Q_OBJECT
  Q_INTERFACES(KTextEditor::ConfigPageInterface)

  public:
    explicit KateSQLPlugin(QObject* parent = 0, const QList<QVariant>& = QList<QVariant>());

    virtual ~KateSQLPlugin();

    QObject *createView(KTextEditor::MainWindow *mainWindow);

    // PluginConfigPageInterface

    int configPages() const { return 1; };
    KTextEditor::ConfigPage *configPage (int number = 0, QWidget *parent = 0);
    QString configPageName (int number = 0) const;
    QString configPageFullName (int number = 0) const;
    QIcon configPageIcon (int number = 0) const;

  Q_SIGNALS:
    void globalSettingsChanged();
};

#endif // KATESQLPLUGIN_H

