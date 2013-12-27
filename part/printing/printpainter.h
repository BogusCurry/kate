/*  This file is part of the KDE libraries and the Kate part.
 *
 *  Copyright (C) 2002-2010 Anders Lund <anders@alweb.dk>
 *
 *  Rewritten based on code of Copyright (c) 2002 Michael Goffioul <kdeprint@swing.be>
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

#ifndef __KATE_PRINTER_PAINTER_H__
#define __KATE_PRINTER_PAINTER_H__

#include <QString>
#include <QColor>
#include <QFont>

class KateDocument;
class QPrinter;

namespace KatePrinter
{
  class PrintPainter
  {
    public:
      PrintPainter(KateDocument *doc);

      void paint(QPrinter *printer) const;

      // Attributes
      void setColorScheme(const QString &scheme) { m_colorScheme = scheme; }
      void setPrintGuide(const bool on) { m_printGuide = on; }
      void setPrintLineNumbers(const bool on) { m_printLineNumbers = on; }
      void setUseHeader(const bool on) { m_useHeader = on; }
      void setUseFooter(const bool on) { m_useFooter = on; }
      void setUseBackground(const bool on) { m_useBackground = on; }
      void setUseBox(const bool on) { m_useBox = on; }
      void setBoxMargin(const int margin) { m_boxMargin = margin; }
      void setBoxWidth(const int width) { m_boxWidth = width; }
      void setBoxColor(const QColor &color) { m_boxColor = color; }
      void setHeadersFont(const QFont &font) { m_fhFont = font; }

      void setHeaderBackground(const QColor &color) { m_headerBackground = color; }
      void setHeaderForeground(const QColor &color) { m_headerForeground = color; }
      void setUseHeaderBackground(const bool on) { m_useHeaderBackground = on; }

      void setFooterBackground(const QColor &color) { m_footerBackground = color; }
      void setFooterForeground(const QColor &color) { m_footerForeground = color; }
      void setUseFooterBackground(const bool on) { m_useFooterBackground = on; }

      void setHeaderFormat(const QStringList &list) { m_headerFormat = list; }
      void setFooterFormat(const QStringList &list) { m_footerFormat = list; }

    private:
      KateDocument *m_doc;

      QString m_colorScheme;
      bool m_printGuide;
      bool m_printLineNumbers;
      bool m_useHeader;
      bool m_useFooter;
      bool m_useBackground;
      bool m_useBox;
      bool m_useHeaderBackground;
      bool m_useFooterBackground;

      int m_boxMargin;
      int m_boxWidth;
      QColor m_boxColor;

      QColor m_headerBackground;
      QColor m_headerForeground;
      QColor m_footerBackground;
      QColor m_footerForeground;

      QFont m_fhFont;

      QStringList m_headerFormat;
      QStringList m_footerFormat;
  };
};

#endif
