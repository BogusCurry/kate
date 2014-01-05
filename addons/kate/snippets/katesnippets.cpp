/*  This file is part of the Kate project.
 *
 *  Copyright (C) 2010 Christoph Cullmann <cullmann@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "katesnippets.h"
#include "snippetcompletionmodel.h"
#include "katesnippetglobal.h"

#include <QAction>

#include <KActionCollection>
#include <KXMLGUIFactory>
#include <KIconLoader>
#include <KToolBar>
#include <KLocalizedString>
#include <KPluginFactory>

#include <KTextEditor/CodeCompletionInterface>

K_PLUGIN_FACTORY_WITH_JSON (KateSnippetsPluginFactory, "katesnippetsplugin.json", registerPlugin<KateSnippetsPlugin>();)

KateSnippetsPlugin::KateSnippetsPlugin( QObject* parent, const QList<QVariant>& ):
    KTextEditor::ApplicationPlugin ( parent )
    , m_snippetGlobal (new KateSnippetGlobal (this))
{
}

KateSnippetsPlugin::~KateSnippetsPlugin()
{
}

QObject *KateSnippetsPlugin::createView (KTextEditor::MainWindow *mainWindow)
{
  KateSnippetsPluginView *view = new KateSnippetsPluginView (this, mainWindow);
  return view;
}

KateSnippetsPluginView::KateSnippetsPluginView (KateSnippetsPlugin* plugin, KTextEditor::MainWindow *mainWindow)
    : QObject (mainWindow), m_plugin(plugin), m_mainWindow(mainWindow), m_toolView (0), m_snippets(0)
{
  KXMLGUIClient::setComponentName ("katesnippets", i18n ("Snippets tool view"));
  setXMLFile( "ui.rc" );
  
  // Toolview for snippets
  m_toolView = mainWindow->createToolView (0,"kate_private_plugin_katesnippetsplugin", KTextEditor::MainWindow::Right, SmallIcon("document-new"), i18n("Snippets"));
  
  // snippets toolbar
  KToolBar *topToolbar = new KToolBar (m_toolView, "snippetsToolBar");
  topToolbar->setToolButtonStyle (Qt::ToolButtonIconOnly);
  topToolbar->addActions (m_snippets->actions());

  // add snippets widget
  m_snippets = KateSnippetGlobal::self()->snippetWidget();
  m_snippets->setParent (m_toolView);
  
  // register this view
  m_plugin->mViews.append ( this );
  
  // create actions
  QAction *a = actionCollection()->addAction( QLatin1String("tools_create_snippet") );
  a->setIcon (QIcon::fromTheme(QLatin1String("document-new")));
  a->setText( i18n("Create Snippet") );
  connect(a, SIGNAL(triggered(bool)), SLOT(createSnippet()));

  a = actionCollection()->addAction( QLatin1String("tools_snippets") );
  a->setText( i18n("Snippets...") );
  connect(a, SIGNAL(triggered(bool)), SLOT(showSnippetsDialog()));
  
  /**
   * connect for all already existing views
   */
  foreach (KTextEditor::View *view, mainWindow->views())
    slotViewCreated (view);

  m_mainWindow->guiFactory()->addClient (this);
}

KateSnippetsPluginView::~KateSnippetsPluginView ()
{
  // cleanup for all views
  foreach (QObject *view, m_textViews)
    if (auto iface = qobject_cast<KTextEditor::CodeCompletionInterface *> (view))
      iface->unregisterCompletionModel (KateSnippetGlobal::self()->completionModel());
  
  m_mainWindow->guiFactory()->removeClient (this);
  
  // unregister this view
  m_plugin->mViews.removeAll (this);
  
  // cleanup, kill toolview
  delete m_snippets;
  delete m_toolView;
}

void KateSnippetsPluginView::slotViewCreated (KTextEditor::View *view)
{
  /**
   * connect to destroyed
   */
  connect (view, SIGNAL(destroyed (QObject *)), this, SLOT(slotViewDestroyed (QObject *)));

  /**
   * remember for this view we need to cleanup!
   */
  m_textViews.insert (view);
  
  // add snippet completion
  if (auto iface = qobject_cast<KTextEditor::CodeCompletionInterface *> (view)) {
    iface->unregisterCompletionModel (KateSnippetGlobal::self()->completionModel());
    iface->registerCompletionModel (KateSnippetGlobal::self()->completionModel());  
  }
}

void KateSnippetsPluginView::slotViewDestroyed (QObject *view)
{
  /**
   * remove remembered views for which we need to cleanup on exit!
   */
  m_textViews.remove (view);
}

void KateSnippetsPluginView::createSnippet ()
{
  KateSnippetGlobal::self()->createSnippet (m_mainWindow->activeView());
}

void KateSnippetsPluginView::showSnippetsDialog ()
{
  KateSnippetGlobal::self()->showDialog (m_mainWindow->activeView());
}

#include "katesnippets.moc"

// kate: space-indent on; indent-width 2; replace-tabs on;
