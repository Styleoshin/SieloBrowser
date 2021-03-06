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

#include "Bookmarks/BookmarkManager.hpp"

#include <QFile>
#include <QFileDialog>

#include <QMessageBox>

#include <QDate>

#include <QCoreApplication>
#include <QtDebug>

#include "Application.hpp"

#include "Utils/AutoSaver.hpp"

#include "Bookmarks/BookmarkNode.hpp"
#include "Bookmarks/BookmarksModel.hpp"
#include "Bookmarks/BookmarksDialog.hpp"
#include "Bookmarks/XBelReader.hpp"
#include "Bookmarks/XBelWriter.hpp"
#include "Bookmarks/BookmarksCommands.hpp"

namespace Sn {
BookmarksManager::BookmarksManager(QObject* parent) :
		QObject(parent),
		m_loaded(false),
		m_saveTimer(new AutoSaver(this)),
		m_bookmarkRootNode(nullptr),
		m_bookmarksModel(nullptr)
{
	connect(this, &BookmarksManager::entryAdded, m_saveTimer, &AutoSaver::changeOccurred);
	connect(this, &BookmarksManager::entryRemoved, m_saveTimer, &AutoSaver::changeOccurred);
	connect(this, &BookmarksManager::entryChanged, m_saveTimer, &AutoSaver::changeOccurred);
}

BookmarksManager::~BookmarksManager()
{
	m_saveTimer->saveIfNeccessary();
}

void BookmarksManager::addBookmark(BookmarkNode* parent, BookmarkNode* node, int row)
{
	if (!m_loaded)
		return;

	Q_ASSERT(parent);

	InsertBookmarksCommand* command{new InsertBookmarksCommand(this, parent, node, row)};
	command->undo();
}

void BookmarksManager::removeBookmark(BookmarkNode* node)
{
	if (!m_loaded)
		return;

	Q_ASSERT(node);

	BookmarkNode* parent{node->parent()};
	int row{parent->children().indexOf(node)};

	RemoveBookmarksCommand* command{new RemoveBookmarksCommand(this, parent, row)};
	command->redo();
}

void BookmarksManager::setTitle(BookmarkNode* node, const QString& newTitle)
{
	if (!m_loaded)
		return;

	Q_ASSERT(node);

	ChangeBookmarksCommand* command{new ChangeBookmarksCommand(this, node, newTitle, true)};
	m_commands.push(command);
}

void BookmarksManager::setUrl(BookmarkNode* node, const QString& newUrl)
{
	if (!m_loaded)
		return;

	Q_ASSERT(node);

	ChangeBookmarksCommand* command{new ChangeBookmarksCommand(this, node, newUrl, false)};
	m_commands.push(command);
}

void BookmarksManager::changeExpanded()
{
	m_saveTimer->changeOccurred();
}

BookmarkNode* BookmarksManager::bookmarks()
{
	if (!m_loaded)
		load();

	return m_bookmarkRootNode;
}

BookmarkNode* BookmarksManager::menu()
{
	if (!m_loaded)
		load();

	for (int i{m_bookmarkRootNode->children().count() - 1}; i >= 0; --i) {
		BookmarkNode* node{m_bookmarkRootNode->children()[i]};

		if (node->title == tr("Bookmarks Menu"))
			return node;
	}

	Q_ASSERT(false);
	return nullptr;
}

BookmarkNode* BookmarksManager::toolbar()
{
	if (!m_loaded)
		load();

	for (int i{m_bookmarkRootNode->children().count() - 1}; i >= 0; --i) {
		BookmarkNode* node{m_bookmarkRootNode->children()[i]};
		if (node->title == tr("Bookmarks Bar"))
			return node;
	}

	Q_ASSERT(false);
	return 0;
}

BookmarksModel* BookmarksManager::bookmarksModel()
{
	if (!m_bookmarksModel)
		m_bookmarksModel = new BookmarksModel(this, this);

	return m_bookmarksModel;
}

void BookmarksManager::importBookmarks()
{
	QString fileName{QFileDialog::getOpenFileName(nullptr, tr("Open File"), QString(), tr("XBEL (*.xbel *.xml)"))};

	if (fileName.isEmpty())
		return;

	XBelReader reader{};
	BookmarkNode* importRootNode{reader.read(fileName)};

	if (reader.error() != QXmlStreamReader::NoError) {
		QMessageBox::warning(nullptr,
							 tr("Loading Bookmark"),
							 tr("Error when loading bookmarks on line %1, column %2:\n%3").arg(reader.lineNumber())
									 .arg(reader.columnNumber()).arg(reader.errorString()));
	}

	importRootNode->setType(BookmarkNode::Folder);
	importRootNode->title = (tr("Imported %1").arg(QDate::currentDate().toString(Qt::SystemLocaleShortDate)));
}

void BookmarksManager::exportBookmarks()
{
	QString fileName{QFileDialog::getSaveFileName(nullptr,
												  tr("Save File"),
												  tr("%1 Bookmarks.xbel").arg(QCoreApplication::applicationName()),
												  tr("XBEL (*.xbel *.xml)"))};

	if (fileName.isEmpty())
		return;

	XBelWriter writer{};

	if (!writer.write(fileName, m_bookmarkRootNode))
		QMessageBox::critical(nullptr, tr("Export error"), tr("Error saving bookmarks"));
}

void BookmarksManager::showBookmarks()
{
	BookmarksDialog* dialog{new BookmarksDialog(nullptr, this)};
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}

void BookmarksManager::save() const
{
	if (!m_loaded)
		return;

	XBelWriter writer{};
	QString directory{Application::instance()->paths()[Application::P_Data]};
	QString bookmarksFile{directory + QLatin1String("/bookmarks.xbel")};

	if (!writer.write(bookmarksFile, m_bookmarkRootNode))
		qWarning() << "BookmarksManager: error saving to " << bookmarksFile;
}

void BookmarksManager::load()
{
	if (m_loaded)
		return;

	m_loaded = true;

	QString directory{Application::instance()->paths()[Application::P_Data]};
	QString bookmarksFile{directory + QLatin1String("/bookmarks.xbel")};

	if (!QFile::exists(bookmarksFile)) {
		QFile::copy(QLatin1String(":data/bookmarks.xbel"), directory + QLatin1String("/bookmarks.xbel"));
		QFile::setPermissions(directory + QLatin1String("/bookmarks.xbel"), QFileDevice::ReadUser | QFileDevice::WriteUser);
	}

	XBelReader reader{};

	m_bookmarkRootNode = reader.read(bookmarksFile);

	if (reader.error() != QXmlStreamReader::NoError) {
		QMessageBox::warning(nullptr,
							 tr("Loading Bookmarks"),
							 tr("Error when loading bookmarks on line %1, column %2:\n%3").arg(reader.lineNumber())
									 .arg(reader.columnNumber()).arg(reader.errorString()));
	}

	BookmarkNode* toolbar{nullptr};
	BookmarkNode* menu{nullptr};

	QList<BookmarkNode*> others;

	for (int i{m_bookmarkRootNode->children().count() - 1}; i >= 0; --i) {
		BookmarkNode* node{m_bookmarkRootNode->children()[i]};

		if (node->type() == BookmarkNode::Folder) {
			if (node->title == tr("Toolbar Bookmarks") && !toolbar)
				node->title = tr("Bookmarks Bar");

			if (node->title == tr("Bookmarks Bar") && !toolbar)
				toolbar = node;

			if (node->title == tr("Menu") && !menu)
				node->title = tr("Bookmarks Menu");

			if (node->title == tr("Bookmarks Menu") && !menu)
				menu = node;
		}
		else {
			others.append(node);
		}

		m_bookmarkRootNode->remove(node);
	}

	Q_ASSERT(m_bookmarkRootNode->children().count() == 0);

	if (!toolbar) {
		toolbar = new BookmarkNode(BookmarkNode::Folder, m_bookmarkRootNode);
		toolbar->title = tr("Bookmarks Bar");
	}
	else {
		m_bookmarkRootNode->add(toolbar);
	}

	if (!menu) {
		menu = new BookmarkNode(BookmarkNode::Folder, m_bookmarkRootNode);
		menu->title = tr("Bookmarks Menu");
	}
	else {
		m_bookmarkRootNode->add(menu);
	}

	for (int i{0}; i < others.count(); ++i)
		menu->add(others[i]);
}
}