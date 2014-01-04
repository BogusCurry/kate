/* This file is part of the KDE project
   Copyright (C) 2010 Thomas Fjellstrom <thomas@fjellstrom.ca>

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

#include "katefiletreemodel.h"

#include <QDir>
#include <QFileInfo>
#include <QList>
#include <QMimeDatabase>
#include <QIcon>

#include <KColorScheme>
#include <KColorUtils>
#include <KLocalizedString>
#include <kiconutils.h>

#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/application.h>

#include "katefiletreedebug.h"

class ProxyItemDir;
class ProxyItem {
  friend class KateFileTreeModel;

  public:
    enum Flag { None = 0, Dir = 1, Modified = 2, ModifiedExternally = 4, DeletedExternally = 8, Empty = 16, ShowFullPath = 32, Host=64 };
    Q_DECLARE_FLAGS(Flags, Flag)

    ProxyItem(QString n, ProxyItemDir *p = 0, Flags f = ProxyItem::None);
    ~ProxyItem();

    int addChild(ProxyItem *p);
    void remChild(ProxyItem *p);

    ProxyItemDir *parent();

    ProxyItem *child(int idx);
    int childCount();

    int row();

    QString display();
    QString path();
    QString documentName();
    void setPath(const QString &str);

    void setIcon(QIcon i);
    QIcon icon();

    QList<ProxyItem*> &children();

    void setDoc(KTextEditor::Document *doc);
    KTextEditor::Document *doc();
    QList<KTextEditor::Document*> docTree() const;

    void setFlags(Flags flags);
    void setFlag(Flag flag);
    void clearFlag(Flag flag);
    bool flag(Flag flag);
    void setHost(const QString &host);
    const QString& host() const;

  private:
    QString m_path;
    QString m_documentName;
    ProxyItemDir *m_parent;
    QList<ProxyItem*> m_children;
    int m_row;
    Flags m_flags;

    QString m_display;
    QIcon m_icon;
    KTextEditor::Document *m_doc;
    QString m_host;
  protected:
    void initDisplay();
};

QDebug operator<<(QDebug dbg, ProxyItem *item)
{
  if(!item) {
    dbg.nospace() << "ProxyItem(0x0) ";
    return dbg.maybeSpace();
  }

  void *parent = static_cast<void *>(item->parent());

  dbg.nospace() << "ProxyItem(" << (void*)item << ",";
  dbg.nospace() << parent << "," << item->row() << ",";
  dbg.nospace() << item->doc() << "," << item->path() << ") ";
  return dbg.maybeSpace();
}


class ProxyItemDir : public ProxyItem
{
  public:
    ProxyItemDir(QString n, ProxyItemDir *p = 0) : ProxyItem(n, p) { setFlag(ProxyItem::Dir); initDisplay();}
};

QDebug operator<<(QDebug dbg, ProxyItemDir *item)
{
  if(!item) {
    dbg.nospace() << "ProxyItemDir(0x0) ";
    return dbg.maybeSpace();
  }

  void *parent = static_cast<void *>(item->parent());

  dbg.nospace() << "ProxyItemDir(" << (void*)item << ",";
  dbg.nospace() << parent << "," << item->row() << ",";
  dbg.nospace() << item->path() << ", children:" << item->childCount() << ") ";
  return dbg.maybeSpace();
}

Q_DECLARE_OPERATORS_FOR_FLAGS(ProxyItem::Flags)

ProxyItem::ProxyItem(QString d, ProxyItemDir *p, ProxyItem::Flags f)
  : m_path(d), m_parent(p), m_row(-1), m_flags(f), m_doc(0)
{
  initDisplay();

  if(p)
    p->addChild(this);
}

ProxyItem::~ProxyItem()
{
  foreach(ProxyItem *item, m_children) {
    delete item;
  }
}

void ProxyItem::initDisplay()
{
  // triggers only if this is a top level node and the root has the show full path flag set.
  if (flag(ProxyItem::Dir) && m_parent && !m_parent->m_parent && m_parent->flag(ProxyItem::ShowFullPath)) {
    m_display = m_path;
    if(m_display.startsWith(QDir::homePath())) {
      m_display.replace(0, QDir::homePath().length(), "~");
    }
  } else {
    m_display = m_path.section(QLatin1Char('/'), -1, -1);
    if (flag(ProxyItem::Host) && (!m_parent || (m_parent && !m_parent->m_parent))) {
      QString hostPrefix="["+host()+"]";
      if (hostPrefix!=m_display)
        m_display=hostPrefix+m_display;
    }
  }

}

int ProxyItem::addChild(ProxyItem *item)
{
  int item_row = m_children.count();
  item->m_row = item_row;
  m_children.append(item);
  item->m_parent = static_cast<ProxyItemDir*>(this);

  item->initDisplay();

  return item_row;
}

void ProxyItem::remChild(ProxyItem *item)
{
  m_children.removeOne(item);
  // fix up item rows
  // could be done a little better, but this'll work.
  for(int i = 0; i < m_children.count(); i++) {
    m_children[i]->m_row = i;
  }

  item->m_parent = 0;
}

ProxyItemDir *ProxyItem::parent()
{
  return m_parent;
}

ProxyItem *ProxyItem::child(int idx)
{
  if(idx < 0 || idx >= m_children.count()) return 0;
  return m_children[idx];
}

int ProxyItem::childCount()
{
  return m_children.count();
}

int ProxyItem::row()
{
  return m_row;
}

QIcon ProxyItem::icon()
{
  if(m_children.count())
    return QIcon::fromTheme(QLatin1String("folder"));

  return m_icon;
}

void ProxyItem::setIcon(QIcon i)
{
  m_icon = i;
}

QString ProxyItem::documentName() {
  return m_documentName;
}

QString ProxyItem::display()
{
  return m_display;
}

QString ProxyItem::path()
{
  return m_path;
}

void ProxyItem::setPath(const QString &p)
{
  m_path = p;
  initDisplay();
}

QList<ProxyItem*> &ProxyItem::children()
{
  return m_children;
}

void ProxyItem::setDoc(KTextEditor::Document *doc)
{
  m_doc = doc;
  if (!doc)
    m_documentName=QString();
  else {
        QString docName=doc->documentName();
        if (flag(ProxyItem::Host))
          m_documentName="["+m_host+"]"+docName;
        else
          m_documentName=docName;
  }
}

KTextEditor::Document *ProxyItem::doc()
{
  return m_doc;
}

QList<KTextEditor::Document*> ProxyItem::docTree() const
{
  QList<KTextEditor::Document*> result;
  if (m_doc) {
    result.append(m_doc);
  }
  for (QList<ProxyItem*>::const_iterator iter = m_children.constBegin(); iter != m_children.constEnd(); ++iter) {
    result.append((*iter)->docTree());
  }
  return result;
}

bool ProxyItem::flag(Flag f)
{
  return m_flags & f;
}

void ProxyItem::setFlag(Flag f)
{
  m_flags |= f;
}

void ProxyItem::setFlags(Flags f)
{
  m_flags = f;
}

void ProxyItem::clearFlag(Flag f)
{
  m_flags &= ~f;
}

void ProxyItem::setHost(const QString& host)
{
  QString docName;
  if (m_doc)
    docName=m_doc->documentName();
  if (host.isEmpty()) {
    clearFlag(Host);
    m_documentName=docName;
  } else {
    setFlag(Host);
    m_documentName="["+host+"]"+docName;
  }
  m_host=host;

  initDisplay();
}

const QString& ProxyItem::host() const
{
  return m_host;
}

KateFileTreeModel::KateFileTreeModel(QObject *p)
  : QAbstractItemModel(p),
    m_root(new ProxyItemDir(QString("m_root"), 0))
{

  // setup default settings
  // session init will set these all soon
  KColorScheme colors(QPalette::Active);
  QColor bg = colors.background().color();
  m_editShade = KColorUtils::tint(bg, colors.foreground(KColorScheme::ActiveText).color(), 0.5);
  m_viewShade = KColorUtils::tint(bg, colors.foreground(KColorScheme::VisitedText).color(), 0.5);
  m_shadingEnabled = true;
  m_listMode = false;

  initModel();
}

KateFileTreeModel::~KateFileTreeModel()
{

}

bool KateFileTreeModel::shadingEnabled() const
{
  return m_shadingEnabled;
}

void KateFileTreeModel::setShadingEnabled(bool se)
{
  if(m_shadingEnabled != se) {
    updateBackgrounds(true);
    m_shadingEnabled = se;
  }
}

QColor KateFileTreeModel::editShade() const
{
  return m_editShade;
}

void KateFileTreeModel::setEditShade(QColor es)
{
  m_editShade = es;
}

QColor KateFileTreeModel::viewShade() const
{
  return m_viewShade;
}

void KateFileTreeModel::setViewShade(QColor vs)
{
  m_viewShade = vs;
}

bool KateFileTreeModel::showFullPathOnRoots(void)
{
  return m_root->flag(ProxyItem::ShowFullPath);
}

void KateFileTreeModel::setShowFullPathOnRoots(bool s)
{
  if(s)
    m_root->setFlag(ProxyItem::ShowFullPath);
  else
    m_root->clearFlag(ProxyItem::ShowFullPath);

  foreach(ProxyItem *root, m_root->children()) {
    root->initDisplay();
  }
}

// FIXME: optimize this later to insert all at once if possible
// maybe add a "bool emitSignals" to documentOpened
// and possibly use beginResetModel here? I dunno.

void KateFileTreeModel::initModel()
{
  // add already existing documents
  foreach( KTextEditor::Document* doc, KTextEditor::Editor::instance()->application()->documents() )
    documentOpened( doc );
}

void KateFileTreeModel::clearModel()
{
  // remove all items
  // can safely ignore documentClosed here

  beginRemoveRows(QModelIndex(), 0, m_root->childCount()-1);

  delete m_root;
  m_root = new ProxyItemDir(QString("m_root"), 0);

  m_docmap.clear();
  m_viewHistory.clear();
  m_editHistory.clear();
  m_brushes.clear();

  endRemoveRows();
}

void KateFileTreeModel::connectDocument(const KTextEditor::Document *doc)
{
  connect(doc, SIGNAL(documentNameChanged(KTextEditor::Document*)), this, SLOT(documentNameChanged(KTextEditor::Document*)));
  connect(doc, SIGNAL(documentUrlChanged(KTextEditor::Document*)), this, SLOT(documentNameChanged(KTextEditor::Document*)));
  connect(doc, SIGNAL(modifiedChanged(KTextEditor::Document*)), this, SLOT(documentModifiedChanged(KTextEditor::Document*)));
  connect(doc, SIGNAL(modifiedOnDisk(KTextEditor::Document*,bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)),
          this,  SLOT(documentModifiedOnDisc(KTextEditor::Document*,bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)) );
}

QModelIndex KateFileTreeModel::docIndex(KTextEditor::Document *d) const
{
  ProxyItem *item = m_docmap[d];
  if(!item) {
    return QModelIndex();
  }

  return createIndex(item->row(), 0, item);
}

Qt::ItemFlags KateFileTreeModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags flags = Qt::ItemIsEnabled;

  if(!index.isValid())
    return 0;

  ProxyItem *item = static_cast<ProxyItem*>(index.internalPointer());
  if(item && !item->childCount()) {
    flags |= Qt::ItemIsSelectable;
  }

  return flags;
}

#include "metatype_qlist_ktexteditor_document_pointer.h"

QVariant KateFileTreeModel::data( const QModelIndex &index, int role ) const
{
  if(!index.isValid()) {
    return QVariant();
  }

  ProxyItem *item = static_cast<ProxyItem *>(index.internalPointer());
  if(!item) {
    return QVariant();
  }

  switch(role) {
    case KateFileTreeModel::PathRole:
      // allow to sort with hostname + path, bug 271488
      return (item->doc() && !item->doc()->url().isEmpty()) ? item->doc()->url().toString() : item->path();

    case KateFileTreeModel::DocumentRole:
      return QVariant::fromValue(item->doc());

    case KateFileTreeModel::OpeningOrderRole:
      return item->row();

    case KateFileTreeModel::DocumentTreeRole:
      return QVariant::fromValue(item->docTree());

    case Qt::DisplayRole:
      // in list mode we want to use kate's fancy names.
      if(m_listMode) {
        return item->documentName();
      } else
        return item->display();

    case Qt::DecorationRole:
      return item->icon();

    case Qt::ToolTipRole: {
      QString tooltip = item->path();
      if (item->flag(ProxyItem::DeletedExternally) || item->flag(ProxyItem::ModifiedExternally)) {
        tooltip = i18nc("%1 is the full path", "<p><b>%1</b></p><p>The document has been modified by another application.</p>").arg(item->path());
      }

      return tooltip;
    }

    case Qt::ForegroundRole: {
      KColorScheme colors(QPalette::Active);
      if(!item->flag(ProxyItem::Dir) && (!item->doc() || item->doc()->openingError())) return colors.foreground(KColorScheme::InactiveText).color();
    } break;

    case Qt::BackgroundRole:
      // TODO: do that funky shading the file list does...
      if(m_shadingEnabled && m_brushes.contains(item))
        return m_brushes[item];
      break;
  }

  return QVariant();
}

QVariant KateFileTreeModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  Q_UNUSED(orientation);
  Q_UNUSED(role);

  if(section == 0)
    return QString("a header");

  return QVariant();
}

int KateFileTreeModel::rowCount( const QModelIndex &parent ) const
{
  if(!parent.isValid())
    return m_root->childCount();

  ProxyItem *item = static_cast<ProxyItem *>(parent.internalPointer());
  if(!item) {
    return 0;
  }

  return item->childCount();
}

int KateFileTreeModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED(parent);
  return 1;
}


QModelIndex KateFileTreeModel::parent( const QModelIndex &index ) const
{
  if(!index.isValid()) {
    return QModelIndex();
  }

  ProxyItem *item = static_cast<ProxyItem *>(index.internalPointer());
  if(!item) {
    return QModelIndex();
  }

  if(!item->parent()) {
    return QModelIndex();
  }

  if(item->parent() == m_root)
    return QModelIndex();

  return createIndex(item->parent()->row(), 0, item->parent());
}

QModelIndex KateFileTreeModel::index( int row, int column, const QModelIndex &parent ) const
{
  ProxyItem *p = 0;
  if(column != 0) {
    return QModelIndex();
  }

  if(!parent.isValid())
    p = m_root;
  else
    p = static_cast<ProxyItem *>(parent.internalPointer());

  if(!p) {
    return QModelIndex();
  }

  if(row < 0 || row >= p->childCount()) {
    return QModelIndex();
  }

  return createIndex(row, 0, p->child(row));
}

bool KateFileTreeModel::hasChildren( const QModelIndex & parent ) const
{
  if(!parent.isValid())
    return m_root->childCount() > 0;

  ProxyItem *item = static_cast<ProxyItem*>(parent.internalPointer());
  if(!item) {
    return false;
  }

  return item->childCount() > 0;
}

bool KateFileTreeModel::isDir(const QModelIndex &index) const
{
  if(!index.isValid())
    return true;

  ProxyItem *item = static_cast<ProxyItem*>(index.internalPointer());
  if(!item) {
    return false;
  }

  return item->flag(ProxyItem::Dir);
}

bool KateFileTreeModel::listMode() const
{
  return m_listMode;
}

void KateFileTreeModel::setListMode(bool lm)
{
  if(lm != m_listMode) {
    m_listMode = lm;

    clearModel();
    initModel();
  }
}

void KateFileTreeModel::documentOpened(KTextEditor::Document *doc)
{
  QString path = doc->url().path();
  bool isEmpty = false;
  QString host;

  if(doc->url().isEmpty()) {
    path = doc->documentName();
    isEmpty = true;
  } else {
    host=doc->url().host();
    if (!host.isEmpty())
      path="["+host+"]"+path;

  }

  ProxyItem *item = new ProxyItem(path, 0);

  if(isEmpty)
    item->setFlag(ProxyItem::Empty);

  item->setDoc(doc);
  item->setHost(host);
  setupIcon(item);
  handleInsert(item);
  m_docmap[doc] = item;
  connectDocument(doc);

}

void KateFileTreeModel::documentsOpened(const QList<KTextEditor::Document*> &docs)
{
  beginResetModel();

  foreach(KTextEditor::Document *doc, docs) {
    if (m_docmap.contains(doc)) {
      documentNameChanged(doc);
    } else {
      documentOpened(doc);
    }
  }

  endResetModel();
}

void KateFileTreeModel::documentModifiedChanged(KTextEditor::Document *doc)
{
  ProxyItem *item = m_docmap[doc];
  if(!item)
    return;

  if(doc->isModified()) {
    item->setFlag(ProxyItem::Modified);
  }
  else {
    item->clearFlag(ProxyItem::Modified);
    item->clearFlag(ProxyItem::ModifiedExternally);
    item->clearFlag(ProxyItem::DeletedExternally);
  }

  setupIcon(item);

  QModelIndex idx = createIndex(item->row(), 0, item);
  emit dataChanged(idx, idx);
}

void KateFileTreeModel::documentModifiedOnDisc(KTextEditor::Document *doc, bool modified, KTextEditor::ModificationInterface::ModifiedOnDiskReason reason )
{
  Q_UNUSED(modified);
  ProxyItem *item = m_docmap[doc];
  if(!item)
    return;

  // This didn't do what I thought it did, on an ignore
  // we'd get !modified causing the warning icons to disappear
  if(!modified) {
    item->clearFlag(ProxyItem::ModifiedExternally);
    item->clearFlag(ProxyItem::DeletedExternally);
  } else {
    if(reason == KTextEditor::ModificationInterface::OnDiskDeleted) {
      item->setFlag(ProxyItem::DeletedExternally);
    }
    else if(reason == KTextEditor::ModificationInterface::OnDiskModified) {
      item->setFlag(ProxyItem::ModifiedExternally);
    }
    else if(reason == KTextEditor::ModificationInterface::OnDiskCreated) {
      // with out this, on "reload" we don't get the icons removed :(
      item->clearFlag(ProxyItem::ModifiedExternally);
      item->clearFlag(ProxyItem::DeletedExternally);
    }
  }

  setupIcon(item);

  QModelIndex idx = createIndex(item->row(), 0, item);
  emit dataChanged(idx, idx);
}

void KateFileTreeModel::documentActivated(KTextEditor::Document *doc)
{
  if(!m_docmap.contains(doc)) {
    return;
  }

  ProxyItem *item = m_docmap[doc];
  m_viewHistory.removeAll(item);
  m_viewHistory.prepend(item);

  while (m_viewHistory.count() > 10) m_viewHistory.removeLast();

  updateBackgrounds();
}

void KateFileTreeModel::documentEdited(KTextEditor::Document *doc)
{
  if(!m_docmap.contains(doc)) {
    return;
  }

  ProxyItem *item = m_docmap[doc];
  m_editHistory.removeAll(item);
  m_editHistory.prepend(item);
  while (m_editHistory.count() > 10) m_editHistory.removeLast();

  updateBackgrounds();
}

void KateFileTreeModel::slotAboutToDeleteDocuments(const QList<KTextEditor::Document*> &docs)
{
  foreach (const KTextEditor::Document *doc, docs) {
    disconnect(doc, SIGNAL(documentNameChanged(KTextEditor::Document*)), this, SLOT(documentNameChanged(KTextEditor::Document*)));
    disconnect(doc, SIGNAL(documentUrlChanged(KTextEditor::Document*)), this, SLOT(documentNameChanged(KTextEditor::Document*)));
    disconnect(doc, SIGNAL(modifiedChanged(KTextEditor::Document*)), this, SLOT(documentModifiedChanged(KTextEditor::Document*)));
    disconnect(doc, SIGNAL(modifiedOnDisk(KTextEditor::Document*,bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)),
               this,  SLOT(documentModifiedOnDisc(KTextEditor::Document*,bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)) );
  }
}

void KateFileTreeModel::slotDocumentsDeleted(const QList<KTextEditor::Document*> &docs)
{
  foreach (const KTextEditor::Document *doc, docs) {
    connectDocument(doc);
  }
}

class EditViewCount
{
  public:
    EditViewCount(): edit(0), view(0)
    {}
    int edit;
    int view;
};

void KateFileTreeModel::updateBackgrounds(bool force)
{
  if (!m_shadingEnabled && !force) return;

  QMap <ProxyItem *, EditViewCount> helper;
  int i = 1;

  foreach (ProxyItem *item, m_viewHistory) {
    helper[item].view = i;
    i++;
  }

  i = 1;
  foreach (ProxyItem *item, m_editHistory) {
    helper[item].edit = i;
    i++;
  }

  QMap<ProxyItem *, QBrush> oldBrushes = m_brushes;
  m_brushes.clear();

  int hc = m_viewHistory.count();
  int ec = m_editHistory.count();

  for (QMap<ProxyItem *, EditViewCount>::iterator it = helper.begin();it != helper.end();++it)
  {
    QColor shade( m_viewShade );
    QColor eshade( m_editShade );

    if (it.value().edit > 0)
    {
      int v = hc - it.value().view;
      int e = ec - it.value().edit + 1;

      e = e * e;

      int n = qMax(v + e, 1);

      shade.setRgb(
        ((shade.red()*v) + (eshade.red()*e)) / n,
        ((shade.green()*v) + (eshade.green()*e)) / n,
        ((shade.blue()*v) + (eshade.blue()*e)) / n
      );
    }

    // blend in the shade color; latest is most colored.
    double t = double(hc - it.value().view + 1) / double(hc);

    m_brushes[it.key()] = QBrush(KColorUtils::mix(QPalette().color(QPalette::Base), shade, t));
  }

  foreach(ProxyItem *item, m_brushes.keys())
  {
    oldBrushes.remove(item);
    QModelIndex idx = createIndex(item->row(), 0, item);
    dataChanged(idx, idx);
  }

  foreach(ProxyItem *item, oldBrushes.keys())
  {
    QModelIndex idx = createIndex(item->row(), 0, item);
    dataChanged(idx, idx);
  }
}

void KateFileTreeModel::handleEmptyParents(ProxyItemDir *item)
{
  Q_ASSERT(item != 0);

  if(!item || !item->parent()) {
    return;
  }

  ProxyItemDir *parent = item->parent();
  //emit layoutAboutToBeChanged();

  while(parent) {
    if(!item->childCount()) {
      QModelIndex parent_index = parent == m_root ? QModelIndex() : createIndex(parent->row(), 0, parent);
      beginRemoveRows(parent_index, item->row(), item->row());
      parent->remChild(item);
      endRemoveRows();
      delete item;
    }
    else {
      // breakout early, if this node isn't empty, theres no use in checking its parents
      return;
    }

    item = parent;
    parent = item->parent();
  }

  //emit layoutChanged();
}

void KateFileTreeModel::documentClosed(KTextEditor::Document *doc)
{
  QString path = doc->url().path();

  if(!m_docmap.contains(doc)) {
    return;
  }

  if(m_shadingEnabled) {
    ProxyItem *toRemove = m_docmap[doc];
    if(m_brushes.contains(toRemove)) {
      m_brushes.remove(toRemove);
    }

    if(m_viewHistory.contains(toRemove)) {
      m_viewHistory.removeAll(toRemove);
    }

    if(m_editHistory.contains(toRemove)) {
      m_editHistory.removeAll(toRemove);
    }
  }

  ProxyItem *node = m_docmap[doc];
  ProxyItemDir *parent = node->parent();

  QModelIndex parent_index = parent == m_root ? QModelIndex() : createIndex(parent->row(), 0, parent);
  beginRemoveRows(parent_index, node->row(), node->row());
    node->parent()->remChild(node);
  endRemoveRows();

  delete node;
  handleEmptyParents(parent);

  m_docmap.remove(doc);
}

void KateFileTreeModel::documentNameChanged(KTextEditor::Document *doc)
{
  if(!m_docmap.contains(doc)) {
    return;
  }

  ProxyItem *item = m_docmap[doc];
  QString path = doc->url().path();
  QString host;
  if(doc->url().isEmpty()) {
    path = doc->documentName();
    item->setFlag(ProxyItem::Empty);
  }
  else {
    item->clearFlag(ProxyItem::Empty);
    host=doc->url().host();
    if (!host.isEmpty())
      path="["+host+"]"+path;
  }

  if(m_shadingEnabled) {
    ProxyItem *toRemove = m_docmap[doc];
    if(m_brushes.contains(toRemove)) {
      QBrush brush=m_brushes[toRemove];
      m_brushes.remove(toRemove);
      m_brushes.insert(item,brush);
    }

    if(m_viewHistory.contains(toRemove)) {
      int idx=m_viewHistory.indexOf(toRemove);
      if (idx!=-1)
        m_viewHistory.replace(idx,item);
    }

    if(m_editHistory.contains(toRemove)) {
      int idx=m_editHistory.indexOf(toRemove);
      if (idx!=-1)
        m_editHistory.replace(idx,item);
    }
  }

  handleNameChange(item, path, host);

  triggerViewChangeAfterNameChange();
}

ProxyItemDir *KateFileTreeModel::findRootNode(const QString &name, int r)
{
  QString base = name.section(QLatin1Char('/'), 0, -2);
  foreach(ProxyItem *item, m_root->children()) {
    QString path = item->path().section(QLatin1Char('/'), 0, -r);
    if (!item->flag(ProxyItem::Host) && !QFileInfo(path).isAbsolute()) {
      continue;
    }

    // make sure we're actually matching against the right dir,
    // previously the check below would match /foo/xy against /foo/x
    // and return /foo/x rather than /foo/xy
    // this seems a bit hackish, but is the simplest way to solve the
    // current issue.
    path += QLatin1Char('/');

    if(name.startsWith(path) && item->flag(ProxyItem::Dir)) {
      return static_cast<ProxyItemDir*>(item);
    }
  }

  return 0;
}

ProxyItemDir *KateFileTreeModel::findChildNode(ProxyItemDir *parent, const QString &name)
{
  Q_ASSERT(parent != 0);

  if(!parent || !parent->childCount()) {
    return 0;
  }

  foreach(ProxyItem *item, parent->children()) {
    if(item->display() == name) {
      if(!item->flag(ProxyItem::Dir)) {
        return 0;
      }

      return static_cast<ProxyItemDir*>(item);
    }
  }

  return 0;
}

void KateFileTreeModel::insertItemInto(ProxyItemDir *root, ProxyItem *item)
{
  Q_ASSERT(root != 0);
  Q_ASSERT(item != 0);

  QString sep;
  QString tail = item->path();
  tail.remove(0, root->path().length());
  QStringList parts = tail.split(QLatin1Char('/'), QString::SkipEmptyParts);
  ProxyItemDir *ptr = root;
  QStringList current_parts;
  current_parts.append(root->path());

  // seems this can be empty, see bug 286191
  if (!parts.isEmpty())
    parts.pop_back();

  foreach(const QString &part, parts) {
    current_parts.append(part);
    ProxyItemDir *find = findChildNode(ptr, part);
    if(!find) {
      QString new_name = current_parts.join(QLatin1String("/"));
      QModelIndex parent_index = createIndex(ptr->row(), 0, ptr);
      beginInsertRows(ptr == m_root ? QModelIndex() : parent_index, ptr->childCount(), ptr->childCount());
      ptr = new ProxyItemDir(new_name, ptr);
      endInsertRows();
    }
    else {
        ptr = find;
    }
  }

  QModelIndex parent_index = createIndex(ptr->row(), 0, ptr);
  beginInsertRows(ptr == m_root ? QModelIndex() : parent_index, ptr->childCount(), ptr->childCount());
    ptr->addChild(item);
  endInsertRows();
}

void KateFileTreeModel::handleInsert(ProxyItem *item)
{
  Q_ASSERT(item != 0);

  if(m_listMode) {
    beginInsertRows(QModelIndex(), m_root->childCount(), m_root->childCount());
    m_root->addChild(item);
    endInsertRows();
    return;
  }

  if(item->flag(ProxyItem::Empty)) {
    beginInsertRows(QModelIndex(), m_root->childCount(), m_root->childCount());
    m_root->addChild(item);
    endInsertRows();
    return;
  }

  ProxyItemDir *root = findRootNode(item->path());
  if(root) {
    insertItemInto(root, item);
  } else {

    // trim off trailing file and dir
    QString base = item->path().section(QLatin1Char('/'), 0, -2);

    // create new root
    ProxyItemDir *new_root = new ProxyItemDir(base, 0);
    new_root->setHost(item->host());

    // add new root to m_root
    beginInsertRows(QModelIndex(), m_root->childCount(), m_root->childCount());
      m_root->addChild(new_root);
    endInsertRows();

    // same fix as in findRootNode, try to match a full dir, instead of a partial path
    base += QLatin1Char('/');

    // try and merge existing roots with the new root node.
    foreach(ProxyItem *root, m_root->children()) {
      if(root == new_root || !root->flag(ProxyItem::Dir))
        continue;

      if(root->path().startsWith(base)) {
        beginRemoveRows(QModelIndex(), root->row(), root->row());
          m_root->remChild(root);
        endRemoveRows();

        //beginInsertRows(new_root_index, new_root->childCount(), new_root->childCount());
          // this can't use new_root->addChild directly, or it'll potentially miss a bunch of subdirs
          insertItemInto(new_root, root);
        //endInsertRows();
      }
    }

    // add item to new root
    // have to call begin/endInsertRows here, or the new item won't show up.
    QModelIndex new_root_index = createIndex(new_root->row(), 0, new_root);
    beginInsertRows(new_root_index, new_root->childCount(), new_root->childCount());
      new_root->addChild(item);
    endInsertRows();

  }
}

void KateFileTreeModel::handleNameChange(ProxyItem *item, const QString &new_name, const QString& new_host)
{
  Q_ASSERT(item != 0);

  if(m_listMode) {
    item->setPath(new_name);
    item->setHost(new_host);
    QModelIndex idx = createIndex(item->row(), 0, item);
    setupIcon(item);
    emit dataChanged(idx, idx);
    return;
  }

  // for some reason we get useless name changes
  if(item->path() == new_name) {
    return;
  }

  // in either case (new/change) we want to remove the item from its parent

  ProxyItemDir *parent = item->parent();
  if(!parent) {
    item->setPath(new_name);
    item->setHost(new_host);
    return;
  }

  item->setPath(new_name);
  item->setHost(new_host);

  QModelIndex parent_index = parent == m_root ? QModelIndex() : createIndex(parent->row(), 0, parent);
  beginRemoveRows(parent_index, item->row(), item->row());
    parent->remChild(item);
  endRemoveRows();

  // remove empty parent nodes here, recursively.
  handleEmptyParents(parent);

  // set new path
  //item->setPath(new_name);

  // clear all but Empty flag
  if(item->flag(ProxyItem::Empty))
    item->setFlags(ProxyItem::Empty);
  else
    item->setFlags(ProxyItem::None);

  setupIcon(item);

  // new item
  handleInsert(item);
}

void KateFileTreeModel::setupIcon(ProxyItem *item)
{
  Q_ASSERT(item != 0);

  QStringList emblems;
  QString icon_name;

  if(item->flag(ProxyItem::Modified)) {
    icon_name = "document-save";
  }
  else {
    QUrl url(item->path());
    icon_name = QMimeDatabase().mimeTypeForFile(url.path(), QMimeDatabase::MatchExtension).iconName();
  }

  QIcon icon = QIcon::fromTheme(icon_name);

  if(item->flag(ProxyItem::ModifiedExternally) || item->flag(ProxyItem::DeletedExternally)) {
    icon = KIconUtils::addOverlay(icon, QIcon("emblem-important"), Qt::TopLeftCorner);
  }

  item->setIcon(icon);
}

// kate: space-indent on; indent-width 2; replace-tabs on; mixed-indent off;
