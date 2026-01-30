#pragma once

#include <QWidget>
#include "TalkWindowShell.h"

#include "ui_TalkWindow.h"


extern enum GroupType;

class TalkWindow : public QWidget
{
	Q_OBJECT

public:
	TalkWindow(QWidget* parent, const QString& uid, GroupType groupType);
	~TalkWindow();

public:
	void addEmotionImage(int emotionNum);					// 添加表情
	void setWindowName(const QString& name);			// 设置窗口名字

private:
	void initControl();						// 初始化

	void initCompanylTalk();			// 初始化 公司群聊天
	void initPersonelTalk();			// 初始化 人事部聊天
	void initDeveloplTalk();			// 初始化 研发部聊天
	void initMarketTalk();				// 初始化 市场部聊天
	void initPtoPTalk();					// 初始化 市场部聊天

	// 给哪一项添加，所以要传一个地址进去
	void addPeopInfo(QTreeWidgetItem* pRootGroupItem);

private slots:
	void onSendBtnClicked(bool);			// 发送信号
	void onItemDoubleClicked(QTreeWidgetItem* item, int column);		// 双击事件


private:
	Ui::TalkWindow ui;

	QString m_talkId;							// 聊天窗口的ID
	GroupType m_groupType;

	// 群组里面 ， 群成员的映射
	// 比如在群里面，打开 琪琪1聊天窗口，只能得到值，但是得不到窗口名字
	// 因此，要做一个映射，让值和窗口名字，一一对应起来
	QMap<QTreeWidgetItem*, QString> m_groupPeoMap;

};
