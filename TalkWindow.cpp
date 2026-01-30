#include "TalkWindow.h"
#include "RootContactItem.h"
#include "ContactItem.h"
#include "CommonUtils.h"
#include "WindowManager.h"


#include <QToolTip>
#include <QFile>
#include <QMessageBox>



// 构造函数：初始化聊天窗口
// parent: 父窗口指针
// uid: 聊天窗口唯一标识ID
// groupType: 聊天窗口所属群组类型
TalkWindow::TalkWindow(QWidget* parent, const QString& uid, GroupType groupType)
	: QWidget(parent)
	, m_talkId(uid)
	, m_groupType(groupType)
{
	// 初始化UI界面
	ui.setupUi(this);

	// 将当前聊天窗口注册到窗口管理器中，建立ID与窗口的映射关系
	WindowManager::getInstance()->addWindowName(m_talkId, this);

	// 设置窗口关闭时自动销毁，避免内存泄漏
	setAttribute(Qt::WA_DeleteOnClose);
	// 初始化窗口控件
	initControl();

}

// 析构函数：释放聊天窗口资源
TalkWindow::~TalkWindow()
{
	// 从窗口管理器中移除当前聊天窗口的注册信息
	WindowManager::getInstance()->deleteWindowName(m_talkId);
}

// 添加表情图片到输入框
// emotionNum: 表情编号
void TalkWindow::addEmotionImage(int emotionNum)
{
	// 确保输入框获得焦点
	ui.textEdit->setFocus();
	// 向输入框中添加指定编号的表情
	ui.textEdit->addEmotionUrl(emotionNum);
}

// 发送按钮点击事件处理函数
void TalkWindow::onSendBtnClicked(bool)
{
	// 检查输入框内容是否为空
	if (ui.textEdit->toPlainText().isEmpty())
	{
		// 显示提示信息：输入框不能为空，提示框显示2秒
		QToolTip::showText(this->mapToGlobal(QPoint(630, 660)),
			QString::fromLocal8Bit("输入内容不能为空！"), this, QRect(0, 0, 120, 100), 2000);
		return;
	}

	// 获取输入框的HTML格式内容
	QString html = ui.textEdit->document()->toHtml();

	// 如果内容不包含表情(png)或富文本标签(span)，则应用字体样式模板
	if (!html.contains(".png") && !html.contains("</span>"))
	{
		QString fontHtml;
		QString text = ui.textEdit->toPlainText();
		QFile file(":/Resources/MainWindow/MsgHtml/msgFont.txt");
		if (file.open(QIODevice::ReadOnly))
		{
			// 读取字体样式模板文件
			fontHtml = file.readAll();
			// 将模板中的占位符替换为实际输入文本
			fontHtml.replace("%1", text);
			file.close();
		}
		else
		{
			// 模板文件读取失败，显示错误提示
			QMessageBox::information(this, QString::fromLocal8Bit("提示"),
				QString::fromLocal8Bit("无法读取 msgFont.txt 字体模板文件"));
			return;
		}

		// 检查并替换原始文本为带样式的HTML文本
		if (!html.contains(fontHtml))
		{
			html.replace(text, fontHtml);
		}
	}

	// 清空输入框内容
	ui.textEdit->clear();
	// 删除输入框中所有表情图片
	ui.textEdit->deleteAllEmotionImage();

	// 将处理后的消息添加到消息显示区域
	ui.msgWidget->appendMsg(html);

}

// 设置聊天窗口显示的名称
// name: 要显示的名称
void TalkWindow::setWindowName(const QString& name)
{
	ui.nameLabel->setText(name);
}

// 初始化窗口控件和布局
void TalkWindow::initControl()
{
	// 设置右侧面板的尺寸分割比例
	QList<int> rightWidgetSize;
	rightWidgetSize << 600 << 138;

	// 应用分割面板的尺寸设置
	ui.bodySplitter->setSizes(rightWidgetSize);

	// 设置输入框字体大小为10号
	ui.textEdit->setFontPointSize(10);
	// 设置输入框为焦点控件，方便用户直接输入
	ui.textEdit->setFocus();

	// 窗口最小化、关闭按钮信号连接
	connect(ui.sysmin, SIGNAL(clicked(bool)), parent(), SLOT(onShowMin(bool)));
	connect(ui.sysclose, SIGNAL(clicked(bool)), parent(), SLOT(onShowClose(bool)));
	connect(ui.closeBtn, SIGNAL(clicked(bool)), parent(), SLOT(onShowClose(bool)));

	// 表情按钮点击信号连接
	connect(ui.faceBtn, SIGNAL(clicked(bool)), parent(), SLOT(onEmotionBtnClicked(bool)));

	// 发送按钮点击信号连接
	connect(ui.sendBtn, SIGNAL(clicked(bool)), this, SLOT(onSendBtnClicked(bool)));

	// 联系人树控件双击事件连接
	connect(ui.treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
		this, SLOT(onItemDoubleClicked(QTreeWidgetItem*, int)));

	// 根据群组类型初始化不同的聊天窗口样式
	switch (m_groupType)
	{
	case COMPANY:
	{
		initCompanylTalk();			// 初始化公司群聊界面
		break;
	}
	case PERSONELGROUP:
	{
		initPersonelTalk();			// 初始化人事群组聊天界面
		break;
	}
	case DEVELOPMENTGROUP:
	{
		initDeveloplTalk();			// 初始化研发群组聊天界面
		break;
	}
	case MARKETGROUP:
	{
		initMarketTalk();				// 初始化市场群组聊天界面
		break;
	}
	default:								// 默认情况
	{
		initPtoPTalk();					// 初始化点对点聊天界面
		break;
	}
	}
}

// 初始化公司群聊界面
void TalkWindow::initCompanylTalk()
{
	// 创建根节点项
	QTreeWidgetItem* pRootItem = new QTreeWidgetItem();

	// 设置根节点显示展开/折叠指示器
	pRootItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

	// 设置根节点数据标识：0表示根节点
	pRootItem->setData(0, Qt::UserRole, 0);

	// 创建根节点显示控件（不显示复选框）
	RootContactItem* pItemName = new RootContactItem(false, ui.treeWidget);

	// 设置联系人树控件固定高度
	ui.treeWidget->setFixedHeight(646);

	// 模拟公司群成员数量
	int nEmployeeNum = 50;
	// 构建群组名称（在线人数/总人数）
	QString qsGroupName = QString::fromLocal8Bit("公司群 %1/%2").arg(0).arg(nEmployeeNum);
	// 设置群组名称显示
	pItemName->setText(qsGroupName);

	// 将根节点添加到树控件
	ui.treeWidget->addTopLevelItem(pRootItem);
	// 将根节点显示控件绑定到树控件
	ui.treeWidget->setItemWidget(pRootItem, 0, pItemName);

	// 展开根节点
	pRootItem->setExpanded(true);

	// 批量添加群成员信息
	for (int i = 0; i < nEmployeeNum; i++)
	{
		addPeopInfo(pRootItem);
	}
}

// 初始化人事群组聊天界面
void TalkWindow::initPersonelTalk()
{
	// 创建根节点项
	QTreeWidgetItem* pRootItem = new QTreeWidgetItem();

	// 设置根节点显示展开/折叠指示器
	pRootItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

	// 设置根节点数据标识：0表示根节点
	pRootItem->setData(0, Qt::UserRole, 0);

	// 创建根节点显示控件（不显示复选框）
	RootContactItem* pItemName = new RootContactItem(false, ui.treeWidget);

	// 设置联系人树控件固定高度
	ui.treeWidget->setFixedHeight(646);

	// 模拟人事群成员数量
	int nEmployeeNum = 5;
	// 构建群组名称（在线人数/总人数）
	QString qsGroupName = QString::fromLocal8Bit("人事群 %1/%2").arg(0).arg(nEmployeeNum);
	// 设置群组名称显示
	pItemName->setText(qsGroupName);

	// 将根节点添加到树控件
	ui.treeWidget->addTopLevelItem(pRootItem);
	// 将根节点显示控件绑定到树控件
	ui.treeWidget->setItemWidget(pRootItem, 0, pItemName);

	// 展开根节点
	pRootItem->setExpanded(true);

	// 批量添加群成员信息
	for (int i = 0; i < nEmployeeNum; i++)
	{
		addPeopInfo(pRootItem);
	}
}

// 初始化研发群组聊天界面
void TalkWindow::initDeveloplTalk()
{
	// 创建根节点项
	QTreeWidgetItem* pRootItem = new QTreeWidgetItem();

	// 设置根节点显示展开/折叠指示器
	pRootItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

	// 设置根节点数据标识：0表示根节点
	pRootItem->setData(0, Qt::UserRole, 0);

	// 创建根节点显示控件（不显示复选框）
	RootContactItem* pItemName = new RootContactItem(false, ui.treeWidget);

	// 设置联系人树控件固定高度
	ui.treeWidget->setFixedHeight(646);

	// 模拟研发群成员数量
	int nEmployeeNum = 10;
	// 构建群组名称（在线人数/总人数）
	QString qsGroupName = QString::fromLocal8Bit("研发群 %1/%2").arg(0).arg(nEmployeeNum);
	// 设置群组名称显示
	pItemName->setText(qsGroupName);

	// 将根节点添加到树控件
	ui.treeWidget->addTopLevelItem(pRootItem);
	// 将根节点显示控件绑定到树控件
	ui.treeWidget->setItemWidget(pRootItem, 0, pItemName);

	// 展开根节点
	pRootItem->setExpanded(true);

	// 批量添加群成员信息
	for (int i = 0; i < nEmployeeNum; i++)
	{
		addPeopInfo(pRootItem);
	}
}

// 初始化市场群组聊天界面
void TalkWindow::initMarketTalk()
{
	// 创建根节点项
	QTreeWidgetItem* pRootItem = new QTreeWidgetItem();

	// 设置根节点显示展开/折叠指示器
	pRootItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

	// 设置根节点数据标识：0表示根节点
	pRootItem->setData(0, Qt::UserRole, 0);

	// 创建根节点显示控件（不显示复选框）
	RootContactItem* pItemName = new RootContactItem(false, ui.treeWidget);

	// 设置联系人树控件固定高度
	ui.treeWidget->setFixedHeight(646);

	// 模拟市场群成员数量
	int nEmployeeNum = 20;
	// 构建群组名称（在线人数/总人数）
	QString qsGroupName = QString::fromLocal8Bit("市场群 %1/%2").arg(0).arg(nEmployeeNum);
	// 设置群组名称显示
	pItemName->setText(qsGroupName);

	// 将根节点添加到树控件
	ui.treeWidget->addTopLevelItem(pRootItem);
	// 将根节点显示控件绑定到树控件
	ui.treeWidget->setItemWidget(pRootItem, 0, pItemName);

	// 展开根节点
	pRootItem->setExpanded(true);

	// 批量添加群成员信息
	for (int i = 0; i < nEmployeeNum; i++)
	{
		addPeopInfo(pRootItem);
	}
}

// 初始化点对点聊天界面
void TalkWindow::initPtoPTalk()
{
	// 加载聊天背景皮肤图片
	QPixmap pixSkin;
	pixSkin.load(":/Resources/MainWindow/skin.png");

	// 设置背景面板尺寸与图片一致
	ui.widget->setFixedSize(pixSkin.size());

	// 创建背景图片显示标签
	QLabel* skinLabel = new QLabel(ui.widget);
	skinLabel->setPixmap(pixSkin);

	// 设置标签尺寸匹配面板尺寸
	skinLabel->setFixedSize(ui.widget->size());
}

// 向群组中添加人员信息
// pRootGroupItem: 要添加成员的群组根节点
void TalkWindow::addPeopInfo(QTreeWidgetItem* pRootGroupItem)
{
	// 创建成员子节点
	QTreeWidgetItem* pChild = new QTreeWidgetItem();

	// 加载头像遮罩图片
	QPixmap pix1;
	pix1.load(":/Resources/MainWindow/head_mask.png");

	// 加载默认头像图片
	const QPixmap image(":/Resources/MainWindow/girl.png");

	// 设置子节点数据标识：1表示成员节点
	pChild->setData(0, Qt::UserRole, 1);
	// 设置成员唯一标识（使用指针地址作为ID）
	pChild->setData(0, Qt::UserRole + 1, QString::number((int)pChild));

	// 创建成员显示控件
	ContactItem* pContactItem = new ContactItem(ui.treeWidget);

	static int i = 0;

	// 设置成员头像（应用圆形遮罩处理）
	pContactItem->setHeadPixmap(CommonUtils::getRoundImage(image, pix1, pContactItem->getHeadLabelSize()));

	// 设置成员名称（自动递增编号）
	pContactItem->setUserName(QString::fromLocal8Bit("成员%1").arg(i++));
	// 设置成员签名（空字符串）
	pContactItem->setSignName(QString::fromLocal8Bit(""));

	// 将成员节点添加到群组根节点下
	pRootGroupItem->addChild(pChild);

	// 将成员显示控件绑定到树控件
	ui.treeWidget->setItemWidget(pChild, 0, pContactItem);

	// 保存成员节点与名称的映射关系
	QString str = pContactItem->getUserName();
	m_groupPeoMap.insert(pChild, str);
}

// 联系人树控件项双击事件处理
// item: 被双击的树节点
// column: 被双击的列索引
void TalkWindow::onItemDoubleClicked(QTreeWidgetItem* item, int column)
{
	// 判断是否为成员节点（1表示成员节点）
	bool bIsChild = item->data(0, Qt::UserRole).toBool();

	// 如果是成员节点
	if (bIsChild)
	{
		// 获取成员名称
		QString strPeopleName = m_groupPeoMap.value(item);

		// 创建新的点对点聊天窗口
		WindowManager::getInstance()->addNewTalkWindow(item->data(0, Qt::UserRole + 1).toString(), PTOP, strPeopleName);
	}
}