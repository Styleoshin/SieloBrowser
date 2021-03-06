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
#ifndef CORE_WEBPAGE_HPP
#define CORE_WEBPAGE_HPP

#include <QWebEnginePage>
#include <QWebEngineScript>
#include <QWebEngineFullScreenRequest>

#include <QPointF>
#include <QVariant>

#include <QEventLoop>

#include "Password/PasswordManager.hpp"

namespace Sn {

class WebView;
class WebHitTestResult;

class DelayedFileWatcher;

class WebPage: public QWebEnginePage {
Q_OBJECT

public:
	WebPage(QObject* parent = nullptr);
	~WebPage();

	WebView* view() const;

	QVariant executeJavaScript(const QString& scriptSrc, quint32 worldId = QWebEngineScript::MainWorld,
							   int timeout = 500);

	QPointF mapToViewport(const QPointF& pos) const;
	WebHitTestResult hitTestContent(const QPoint& pos) const;

	void scroll(int x, int y);
	void setScrollPosition(const QPointF& pos);

	void javaScriptAlert(const QUrl& securityOrigin, const QString& msg) Q_DECL_OVERRIDE;

	void setJavaScriptEnable(bool enabled);

	bool isRunningLoop();

	bool isLoading() const;

	void setupWebChannel();

	static QString setCSS(const QString& css);
signals:
	void privacyChanged(bool status);

protected slots:
	void progress(int progression);
	void finished();

private slots:
	void cleanBlockedObject();
	void urlChanged(const QUrl& url);
	void watchedFileChanged(const QString& file);
	void windowCloseRequested();
	void featurePermissionRequested(const QUrl& origine, const QWebEnginePage::Feature& feature);

private:
	bool acceptNavigationRequest(const QUrl& url, NavigationType type, bool isMainFrame) Q_DECL_OVERRIDE;
	QWebEnginePage* createWindow(WebWindowType type) Q_DECL_OVERRIDE;

	void handleUnknowProtocol(const QUrl& url);
	void desktopServiceOpen(const QUrl& url);

	DelayedFileWatcher* m_fileWatcher{nullptr};
	QEventLoop* m_runningLoop{nullptr};

	QVector<PasswordEntry> m_passwordEntries;

	int m_loadProgress{-1};
	bool m_blockAlerts{false};
	bool m_secureStatus{false};
	bool m_adjustingSheduled{false};

};

}

#endif // CORE_WEBPAGE_HPP
