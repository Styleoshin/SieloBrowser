/***********************************************************************************
** MIT License                                                                    **
**                                                                                **
** Copyright (c) 2018 Victor DENIS (victordenis01@gmail.com)                      **
**                                                                                **
** Permission is hereby granted, free of charge, to any person obtaining a copy   **
** of this software and associated documentation files (the "Software"), to deal  **
** in the Software without restriction, including without limitation the rights   **
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      **
** copies of the Software, and to permit persons to whom the Software is          **
** furnished to do so, subject to the following conditions:                       **
**                                                                                **
** The above copyright notice and this permission notice shall be included in all **
** copies or substantial portions of the Software.                                **
**                                                                                **
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     **
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       **
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    **
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         **
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  **
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  **
** SOFTWARE.                                                                      **
***********************************************************************************/

#pragma once
#ifndef SIELOBROWSER_TABBEDWEBVIEW_HPP
#define SIELOBROWSER_TABBEDWEBVIEW_HPP

#include <QLabel>
#include <QMenu>

#include <QHostInfo>

#include "Application.hpp"

#include "Web/WebView.hpp"

namespace Sn {
class WebPage;
class BrowserWindow;
class LoadRequest;

class WebTab;

class TabbedWebView: public WebView {
Q_OBJECT

public:
	TabbedWebView(WebTab* tab);

	void setWebPage(WebPage* page);

	BrowserWindow* browserWindow() const { return m_window; }
	void setBrowserWindow(BrowserWindow* window);

	WebTab* webTab() const { return m_webTab; }

	QString getIp() const { return m_currentIp; }

	int tabIndex() const;

	QWidget* overlayWidget() Q_DECL_OVERRIDE;
	void closeView() Q_DECL_OVERRIDE;
	void loadInNewTab(const LoadRequest& request, Application::NewTabType tabType) Q_DECL_OVERRIDE;

	bool isFullScreen() Q_DECL_OVERRIDE;
	void requestFullScreen(bool enable) Q_DECL_OVERRIDE;

signals:
	void wantsCloseTab(int);
	void ipChanged(QString);

public slots:
	void setAsCurrentTab();
	void userLoadAction(const LoadRequest& request);

protected:
	QImage applyBlur(QImage src, qreal radius, bool quality = true, bool alphaOnly = false, int transposed = 0);
	void paintEvent(QPaintEvent* event);

private slots:
	void sLoadStarted();
	void sLoadProgress(int progress);
	void sLoadFinished();
	void urlChanged(const QUrl& url);
	void linkHovered(const QString& link);
	void setIp(const QHostInfo& info);

private:
	void newContextMenuEvent(QContextMenuEvent* event) Q_DECL_OVERRIDE;
	void newMousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
	void newMouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

	void dragEnterEvent(QDragEnterEvent* event);
	void dragLeaveEvent(QDragLeaveEvent* event);
	void dragMoveEvent(QDragMoveEvent* event);
	void dropEvent(QDropEvent* event);
	void enterEvent(QEvent* event);
	void leaveEvent(QEvent* event);

	QFrame* m_highlightedFrame{};

	BrowserWindow* m_window{nullptr};
	WebTab* m_webTab{nullptr};
	QMenu* m_menu{nullptr};

	QString m_currentIp{};

	bool m_cursorIn{false};
};

}

#endif //SIELOBROWSER_TABBEDWEBVIEW_HPP
