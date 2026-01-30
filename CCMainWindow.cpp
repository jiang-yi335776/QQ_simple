#include "CCMainWindow.h"
#include "SkinWindow.h"
#include "SysTray.h"
#include "NotifyManager.h"
#include "RootContactItem.h"
#include "ContactItem.h"
#include "WindowManager.h"
#include "TalkWindowShell.h"

#include <QHBoxLayout>
#include <QProxyStyle>
#include <QPainter>
#include <QTimer>
#include <QEvent>
#include <QTreeWidgetItem>
#include <QMouseEvent>
#include <QApplication>

// 自定义代理样式类 - 用于移除焦点框的绘制
class CCMainCustomProxyStyle : public QProxyStyle
{
public:
    // 重写绘制基础元素的方法
    // 主要目的：移除焦点框的绘制，提升界面美观度
    virtual void drawPrimitive(PrimitiveElement element, const QStyleOption* option,
        QPainter* painter, const QWidget* widget = nullptr) const override
    {
        // 过滤焦点框元素，不进行绘制
        if (element == PE_FrameFocusRect)
        {
            return;
        }
        else
        {
            // 其他元素使用默认绘制方式
            QProxyStyle::drawPrimitive(element, option, painter, widget);
        }
    }
};

// 主窗口构造函数
CCMainWindow::CCMainWindow(QWidget* parent)
    : BasicWindow(parent)
{
    ui.setupUi(this);

    // 设置窗口属性为工具窗口，使其可以置顶显示
    setWindowFlags(windowFlags() | Qt::Tool);

    // 加载主窗口样式表
    loadStyleSheet("CCMainWindow");

    // 初始化控件布局和属性
    initControl();

    // 初始化定时器（用于等级数字动态更新）
    initTimer();
}

// 主窗口析构函数
CCMainWindow::~CCMainWindow()
{
}

// 初始化所有控件
void CCMainWindow::initControl()
{
    // 为联系人树控件设置自定义样式，移除焦点框
    ui.treeWidget->setStyle(new CCMainCustomProxyStyle);

    // 初始化等级显示、头像和状态图标
    setLevelPixmap(0);
    setHeadPixmap(":/Resources/MainWindow/girl.png");
    setStatusMenuIcon(":/Resources/MainWindow/StatusSucceeded.png");

    // 初始化顶部应用图标栏布局
    QHBoxLayout* appupLayout = new QHBoxLayout;
    appupLayout->setContentsMargins(0, 0, 0, 0);  // 清除布局边距
    appupLayout->addWidget(addOtherAppExtension(":/Resources/MainWindow/app/app_7.png", "app_7"));
    appupLayout->addWidget(addOtherAppExtension(":/Resources/MainWindow/app/app_2.png", "app_2"));
    appupLayout->addWidget(addOtherAppExtension(":/Resources/MainWindow/app/app_3.png", "app_3"));
    appupLayout->addWidget(addOtherAppExtension(":/Resources/MainWindow/app/app_4.png", "app_4"));
    appupLayout->addWidget(addOtherAppExtension(":/Resources/MainWindow/app/app_5.png", "app_5"));
    appupLayout->addWidget(addOtherAppExtension(":/Resources/MainWindow/app/app_6.png", "app_6"));
    appupLayout->addWidget(addOtherAppExtension(":/Resources/MainWindow/app/skin.png", "app_skin"));
    appupLayout->addStretch();                   // 添加伸缩项，使图标靠左排列
    appupLayout->setSpacing(2);                  // 设置图标间距为2px
    ui.appWidget->setLayout(appupLayout);        // 应用布局到控件

    // 初始化底部应用图标栏
    ui.bottomLayout_up->addWidget(addOtherAppExtension(":/Resources/MainWindow/app/app_10.png", "app_10"));
    ui.bottomLayout_up->addWidget(addOtherAppExtension(":/Resources/MainWindow/app/app_8.png", "app_8"));
    ui.bottomLayout_up->addWidget(addOtherAppExtension(":/Resources/MainWindow/app/app_11.png", "app_11"));
    ui.bottomLayout_up->addWidget(addOtherAppExtension(":/Resources/MainWindow/app/app_9.png", "app_9"));
    ui.bottomLayout_up->addStretch();            // 添加伸缩项，使图标靠左排列

    // 初始化联系人树控件数据
    initContacTree();

    // 为输入框安装事件过滤器，用于处理焦点和输入事件
    ui.lineEdit->installEventFilter(this);
    ui.searchLineEdit->installEventFilter(this);

    // 绑定窗口控制按钮信号槽
    connect(ui.sysmin, SIGNAL(clicked(bool)), this, SLOT(onShowHide(bool)));  // 最小化按钮
    connect(ui.sysclose, SIGNAL(clicked(bool)), this, SLOT(onShowClose(bool))); // 关闭按钮

    // 监听皮肤更换通知，更新搜索框样式
    connect(NotifyManager::getInstance(), &NotifyManager::signalSkinChanged, [this]()
        {
            updateSeachStyle();
        });

    // 创建系统托盘图标
    SysTray* systray = new SysTray(this);
}

// 初始化定时器
void CCMainWindow::initTimer()
{
    QTimer* timer = new QTimer(this);
    timer->setInterval(500);  // 设置定时器间隔为500ms

    // 定时器超时处理：更新等级数字显示（0-99循环）
    connect(timer, &QTimer::timeout, [this] {
        static int level = 0;
        if (level == 99)
        {
            level = 0;
        }
        level++;
        setLevelPixmap(level);
        });

    timer->start();  // 启动定时器
}

// 设置用户名显示（自动省略过长文本）
void CCMainWindow::setUserName(const QString& username)
{
    ui.nameLabel->adjustSize();  // 调整标签大小适应内容

    // 使用字体度量自动省略过长的用户名，右侧显示省略号
    QString name = ui.nameLabel->fontMetrics().elidedText(username, Qt::ElideRight, ui.nameLabel->width());

    ui.nameLabel->setText(name);  // 设置最终显示的用户名
}

// 设置等级显示图片（动态绘制等级数字）
void CCMainWindow::setLevelPixmap(int level)
{
    // 创建与等级按钮大小一致的透明画布
    QPixmap levelPixmap(ui.levelBtn->size());
    levelPixmap.fill(Qt::transparent);

    // 绘制等级背景图
    QPainter painter(&levelPixmap);
    painter.drawPixmap(0, 4, QPixmap(":/Resources/MainWindow/lv.png"));

    // 分解等级数字的十位和个位
    int unitNum = level % 10;  // 个位数
    int tenNum = level / 10;   // 十位数

    // 绘制十位数（从数字精灵图中截取对应位置）
    painter.drawPixmap(10, 4, QPixmap(":/Resources/MainWindow/levelvalue.png"),
        tenNum * 6, 0, 6, 7);

    // 绘制个位数
    painter.drawPixmap(16, 4, QPixmap(":/Resources/MainWindow/levelvalue.png"),
        unitNum * 6, 0, 6, 7);

    // 设置等级按钮的图标
    ui.levelBtn->setIcon(levelPixmap);
    ui.levelBtn->setIconSize(ui.levelBtn->size());
}

// 设置用户头像（带遮罩的圆形头像）
void CCMainWindow::setHeadPixmap(const QString& headPath)
{
    QPixmap pix;
    pix.load(":/Resources/MainWindow/head_mask.png");
    // 绘制圆形头像并设置到标签
    ui.headLabel->setPixmap(getRoundImage(QPixmap(headPath),
        pix, ui.headLabel->size()));
}

// 设置状态菜单图标
void CCMainWindow::setStatusMenuIcon(const QString& statusPath)
{
    // 创建与状态按钮大小一致的透明画布
    QPixmap statusBtnPixmap(ui.stausBtn->size());
    statusBtnPixmap.fill(Qt::transparent);

    // 绘制状态图标
    QPainter painter(&statusBtnPixmap);
    painter.drawPixmap(4, 4, QPixmap(statusPath));

    // 设置到状态按钮
    ui.stausBtn->setIcon(statusBtnPixmap);
    ui.stausBtn->setIconSize(ui.stausBtn->size());
}

// 创建应用扩展按钮（通用方法）
// 参数: appPath - 图标路径, appName - 按钮标识名
// 返回: 创建好的按钮控件
QWidget* CCMainWindow::addOtherAppExtension(const QString& appPath, const QString& appName)
{
    QPushButton* btn = new QPushButton(this);  // 创建按钮
    btn->setFixedSize(20, 20);                 // 设置固定大小

    // 创建透明画布绘制图标
    QPixmap pixmap(btn->size());
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    QPixmap appPixmap(appPath);
    // 居中绘制图标
    painter.drawPixmap((btn->width() - appPixmap.width()) / 2,
        (btn->height() - appPixmap.height()) / 2, appPixmap);

    // 设置按钮属性
    btn->setIcon(pixmap);
    btn->setIconSize(btn->size());
    btn->setObjectName(appName);            // 设置对象名用于识别
    btn->setProperty("hasborder", true);    // 设置自定义属性标记有边框

    // 绑定点击事件
    connect(btn, &QPushButton::clicked, this, &CCMainWindow::onAppIconClicked);

    return btn;
}

// 初始化联系人树控件
void CCMainWindow::initContacTree()
{
    // 绑定树控件各种交互事件
    connect(ui.treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
        this, SLOT(onItemClicked(QTreeWidgetItem*, int)));          // 单击事件
    connect(ui.treeWidget, SIGNAL(itemExpanded(QTreeWidgetItem*)),
        this, SLOT(onItemExpanded(QTreeWidgetItem*)));              // 展开事件
    connect(ui.treeWidget, SIGNAL(itemCollapsed(QTreeWidgetItem*)),
        this, SLOT(onItemCollapsed(QTreeWidgetItem*)));            // 折叠事件
    connect(ui.treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
        this, SLOT(onItemDoubleClicked(QTreeWidgetItem*, int)));    // 双击事件

    // 创建根节点（公司通讯录）
    QTreeWidgetItem* pRootGroupItem = new QTreeWidgetItem;
    pRootGroupItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator); // 显示子项指示器
    pRootGroupItem->setData(0, Qt::UserRole, 0);                            // 设置自定义数据标记为根节点

    // 创建根节点显示控件
    RootContactItem* pItemName = new RootContactItem(true, ui.treeWidget);
    QString strGroupName = QString::fromLocal8Bit("企业通讯录");
    pItemName->setText(strGroupName);

    // 添加根节点到树控件
    ui.treeWidget->addTopLevelItem(pRootGroupItem);
    ui.treeWidget->setItemWidget(pRootGroupItem, 0, pItemName);

    // 定义公司部门列表
    QStringList sComDeps;
    sComDeps << QString::fromLocal8Bit("企业群")
        << QString::fromLocal8Bit("人事部")
        << QString::fromLocal8Bit("开发部")
        << QString::fromLocal8Bit("市场部");

    // 循环添加各部门节点
    for (int i = 0; i < sComDeps.length(); i++)
    {
        addCompanyDeps(pRootGroupItem, sComDeps.at(i));
    }
}

// 向指定根节点添加公司部门子节点
// 参数: pRootGroupItem - 父节点, sDeps - 部门名称
void CCMainWindow::addCompanyDeps(QTreeWidgetItem* pRootGroupItem, const QString& sDeps)
{
    QTreeWidgetItem* pChild = new QTreeWidgetItem;

    QPixmap pix;
    pix.load(":/Resources/MainWindow/head_mask.png");

    // 设置子节点标记（1表示子节点）
    pChild->setData(0, Qt::UserRole, 1);
    // 设置唯一标识ID（使用指针地址转换）
    pChild->setData(0, Qt::UserRole + 1, QString::number((int)pChild));

    // 创建联系人项显示控件
    ContactItem* pContactItem = new ContactItem(ui.treeWidget);
    pContactItem->setHeadPixmap(getRoundImage(QPixmap(":/Resources/MainWindow/girl.png"),
        pix, pContactItem->getHeadLabelSize()));
    pContactItem->setUserName(sDeps);

    // 添加子节点到父节点
    pRootGroupItem->addChild(pChild);
    ui.treeWidget->setItemWidget(pChild, 0, pContactItem);

    // 记录节点与部门名称的映射关系
    m_groupMap.insert(pChild, sDeps);
}

// 重写窗口大小改变事件
void CCMainWindow::resizeEvent(QResizeEvent* event)
{
    // 更新用户名显示（适配窗口大小变化）
    setUserName(QString::fromLocal8Bit("测试用户"));
    BasicWindow::resizeEvent(event);  // 调用父类处理
}

// 事件过滤器：处理输入框焦点事件
bool CCMainWindow::eventFilter(QObject* obj, QEvent* event)
{
    // 处理搜索框焦点事件
    if (ui.searchLineEdit == obj)
    {
        // 获得焦点时更新样式
        if (event->type() == QEvent::FocusIn)
        {
            ui.searchWidget->setStyleSheet(QString("QWidget#searchWidget{background-color:rgb(255,255,255);border-bottom:1px solid rgba(%1,%2,%3,100)} \
                                                                                QPushButton#searchBtn{border-image:url(:/Resources/MainWindow/search/main_search_deldown.png)} \
                                                                                QPushButton#searchBtn:hover{border-image:url(:/Resources/MainWindow/search/main_search_delhighlight.png)} \
                                                                                QPushButton#searchBtn:pressed{border-image:url(:/Resources/MainWindow/search/main_search_delhighdown.png)}")
                .arg(m_colorBackGround.red())
                .arg(m_colorBackGround.green())
                .arg(m_colorBackGround.blue()));
        }
        // 失去焦点时恢复默认样式
        else if (event->type() == QEvent::FocusOut)
        {
            updateSeachStyle();
        }
    }

    return false;  // 返回false表示不拦截事件
}

// 鼠标按下事件处理：清除输入框焦点
void CCMainWindow::mousePressEvent(QMouseEvent* event)
{
    // 如果点击位置不在搜索框上且搜索框有焦点，则清除焦点
    if (qApp->widgetAt(event->pos()) != ui.searchLineEdit && ui.searchLineEdit->hasFocus())
    {
        ui.searchLineEdit->clearFocus();
    }
    // 同理处理普通输入框
    else if (qApp->widgetAt(event->pos()) != ui.lineEdit && ui.lineEdit->hasFocus())
    {
        ui.lineEdit->clearFocus();
    }

    // 调用父类处理
    BasicWindow::mousePressEvent(event);
}

// 更新搜索框默认样式
void CCMainWindow::updateSeachStyle()
{
    ui.searchWidget->setStyleSheet(QString("QWidget#searchWidget{background-color:rgba(%1,%2,%3,50);border-bottom:1px solid rgba(%1,%2,%3,30)}\
                                                                            QPushButton#searchBtn{border-image:url(:/Resources/MainWindow/search/search_icon.png)}")
        .arg(m_colorBackGround.red())
        .arg(m_colorBackGround.green())
        .arg(m_colorBackGround.blue()));
}

// 树节点单击事件处理
void CCMainWindow::onItemClicked(QTreeWidgetItem* item, int column)
{
    // 判断是否为子节点（通过自定义数据标记）
    bool bIsChild = item->data(0, Qt::UserRole).toBool();

    // 如果是根节点，切换展开/折叠状态
    if (!bIsChild)
    {
        item->setExpanded(!item->isExpanded());
    }
}

// 树节点展开事件处理
void CCMainWindow::onItemExpanded(QTreeWidgetItem* item)
{
    bool bIsChild = item->data(0, Qt::UserRole).toBool();
    if (!bIsChild)
    {
        // 转换为根节点控件并更新展开状态
        RootContactItem* prootItem = dynamic_cast<RootContactItem*>(ui.treeWidget->itemWidget(item, 0));

        if (prootItem)
        {
            prootItem->setExpanded(true);
        }
    }
}

// 树节点折叠事件处理
void CCMainWindow::onItemCollapsed(QTreeWidgetItem* item)
{
    bool bIsChild = item->data(0, Qt::UserRole).toBool();
    if (!bIsChild)
    {
        // 转换为根节点控件并更新折叠状态
        RootContactItem* prootItem = dynamic_cast<RootContactItem*>(ui.treeWidget->itemWidget(item, 0));

        if (prootItem)
        {
            prootItem->setExpanded(false);
        }
    }
}

// 树节点双击事件处理：打开聊天窗口
void CCMainWindow::onItemDoubleClicked(QTreeWidgetItem* item, int column)
{
    // 判断是否为子节点
    bool bIsChild = item->data(0, Qt::UserRole).toBool();

    if (bIsChild)
    {
        // 获取节点对应的部门名称
        QString strGroup = m_groupMap.value(item);

        // 根据不同部门打开对应的聊天窗口
        if (strGroup == QString::fromLocal8Bit("企业群"))
        {
            WindowManager::getInstance()->addNewTalkWindow(item->data(0, Qt::UserRole + 1).toString(), COMPANY);
        }
        else if (strGroup == QString::fromLocal8Bit("人事部"))
        {
            WindowManager::getInstance()->addNewTalkWindow(item->data(0, Qt::UserRole + 1).toString(), PERSONELGROUP);
        }
        else if (strGroup == QString::fromLocal8Bit("市场部"))
        {
            WindowManager::getInstance()->addNewTalkWindow(item->data(0, Qt::UserRole + 1).toString(), MARKETGROUP);
        }
        else if (strGroup == QString::fromLocal8Bit("开发部"))
        {
            WindowManager::getInstance()->addNewTalkWindow(item->data(0, Qt::UserRole + 1).toString(), DEVELOPMENTGROUP);
        }
    }
}

// 应用图标点击事件处理
void CCMainWindow::onAppIconClicked()
{
    // 判断是否点击了换肤图标
    if (sender()->objectName() == "app_skin")
    {
        // 打开皮肤设置窗口
        SkinWindow* skinWindow = new SkinWindow;
        skinWindow->show();
    }
}